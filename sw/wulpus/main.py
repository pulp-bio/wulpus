import asyncio
import inspect
import json
import os
import time
import subprocess
from typing import List, Optional, Union

import uvicorn
from fastapi import (FastAPI, HTTPException,
                     WebSocket, WebSocketDisconnect, Request)
from fastapi.responses import FileResponse, PlainTextResponse
from fastapi.staticfiles import StaticFiles
from wulpus.wulpus_model import Status
from wulpus.data_processing import AnalysisConfig, MeasurementProcessor
from wulpus.series import series_loop, SeriesConfig, SeriesStartRequest
from wulpus.wulpus_api import CONFIG_FILE_EXTENSION, DATA_FILE_EXTENSION
from wulpus.helper import PassByRef, check_if_filereq_is_legitimate, ensure_dir, estimate_measurement_duration_seconds, get_saved_analysis_config
from wulpus.websocket_manager import WebsocketManager
from wulpus.wulpus_config_models import (ConDev, MultiWulpusConfig, TxRxConfig, UsConfig,
                                         WulpusConfig)
from wulpus.data_processing import ANALYSIS_CONFIG_DIR, ANALYSIS_CONFIG_FILENAME
from pydantic import BaseModel, Field
from wulpus.wulpus_mock import WulpusMock
import wulpus as wulpus_pkg
from wulpus.wulpus import Wulpus

THIS_DIR = os.path.dirname(inspect.getfile(wulpus_pkg))
MEASUREMENTS_DIR = os.path.join(THIS_DIR, 'measurements')
CONFIG_DIR = os.path.join(THIS_DIR, 'configs')
FRONTEND_DIR = os.path.join(THIS_DIR, 'production-frontend')

wulpi = [Wulpus()]  # List of Wulpus instances, can be expanded later
wulpus_mocks = [WulpusMock(), WulpusMock(1)]

processor = MeasurementProcessor(get_saved_analysis_config())

manager = WebsocketManager(wulpi, processor)
app = FastAPI()
app.state.send_data_task = None  # type: Optional[asyncio.Task]
app.state.series_task = None  # type: Optional[asyncio.Task]
# type: PassByRef[Optional[SeriesConfig]]
app.state.series_info = PassByRef(None)


@app.post("/api/start")
async def start(config: Union[WulpusConfig,  MultiWulpusConfig]):
    ready_wulpus = 0
    for w in manager.get_wulpus():
        if w.get_status()["status"] == Status.READY:
            ready_wulpus += 1
        w.set_config(config)
    if ready_wulpus == 0:
        raise HTTPException(
            status_code=400, detail="No Wulpus is ready!")
    for w in manager.get_wulpus():
        if w.get_status()["status"] == Status.READY:
            await w.start()
        else:
            manager.remove_wulpus(w.wulpus_id)
    return {"ok": "ok"}


@app.post("/api/stop")
def stop():
    for w in manager.get_wulpus():
        w.stop()
    processor.reset()
    return {"ok": "ok"}


@app.post("/api/series/start")
async def start_series(req: SeriesStartRequest):
    """Start a repeating measurement series at a fixed interval (seconds)."""
    # Reject if a series is already running
    if app.state.series_task and not app.state.series_task.done():
        raise HTTPException(status_code=400, detail="Series already running")

    # Check if duration of single measurement is less than interval
    est = estimate_measurement_duration_seconds(req.config)
    if est >= req.interval_seconds:
        raise HTTPException(
            status_code=400, detail=f"Estimated measurement duration {est:.2f}s exceeds or equals interval {req.interval_seconds}s")

    app.state.series_info.value = SeriesConfig(
        active=True,
        config=req.config,
        interval_seconds=req.interval_seconds,
        number=req.number,
        progress_count=0
    )

    processor.reset()
    app.state.series_task = asyncio.create_task(
        series_loop(app.state.series_info, start))
    return {"ok": True}


@app.post("/api/series/stop")
async def stop_series():
    if app.state.series_task:
        app.state.series_task.cancel()
    app.state.series_info.value = None
    return stop()


@app.get("/api/connections")
async def get_connections():
    results = []
    for w in manager.get_wulpus():
        options = await w.get_connection_options()
        result = {"options": options}
        if hasattr(w, 'id'):
            result["wulpus_id"] = w.wulpus_id
        results.append(result)
    return results


@app.post("/api/connect")
async def connect_single(conf: ConDev):
    for w in manager.get_wulpus():
        if (w.get_status()["status"] == Status.NOT_CONNECTED):
            await w.connect(conf.con_dev)
            return
    # All existing Wulpus are connected, create a new one
    new_wulpus = Wulpus(manager.find_free_id())
    new_wulpus.set_new_measurement_event(app.state.new_data_event)
    manager.add_wulpus(new_wulpus)
    await new_wulpus.connect(conf.con_dev)


@app.post("/api/connect/{wulpus_id}")
async def connect_specific(wulpus_id: int, conf: ConDev):
    wulpus = manager.get_wulpus(wulpus_id)
    if (len(wulpus) == 1):
        w = wulpus[0]
        await w.connect(conf.con_dev)
    else:
        new_wulpus = Wulpus(wulpus_id)
        new_wulpus.set_new_measurement_event(app.state.new_data_event)
        manager.add_wulpus(new_wulpus)
        await new_wulpus.connect(conf.con_dev)


@app.post("/api/disconnect")
async def disconnect(conf: ConDev):
    for w in manager.get_wulpus():
        if conf.con_dev == w.get_status()["endpoint"]:
            await w.disconnect()
            if (len(manager.get_wulpus()) > 1):
                manager.remove_wulpus(w.wulpus_id)
            break


@app.post("/api/disconnect/all")
async def disconnect_all():
    for w in manager.get_wulpus():
        await w.disconnect()
        if (len(manager.get_wulpus()) > 1):
            manager.remove_wulpus(w.wulpus_id)


@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    # Task to periodically broadcast status updates to all clients
    asyncio.create_task(manager.task_send_status(
        websocket, app.state.series_info))

    # Task to broadcast measurements to all clients
    if app.state.send_data_task is None or app.state.send_data_task.done():
        app.state.new_data_event = asyncio.Event()
        new_data_event = app.state.new_data_event

        for w in manager.get_wulpus():
            w.set_new_measurement_event(new_data_event)

        app.state.send_data_task = asyncio.create_task(
            manager.task_broadcast_data(new_data_event))

    await manager.send_data()
    try:
        while True:
            data = await websocket.receive_text()
            await manager.broadcast_text(f"Client says: {data}")
    except WebSocketDisconnect:
        manager.disconnect(websocket)
        await manager.broadcast_text("A Client left the chat")


@app.get("/api/logs", response_model=List[str])
def list_logs() -> List[str]:
    """Return list of saved measurement files (npz) relative names."""
    ensure_dir(MEASUREMENTS_DIR)
    try:
        files = [f for f in os.listdir(
            MEASUREMENTS_DIR) if f.lower().endswith(DATA_FILE_EXTENSION)]
        files.sort(reverse=True)
        return files
    except FileNotFoundError:
        return []


@app.get("/logs/{filename}")
def download_log(filename: str):
    """Download a specific measurement file by filename."""
    ensure_dir(MEASUREMENTS_DIR)
    filepath = check_if_filereq_is_legitimate(
        filename, MEASUREMENTS_DIR, DATA_FILE_EXTENSION)
    return FileResponse(filepath, media_type='application/octet-stream', filename=filename)


@app.get("/api/configs", response_model=List[str])
def list_configs() -> List[str]:
    """Return list of saved config files (json) relative names."""
    ensure_dir(CONFIG_DIR)
    try:
        files = [f for f in os.listdir(
            CONFIG_DIR) if f.lower().endswith(CONFIG_FILE_EXTENSION)]
        files.sort(reverse=True)
        return files
    except FileNotFoundError:
        return []


@app.get("/api/configs/{filename}")
def download_config(filename: str):
    """Download a specific config file by filename."""
    ensure_dir(CONFIG_DIR)
    filepath = check_if_filereq_is_legitimate(
        filename, CONFIG_DIR, CONFIG_FILE_EXTENSION)
    return FileResponse(filepath, media_type='application/octet-stream', filename=filename)


@app.post("/api/configs")
async def save_config(config: WulpusConfig, name: Optional[str] = None):
    """Save the provided config JSON to a file in the configs directory."""
    ensure_dir(CONFIG_DIR)
    # derive safe base filename
    if name is None or len(name.strip()) == 0:
        name = "wulpus-config-" + \
            time.strftime("%Y-%m-%d_%H-%M-%S", time.localtime())
    # very simple sanitization
    if os.path.sep in name or (os.path.altsep and os.path.altsep in name) or len(name) > 100:
        raise HTTPException(status_code=400, detail="Invalid name")
    base = os.path.join(CONFIG_DIR, name)
    filename = base + ".json"
    # avoid overwriting existing files
    suffix = 1
    while os.path.exists(filename):
        filename = f"{base}_{suffix}.json"
        suffix += 1
    try:
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(config.model_dump(), f, ensure_ascii=False, indent=2)
        return {"filename": os.path.basename(filename)}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e)) from e


@app.delete("/api/configs/{filename}")
def delete_config(filename: str):
    """Delete a specific config file by filename."""
    ensure_dir(CONFIG_DIR)
    try:
        filepath = check_if_filereq_is_legitimate(
            filename, CONFIG_DIR, '.json')
        os.remove(filepath)
        return {"ok": True}
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e)) from e


@app.post("/api/activate-mock")
def activate_mock():
    for w in manager.get_wulpus():
        w.stop()
    for w in wulpus_mocks:
        w.set_new_measurement_event(app.state.new_data_event)
    manager.set_wulpus(wulpus_mocks)
    processor.reset()
    return {"ok": "ok"}


@app.post("/api/deactivate-mock")
def deactivate_mock():
    for w in wulpus_mocks:
        w.stop()
    manager.set_wulpus(wulpi)
    processor.reset()
    return {"ok": "ok"}


@app.post("/api/replay/{filename}")
async def replay_file(filename: str):
    # Build a minimal default config: one empty TxRxConfig and a UsConfig with its own defaults
    default_config = WulpusConfig(
        tx_rx_config=[TxRxConfig()], us_config=UsConfig())
    for w in manager.get_wulpus():
        w.stop()
    for w in wulpus_mocks:
        w.set_new_measurement_event(app.state.new_data_event)
    ensure_dir(MEASUREMENTS_DIR)
    manager.set_wulpus(wulpus_mocks)
    filepath = check_if_filereq_is_legitimate(
        filename, MEASUREMENTS_DIR, DATA_FILE_EXTENSION)
    for w in wulpus_mocks:
        w.set_config(default_config)
        w.set_replay_file(filepath)
    processor.reset()
    await asyncio.gather(*(w.start() for w in wulpus_mocks))


@app.get("/api/analyzeConfig", response_model=Optional[AnalysisConfig])
def get_analyze_config():
    return processor.get_analyze_config()


@app.post("/api/analyzeConfig", response_model=Optional[AnalysisConfig])
def set_analyze_config(config: AnalysisConfig):
    ensure_dir(ANALYSIS_CONFIG_DIR)
    filename = os.path.join(ANALYSIS_CONFIG_DIR, ANALYSIS_CONFIG_FILENAME)
    with open(filename, 'w', encoding='utf-8') as f:
        json.dump(config.model_dump(), f, ensure_ascii=False, indent=2)
    processor.set_analyze_config(config)
    return get_analyze_config()


@app.get("/api/version", response_model=Optional[AnalysisConfig])
def get_version() -> str:
    path = os.path.join(THIS_DIR, "version.txt")
    if os.path.isfile(path):
        with open(path, "r", encoding="utf-8") as f:
            return PlainTextResponse(f.read().strip())
    else:
        text = "unknown"
        try:
            commit_hash = subprocess.check_output(
                ["git", "rev-parse", "HEAD"], cwd=THIS_DIR, stderr=subprocess.DEVNULL
            ).decode("utf-8").strip()
            commit_date = subprocess.check_output(
                ["git", "show", "-s", "--format=%cd", "--date=iso-strict", "HEAD"],
                cwd=THIS_DIR,
                stderr=subprocess.DEVNULL,
            ).decode("utf-8").strip().split("T")[0]
            text = f"dev: {commit_hash} from {commit_date}"
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass
        return PlainTextResponse(text)


app.mount("/assets", StaticFiles(directory=os.path.join(FRONTEND_DIR,
          'assets')), name="assets")


@app.get("/{full_path:path}")
async def frontend_fallback(full_path: str, request: Request):
    index_path = os.path.join(FRONTEND_DIR, "index.html")
    return FileResponse(index_path)


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
