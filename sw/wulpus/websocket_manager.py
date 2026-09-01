from __future__ import annotations

import asyncio
import inspect
from typing import TYPE_CHECKING, Any, Callable, Optional, Union

from fastapi import WebSocket, WebSocketDisconnect
from fastapi.encoders import jsonable_encoder
from fastapi.websockets import WebSocketState
from wulpus.data_processing import MeasurementProcessor
from wulpus.helper import PassByRef
from wulpus.series import SeriesConfig

if TYPE_CHECKING:
    from wulpus.wulpus import Wulpus


class WebsocketManager:
    def __init__(self, _wulpus: list[Wulpus], _processor: MeasurementProcessor = MeasurementProcessor()):
        self.active_connections: list[WebSocket] = []
        self._processor = _processor
        self.set_wulpus(_wulpus)

    def set_wulpus(self, wulpus: Union[list[Wulpus], Wulpus]):
        if isinstance(wulpus, list):
            self.wulpus = list(wulpus)
        else:
            self.wulpus = [wulpus]

    def add_wulpus(self, wulpus: Wulpus):
        self.wulpus.append(wulpus)
        return wulpus

    def remove_wulpus(self, wulpus_id: int):
        wulpus = self._select_wulpus(wulpus_id)
        if wulpus:
            print(f"Removing Wulpus with id {wulpus_id}")
            self.wulpus.remove(wulpus)
            print("Removing done")

    def _select_wulpus(self, wulpus_id: int) -> Optional[Wulpus]:
        filtered_wulpus = list(
            filter(lambda w: w.wulpus_id == wulpus_id, self.wulpus))
        if (len(filtered_wulpus) > 1):
            raise ValueError(
                f"Multiple Wulpus instances with id {wulpus_id} found.")
        elif (len(filtered_wulpus) == 0):
            print(f"No Wulpus instance with id {wulpus_id} found.")
            return None
        else:
            return filtered_wulpus[0]

    def get_wulpus(self, wulpus_id: Optional[int] = None) -> list[Wulpus]:
        """Get Wulpus instances
        If ID is provided, a single instance with this id will be returned in a list.
        If no ID is provided, all instances will be returned.
        """
        if wulpus_id is None:
            return self.wulpus
        elif self._select_wulpus(wulpus_id) is not None:
            return [self._select_wulpus(wulpus_id)]
        else:
            return []

    async def exec_wulpus_function(self, func: Callable[[Wulpus], Any]):
        results = []
        for w in self.wulpus:
            result = func(w)
            if inspect.iscoroutine(result):
                result = await result
            results.append(result)
        return results

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
        old_status: Union[list[dict[str, Any]], None] = None
        try:
            while websocket.application_state == WebSocketState.CONNECTED:
                statuses = []
                for w in self.wulpus:
                    status = w.get_status()
                    status = {**status, "wulpus_id": w.wulpus_id}
                    if series_info.value:
                        status = {**status, "series": series_info.value}
                    statuses.append(status)
                if statuses != old_status:
                    await websocket.send_json(jsonable_encoder(statuses))
                old_status = statuses
                await asyncio.sleep(0.05)
        except (RuntimeError, WebSocketDisconnect):  # Client disconnected
            return

    async def task_broadcast_data(self, new_measurement_event: asyncio.Event):
        while True:
            await new_measurement_event.wait()
            new_measurement_event.clear()
            await self.send_data()

    async def send_data(self):
        for w in self.wulpus:
            data = w.get_latest_frame()
            if data is not None:
                processed = self._processor.process_measurement(
                    data, w.get_config())
                processed = {**dict(processed), "wulpus_id": w.wulpus_id}
                await self.broadcast_json(processed)

    def find_free_id(self) -> int:
        existing_ids = {w.wulpus_id for w in self.wulpus}
        new_id = 0
        while new_id in existing_ids:
            new_id += 1
        return new_id
