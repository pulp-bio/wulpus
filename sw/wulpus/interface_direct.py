from __future__ import annotations
import asyncio
import re
from typing import Union, Optional, List

import numpy as np

from bleak import BleakClient, BleakScanner, BLEDevice
from bleak.exc import BleakError
from serial.tools.list_ports_common import ListPortInfo
from typing import TYPE_CHECKING
from .interface import DongleInterface, ConnectionType, ConnectionOption
if TYPE_CHECKING:
    from wulpus.wulpus import Wulpus

# Standard Nordic UART Service (NUS) UUIDs
NUS_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
NUS_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  # From PC to Wulpus
NUS_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  # From Wulpus to PC
WULPUS_NAME_PATTERN = re.compile(r"WULPUS_PROBE_\d+")


class WulpusDongleDirect(DongleInterface):
    """
    Class representing the Wulpus via direct Bluetooth connection
    """

    def __init__(self) -> None:
        super().__init__()
        self._devices: list[BLEDevice] = []
        self._bleak_client: Union[BleakClient, None] = None
        self._data_queue: Optional["asyncio.Queue[bytes]"] = None
        self._loop: Optional[asyncio.AbstractEventLoop] = None

    async def get_available(self) -> List[ConnectionOption]:
        """
        Get a list of available devices.

        Returns:
            list[dict[str, object]]: A list of dicts with keys "device", "description" and "type" (ConnectionType).
        """
        try:
            devices = await BleakScanner.discover()
            devices = [
                d for d in devices if d.name and WULPUS_NAME_PATTERN.fullmatch(d.name)]
            self._devices = sorted(devices, key=lambda d: d.name)
            return [
                {"device": str(d.address), "description": str(
                    d.name), "type": ConnectionType.BLE}
                for d in self._devices
            ]
        except OSError:
            print("OSError during Bluetooth discovery - Adapter probably disabled")
            return []

    async def connect(self, device: Optional[ListPortInfo] = None, device_str: Optional[str] = None) -> bool:
        """
        Open the device connection.
        """
        if (device):
            raise ValueError(
                "Device works only with Serial USB Dongle (not direct BLE connection)")

        target_address: Optional[str] = None
        if device_str:
            target_address = device_str
        # If a previous discovery returned devices, allow selecting by index via device.device if provided as ListPortInfo-like.
        # But for BLE we'll take device_str (MAC) as the source of truth.

        if not target_address:
            # Try to pick the first discovered WULPUS device
            if not self._devices:
                await self.get_available()
            if not self._devices:
                print("Error: no BLE WULPUS device found.")
                return False
            target = self._devices[0]
            target_address = target.address

        try:
            # Bind to the current running loop and create queue here to avoid cross-loop issues
            self._loop = asyncio.get_running_loop()
            # Reasonable buffer to absorb short bursts without unbounded growth
            self._data_queue = asyncio.Queue(maxsize=1000)
            self._bleak_client = BleakClient(target_address)
            await self._bleak_client.connect()
            await self._bleak_client.start_notify(NUS_TX_CHAR_UUID, self._notification_handler)
            return True
        except (BleakError, OSError, TimeoutError) as e:
            print("Error while trying to open BLE device:", e)
            return False

    def _notification_handler(self, _sender: int, data: bytearray):
        """
        Handle incoming notifications from the BLE device.
        """
        # Ensure we enqueue on the correct event loop thread (Bleak callback may be on another thread)
        if self._loop and self._data_queue is not None:
            def _enqueue():
                try:
                    if self._data_queue.full():
                        # Drop oldest to make space
                        self._data_queue.get_nowait()
                    self._data_queue.put_nowait(bytes(data))
                except (asyncio.QueueEmpty, asyncio.QueueFull):
                    # Swallow to avoid noisy exceptions in callback thread
                    pass
            self._loop.call_soon_threadsafe(_enqueue)

    async def close(self):
        """
        Close the device connection.
        """
        try:
            if self._bleak_client and self._bleak_client.is_connected:
                await self._bleak_client.stop_notify(NUS_TX_CHAR_UUID)
                try:
                    await self._bleak_client.disconnect()
                except BleakError:
                    pass
        except BleakError as e:
            print("Error while trying to close BLE device:", e)
        finally:
            self._bleak_client = None
            self._data_queue = None
            self._loop = None

    async def send_config(self, conf_bytes_pack: bytes):
        """
        Send a configuration package to the device.
        """
        if not self._bleak_client:
            print("Error: BLE client is not connected.")
            return False

        try:
            await self._bleak_client.write_gatt_char(NUS_RX_CHAR_UUID, conf_bytes_pack)
            return True
        except BleakError as e:
            print("Error while trying to send config to BLE device:", e)
            return False

    async def receive_data(self, wulpus: Wulpus, acq_length: int = 400):
        """
        Receives and processes data from the Wulpus device.

        The function assembles a complete data frame from incoming BLE packets.
        A frame is composed of 4 packets, totaling 804 bytes.
        - The first packet is 202 bytes, with the first byte being a start-of-frame marker (0xFF).
          The last byte of this packet is ignored.
        - The following three packets are each 201 bytes.

        The assembled 804-byte frame has the following structure:
        - 1-byte start of frame marker (0xFF)
        - 1-byte tx_rx_id
        - 2-byte acquisition number (little-endian)
        - 800-byte RF data payload (400 samples of 16-bit signed integers)
        """
        if not self._bleak_client or not self._bleak_client.is_connected:
            print("Error: BLE client is not connected.")
            return None, None, None

        if self._data_queue is None:
            print("Error: Data queue not initialized.")
            return None, None, None

        frame_buffer = bytearray()

        while wulpus.get_acquisition_running():
            try:
                # Wait for data with a timeout to allow checking the acquisition_running flag
                data = await asyncio.wait_for(self._data_queue.get(), timeout=0.1)
            except asyncio.TimeoutError:
                continue

            # Start of a new frame
            if len(data) == 202 and data[0] == 0xFF:
                if len(frame_buffer) > 0:
                    print(
                        f"Warning: Incomplete frame discarded ({len(frame_buffer)} bytes)")
                # The first packet is 202 bytes, but we discard the last byte which is likely garbage due to a firmware bug.
                frame_buffer = bytearray(data[:201])
                continue

            # Subsequent packets of the current frame
            if frame_buffer and len(data) == 201:
                frame_buffer.extend(data)

            # A full frame has been received
            if len(frame_buffer) == 804:
                # Frame structure: [0xFF, tx_rx_id, acq_nr_L, acq_nr_H, data...]
                tx_rx_id = frame_buffer[1]
                acq_nr = int.from_bytes(frame_buffer[2:4], 'little')
                # The actual RF data starts after the 4-byte header
                rf_arr = np.frombuffer(frame_buffer[4:], dtype='<i2')

                # Verify data length
                if len(rf_arr) == acq_length:
                    return rf_arr, acq_nr, tx_rx_id
                else:
                    print(
                        f"Warning: Malformed frame received (data length {len(rf_arr)}, expected {acq_length}). Discarding.")
                    # Reset buffer if data is malformed
                    frame_buffer = bytearray()

        return None, None, None

    def get_status(self):
        if self._bleak_client and self._bleak_client.is_connected:
            addr = getattr(self._bleak_client, 'address', None)
            return f"connected to {addr}" if addr else "connected"
        return "not connected"

    def get_connection_endpoint(self) -> str:
        if self._bleak_client and self._bleak_client.is_connected:
            addr = getattr(self._bleak_client, 'address', None)
            return addr if addr else ""
        return ""


if __name__ == "__main__":
    async def _main():
        dongle = WulpusDongleDirect()
        print(await dongle.get_available())
    asyncio.run(_main())
