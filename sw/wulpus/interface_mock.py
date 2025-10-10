from __future__ import annotations
import asyncio
from serial.tools.list_ports_common import ListPortInfo
import numpy as np
from wulpus.interface_usb import WulpusDongleUsb
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from wulpus.wulpus import Wulpus


class WulpusDongleMock(WulpusDongleUsb):
    """
    Class representing the Wulpus dongle (mock implementation).
    """

    def __init__(self, port: str = '', timeout_write: int = 3, baudrate: int = 4000000):
        super().__init__(port=port, timeout_write=timeout_write, baudrate=baudrate)
        self.acq_num = 0
        self.acq_length = 400

    async def connect(self, device: ListPortInfo = None, device_str: str = None):
        """
        Open the device connection.
        """
        self.acq_num = 0
        return True

    async def close(self):
        """
        Close the device connection.
        """
        return

    async def send_config(self, conf_bytes_pack: bytes):
        """
        Send a configuration package to the device.
        """
        print("Configuration sent:", conf_bytes_pack)
        self.acq_num = 0
        return True

    async def receive_data(self, wulpus: Wulpus, acq_length: int = 400):
        """
        Mock: Return random data with the same structure as the original (async).
        """
        self.acq_length = acq_length
        await asyncio.sleep(0.2)  # Simulate some delay
        rf_arr = np.random.randint(
            1, 1001, size=self.acq_length, dtype="<i2")
        tx_rx_id = 0
        acq_num = self.acq_num

        self.acq_num += 1
        return rf_arr, acq_num, tx_rx_id

    def get_status(self):
        return "Dongle is mocked!"

    def get_connection_endpoint(self) -> str:
        return "mocked_endpoint"
