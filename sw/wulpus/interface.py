"""
Generic dongle interface for Wulpus device backends.

Defines a typed, async-first contract used by concrete implementations:
- Serial-based dongle (dongle.py)
- Mock dongle (dongle_mock.py)
- Direct (e.g., BLE) dongle (dongle_direct.py)
"""

from __future__ import annotations

from abc import ABC, abstractmethod
from enum import Enum
from typing import Optional, List, Tuple
from typing_extensions import TypedDict

import numpy as np
from serial.tools.list_ports_common import ListPortInfo
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from wulpus.wulpus import Wulpus


class ConnectionType(Enum):
    SERIAL = "serial"
    BLE = "ble"


class ConnectionOption(TypedDict):
    device: str
    description: str
    type: ConnectionType


class DongleInterface(ABC):
    """Abstract base class for Wulpus dongles."""

    acq_length: int

    def __init__(self, *, acq_length: int = 400) -> None:
        self.acq_length = acq_length

    @abstractmethod
    async def get_available(self) -> List[ConnectionOption]:
        """Discover and return available connection options."""
        raise NotImplementedError

    @abstractmethod
    async def connect(
        self,
        device: Optional[ListPortInfo] = None,
        device_str: Optional[str] = None,
    ) -> bool:
        """Open the device connection."""
        raise NotImplementedError

    @abstractmethod
    async def close(self) -> None:
        """Close the device connection, if open."""
        raise NotImplementedError

    @abstractmethod
    async def send_config(self, conf_bytes_pack: bytes) -> bool:
        """Send a configuration package to the device."""
        raise NotImplementedError

    @abstractmethod
    async def receive_data(
        self,
        wulpus: Wulpus,
        acq_length: int = 400,
    ) -> Optional[Tuple[np.ndarray, int, int]]:
        """
        Receive a single data acquisition frame.
        acquisition_running is dict {"running": bool}
        Returns a tuple of (rf_data int16 array, acq_number, tx_rx_id) or None.
        """
        raise NotImplementedError

    @abstractmethod
    def get_status(self) -> str:
        """Human-readable backend status info."""
        raise NotImplementedError
