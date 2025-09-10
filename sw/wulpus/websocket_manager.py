from __future__ import annotations
import asyncio
import json
from typing import TYPE_CHECKING, Union, Any

from fastapi import WebSocket, WebSocketDisconnect
from fastapi.encoders import jsonable_encoder
from fastapi.websockets import WebSocketState

if TYPE_CHECKING:
    from wulpus.wulpus import Wulpus


class WebsocketManager:
    def __init__(self, _wulpus: Wulpus):
        self.active_connections: list[WebSocket] = []
        self.wulpus = _wulpus

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
        for connection in self.active_connections:
            try:
                await connection.send_json(message)
            except (RuntimeError, WebSocketDisconnect):  # Client disconnected
                self.disconnect(connection)

    async def send_status(self, websocket: WebSocket):
        old_status: Union[dict[str, Any], None] = None
        try:
            while websocket.application_state == WebSocketState.CONNECTED:
                status = self.wulpus.get_status()
                if status != old_status:
                    await websocket.send_json(jsonable_encoder(status))
                old_status = status
                await asyncio.sleep(0.05)
        except (RuntimeError, WebSocketDisconnect):  # Client disconnected
            return

    async def send_data(self, new_measurement_event: asyncio.Event):
        while True:
            await new_measurement_event.wait()
            new_measurement_event.clear()
            data = self.wulpus.get_latest_frame()
            if data is not None:
                await self.broadcast_json(data)
