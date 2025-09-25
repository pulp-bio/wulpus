from enum import IntEnum
from pydantic import BaseModel
from typing_extensions import TypedDict


class Status(IntEnum):
    NOT_CONNECTED = 0
    CONNECTING = 1
    READY = 2
    RUNNING = 3
    ERROR = 9


class Measurement(TypedDict):
    data: list[float]
    time: int
    tx: list[int]
    rx: list[int]


class HTTPMeasurementResponse(BaseModel):
    measurement: Measurement
    peaks: list[float]
    wavelet: list[float]
    spacer_region: list[float] = []  # [start, end] in ticks
