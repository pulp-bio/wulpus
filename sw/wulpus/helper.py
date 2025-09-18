from __future__ import annotations

import glob
import inspect
import io
import json
import os
from typing import Any, Generic, Iterable, Iterator, Tuple, TypeVar
from zipfile import ZipFile

import numpy as np
import pandas as pd
from fastapi import HTTPException
from wulpus.wulpus_config_models import WulpusConfig

import wulpus as wulpus_pkg


def ensure_dir(dir: str) -> None:
    os.makedirs(dir, exist_ok=True)


def check_if_filereq_is_legitimate(req_name: str, system_dir: str, allowed_ending: str) -> str:
    """ Check if the requested file seems plausible.

    Raise HTTPExceptions if invalid.

    Returns:
        str: The validated file path.
    """
    if os.path.sep in req_name or (os.path.altsep and os.path.altsep in req_name) or len(req_name) > 100:
        raise HTTPException(status_code=400, detail="Invalid req_name")
    if not req_name.lower().endswith(allowed_ending):
        raise HTTPException(status_code=400, detail="Invalid file type")
    path = os.path.join(system_dir, req_name)
    if not os.path.isfile(path):
        raise HTTPException(status_code=404, detail="File not found")
    return path


def zip_to_dataframe(path: str) -> Tuple[pd.DataFrame, WulpusConfig]:
    """
    Returns df: DataFrame with log; config: WulpusConfig object
    """
    with ZipFile(path, 'r') as zf:
        config_raw = json.loads(zf.read('config-0.json').decode('utf-8'))
        df_flat = pd.read_parquet(io.BytesIO(zf.read('data.parquet')))

    # Columns created by save
    meta_cols = {'tx', 'rx', 'aq_number', 'tx_rx_id', 'log_version'}
    sample_cols = [c for c in df_flat.columns if c not in meta_cols]

    # Ensure numeric order for sample columns (they were saved as strings)
    sample_cols = sorted(sample_cols, key=lambda c: int(c))

    # Rebuild `measurement` as a Series per row
    measurements = list(
        pd.Series(row[sample_cols].to_numpy(copy=False))
        for _, row in df_flat.iterrows()
    )

    df = pd.DataFrame({
        'measurement': measurements,
        'tx': df_flat['tx'].tolist(),
        'rx': df_flat['rx'].tolist(),
        'aq_number': df_flat['aq_number'].to_numpy(),
        'tx_rx_id': df_flat['tx_rx_id'].to_numpy() if 'tx_rx_id' in df_flat else np.arange(len(df_flat)),
        'log_version': df_flat['log_version'].to_numpy() if 'log_version' in df_flat else np.full(len(df_flat), 1, dtype=int),
    }, index=df_flat.index)

    config = WulpusConfig.model_validate(config_raw)
    return df, config


def find_latest_measurement_zip(n=1) -> list[str]:
    """Return path to the most recent .zip in the package measurements folder.

    Raises FileNotFoundError if the folder or files don't exist.
    """
    measurement_dir = os.path.join(os.path.dirname(
        inspect.getfile(wulpus_pkg)), 'measurements')
    if not os.path.exists(measurement_dir):
        raise FileNotFoundError(
            f"Measurements directory not found: {measurement_dir}")
    zip_files = glob.glob(os.path.join(measurement_dir, '*.zip'))
    if not zip_files:
        raise FileNotFoundError(f"No zip files found in {measurement_dir}")
    zip_files.sort()
    return zip_files[:n]


def estimate_measurement_duration_seconds(config: WulpusConfig) -> float:
    """Estimate duration of a job in seconds."""
    return (config.us_config.num_acqs * config.us_config.meas_period) / 1e6


def concat_dataframes(dfs: list[pd.DataFrame]) -> pd.DataFrame:
    """Concatenate multiple DataFrames with same columns."""
    return pd.concat(dfs, axis=0)


def get_all_zips_from_folder(folder: str) -> list[str]:
    """Return a list of all .zip files in the specified folder."""
    if not os.path.exists(folder):
        raise FileNotFoundError(f"Folder not found: {folder}")
    zip_files = glob.glob(os.path.join(folder, '*.zip'))
    if not zip_files:
        raise FileNotFoundError(f"No zip files found in {folder}")
    zip_files.sort()
    return zip_files


T = TypeVar('T')


class PassByRef(Generic[T]):
    """Simple generic wrapper to carry a mutable reference to a value.
    """

    def __init__(self, value: T):
        self.value: T = value

    def set(self, value: T) -> None:
        self.value = value

    def __getattr__(self, item: str) -> Any:  # runtime delegation
        return getattr(self.value, item)

    def __repr__(self) -> str:  # pragma: no cover simple convenience
        return f"PassByRef({self.value!r})"

    # Optional iteration delegation if underlying is iterable
    def __iter__(self) -> Iterator[Any]:  # type: ignore[override]
        if isinstance(self.value, Iterable):
            return iter(self.value)
        raise TypeError(f"{type(self.value).__name__} object is not iterable")
