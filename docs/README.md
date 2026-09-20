# Documentation

Markdown sources for the WULPUS GitHub Pages user guide (MkDocs + Material), following the same approach as [BioGUI](https://github.com/pulp-bio/biogui). Photos of the system are in [`images/`](images/).

## Preview the site locally

From the repository root:

```sh
uv sync
uv run mkdocs serve
```

Then open the URL printed by MkDocs (typically `http://127.0.0.1:8000/wulpus/`).

The site is published to GitHub Pages from `main` via `.github/workflows/docs.yml`.
