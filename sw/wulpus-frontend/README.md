# WULPUS Frontend

This is the React/Vite frontend for the WULPUS web GUI. During the production build, `build.py` runs the Vite build and copies the generated files into `sw/wulpus/production-frontend`, where the FastAPI backend serves them.

## Development

Install dependencies:

```sh
npm ci
```

Start the frontend dev server:

```sh
npm run dev
```

The dev server usually runs at [http://localhost:5173/](http://localhost:5173/) and talks to the backend at [http://127.0.0.1:8000/](http://127.0.0.1:8000/).

## Build

Build the frontend and copy it into the backend's production frontend directory:

```sh
python build.py
```

Run linting:

```sh
npm run lint
```
