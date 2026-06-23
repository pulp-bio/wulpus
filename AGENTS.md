# WULPUS Agent Guidelines

## Commands

- **Start backend**: `uv run python -m wulpus.main` in `sw/` (hosts backend at http://127.0.0.1:8000/)
- **Frontend dev**: `npm run dev` in `sw/wulpus-frontend/` (dev server at http://localhost:5173/)
- **Frontend build**: `npm run build` in `sw/wulpus-frontend/`
- **Frontend lint**: `npm run lint` in `sw/wulpus-frontend/`

## Architecture

- **Firmware**: `fw/msp430/` (ultrasound MCU), `fw/nrf52/` (BLE MCU + USB dongle)
- **Software**: `sw/wulpus/` (FastAPI backend), `sw/wulpus-frontend/` (React frontend)
- **Communication**: WebSocket for real-time data, REST API for control

## Code Style

- **Python**: Type hints, snake_case, async/await for I/O
- **TypeScript/React**: camelCase, functional components, strict typing
- **Imports**: Standard library first, third-party, then local imports
- **File structure**: Clear separation of concerns, API models in separate files
- **Error handling**: Use proper exception types, HTTP status codes for API errors
- **Data processing**: NumPy for measurements, Pandas for structured data analysis

### General
Write code where the naming of variables and functions is self explanatory.
Comments should be used sparingly.
Make sure to not catch error-cases that logically can't occur.
