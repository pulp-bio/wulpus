# Documentation

Markdown sources for the WULPUS GitHub Pages user guide (MkDocs + Material). Release photos of the hardware are in [`images/`](images/). User-guide figures are in [`figures/`](figures/).

## Preview the site locally

From the repository root:

```sh
uv sync
uv run mkdocs serve
```

Then open the URL printed by MkDocs (typically `http://127.0.0.1:8000/wulpus/`).

The site is published to GitHub Pages from `main` via `.github/workflows/docs.yml`.
