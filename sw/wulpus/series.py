import asyncio
from typing import Callable
from wulpus.helper import PassByRef
from wulpus.wulpus_config_models import WulpusConfig
from pydantic import BaseModel, Field


class SeriesStartRequest(BaseModel):
    interval_seconds: int = Field(
        gt=0, description="Interval in seconds between triggered measurements")
    config: WulpusConfig
    number: int = Field(
        gt=0, description="Number of times to repeat the measurement in this series")


class SeriesConfig(BaseModel):
    active: bool = Field(default=False)
    config: WulpusConfig
    interval_seconds: int = Field(default=10, gt=0)
    number: int = Field(default=0, gt=0)
    progress_count: int = Field(default=0)


async def series_loop(series_config: PassByRef[SeriesConfig],
                      start_job_callback: Callable[[WulpusConfig], object]):
    config = series_config.value
    # Access underlying value via .value for static typing clarity
    if not config.active:
        return
    while True:
        await start_job_callback(config.config)
        config.progress_count += 1
        if config.progress_count < config.number:
            await asyncio.sleep(config.interval_seconds)
        else:
            break
    config.active = False
