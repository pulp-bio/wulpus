from __future__ import annotations
from pydantic import BaseModel

from wulpus.wulpus import Measurement


class HTTPMeasurementResponse(BaseModel):
    measurement: Measurement
    peaks: list[float]
    wavelet: list[float]
