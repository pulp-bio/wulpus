"""
Serial dongle implementation conforming to DongleInterface.
"""
from __future__ import annotations
import asyncio
from typing import Optional, List

import numpy as np
import serial
from serial.tools.list_ports import comports
from serial.tools.list_ports_common import ListPortInfo
from typing import TYPE_CHECKING
from .interface import DongleInterface, ConnectionType, ConnectionOption

if TYPE_CHECKING:
    from wulpus.wulpus import Wulpus


class WulpusDongleUsb(DongleInterface):
    """Serial implementation of the Wulpus dongle."""

    def __init__(self, port: str = '', timeout_write: int = 3, baudrate: int = 4000000) -> None:
        super().__init__()

        # Serial port setup
        self.__ser__ = serial.Serial()
        self.__ser__.port = port
        self.__ser__.baudrate = baudrate
        self.__ser__.bytesize = serial.EIGHTBITS
        self.__ser__.parity = serial.PARITY_NONE
        self.__ser__.stopbits = serial.STOPBITS_ONE
        self.__ser__.timeout = None
        self.__ser__.xonxoff = False
        self.__ser__.rtscts = False
        self.__ser__.dsrdtr = False
        self.__ser__.writeTimeout = timeout_write

        self._ports: list[ListPortInfo] = []

    async def get_available(self) -> List[ConnectionOption]:
        ports = comports()
        self._ports = sorted(ports)
        return [
            {"device": str(p.device), "description": str(
                p.description), "type": ConnectionType.SERIAL}
            for p in self._ports
        ]

    async def connect(self, device: Optional[ListPortInfo] = None, device_str: Optional[str] = None) -> bool:
        if self.__ser__.is_open:
            return True
        if device_str is not None:
            self.__ser__.port = device_str
        if device is not None:
            self.__ser__.port = device.device
        if not self.__ser__.port:
            print("Error: no serial port specified.")
            return False
        try:
            self.__ser__.open()
            return True
        except serial.SerialException:
            print("Error while trying to open serial port ",
                  str(self.__ser__.port))
            return False

    async def close(self) -> None:
        if not self.__ser__.is_open:
            return
        try:
            self.__ser__.close()
        except serial.SerialException:
            print("Error while trying to close serial port ",
                  str(self.__ser__.port))

    async def send_config(self, conf_bytes_pack: bytes) -> bool:
        if not self.__ser__.is_open:
            print("Error: serial port is not open.")
            return False
        self.__ser__.flushInput()
        self.__ser__.flushOutput()
        self.__ser__.write(conf_bytes_pack)
        return True

    def _get_rf_data_and_info__(self, bytes_arr: bytes):
        rf_arr = np.frombuffer(bytes_arr[7:], dtype='<i2')
        tx_rx_id = bytes_arr[4]
        acq_nr = np.frombuffer(bytes_arr[5:7], dtype='<u2')[0]
        return rf_arr, acq_nr, tx_rx_id

    async def receive_data(self, wulpus: Wulpus, acq_length: int = 400):
        self.acq_length = acq_length
        if not self.__ser__.is_open:
            print("Error: serial port is not open.")
            return None

        def _read_packet(wulpus: Wulpus):
            while wulpus.get_acquisition_running():
                response_start = self.__ser__.readline()
                if len(response_start) != 0 and response_start[-6:] == b'START\n':
                    response = self.__ser__.read(self.acq_length * 2 + 7)
                    return self._get_rf_data_and_info__(response)
            return None

        return await asyncio.to_thread(_read_packet, wulpus)

    def get_status(self) -> str:
        return f"connected to {self.__ser__.port}" if self.__ser__.is_open else "not connected"

    def get_connection_endpoint(self) -> str:
        return self.__ser__.port if self.__ser__.is_open else ""


if __name__ == "__main__":
    async def _main():
        d = WulpusDongleUsb()
        print(await d.get_available())
    asyncio.run(_main())
