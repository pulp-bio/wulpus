from __future__ import annotations

import asyncio
import json
from typing import TYPE_CHECKING, Any, Callable, Dict, Optional, Union

from fastapi import WebSocket, WebSocketDisconnect
from fastapi.encoders import jsonable_encoder
from fastapi.websockets import WebSocketState
from wulpus.data_processing import MeasurementProcessor
from wulpus.helper import PassByRef
from wulpus.series import SeriesConfig

if TYPE_CHECKING:
    from wulpus.wulpus import Wulpus


class WebsocketManager:
    def __init__(self, _wulpus: Wulpus, _processor: MeasurementProcessor = MeasurementProcessor()):
        self.active_connections: list[WebSocket] = []
        self.wulpus = _wulpus
        self._processor = _processor

    def set_wulpus(self, wulpus: Wulpus):
        self.wulpus = wulpus

    def get_wulpus(self) -> Wulpus:
        return self.wulpus

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        self.active_connections.remove(websocket)

    async def send_single_client(self, message: str, websocket: WebSocket):
        await websocket.send_text(message)

    async def broadcast_text(self, message: str):
        for connection in self.active_connections:
            try:
                await connection.send_text(message)
            except RuntimeError:
                self.disconnect(connection)

    async def broadcast_json(self, message):
        # Ensure message is JSON serializable (handles pydantic models, numpy types, etc.)
        payload = jsonable_encoder(message)
        for connection in self.active_connections:
            try:
                await connection.send_json(payload)
            except (RuntimeError, WebSocketDisconnect):  # Client disconnected
                self.disconnect(connection)

    async def task_send_status(self, websocket: WebSocket, series_info: PassByRef[Optional[SeriesConfig]]):
        old_status: Union[dict[str, Any], None] = None
        try:
            while websocket.application_state == WebSocketState.CONNECTED:
                status = self.wulpus.get_status()
                if series_info.value:
                    status = {**status, "series": series_info.value}
                if status != old_status:
                    await websocket.send_json(jsonable_encoder(status))
                old_status = status
                await asyncio.sleep(0.05)
        except (RuntimeError, WebSocketDisconnect):  # Client disconnected
            return

    async def task_send_data(self, new_measurement_event: asyncio.Event):
        while True:
            await new_measurement_event.wait()
            new_measurement_event.clear()
            await self.send_data()

    async def send_data(self):
        data = self.wulpus.get_latest_frame()
        if data is not None:
            processed = self._processor.process_measurement(
                data, self.wulpus.get_config())
            await self.broadcast_json(processed)
