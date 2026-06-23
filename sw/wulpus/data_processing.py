from __future__ import annotations

import math
import os
import inspect
import pywt
import numpy as np
import pandas as pd

from collections import defaultdict, deque
from typing import TYPE_CHECKING, Deque, Dict, List, Optional, Union
from pydantic import BaseModel, Field, field_validator
from scipy.signal import find_peaks, hilbert, resample
from wulpus.wulpus_model import HTTPMeasurementResponse
import wulpus as wulpus_pkg

if TYPE_CHECKING:
    from wulpus.wulpus import Measurement
    from wulpus.wulpus_config_models import WulpusConfig

ANALYSIS_CONFIG_DIR = os.path.join(os.path.dirname(
    inspect.getfile(wulpus_pkg)), 'config-analysis')
ANALYSIS_CONFIG_FILENAME = 'analysis_config.json'


class AnalysisConfig(BaseModel):
    spacers: List[Dict[str, Union[float, str]]] = [
        {"thickness": 0.6, "note": "PDMS", "speedOfSound": 1030},
        {"thickness": 8, "note": "PEEK spacer", "speedOfSound": 2500}
    ]
    peakConsistency: int = Field(default=5, ge=1)
    peakThreshold: float = Field(default=5, ge=0.0)
    peakHistory: int = Field(default=10, ge=1)
    nMaxPeaks: int = Field(default=5, ge=1)
    upsamplingFactor: int = Field(default=10, ge=1)


class MeasurementProcessor:
    """Stateful processor that computes peaks and keeps recent history to filter for consistency.

    Consistency definition: a peak (sample index, possibly fractional) is considered consistent for a
    channel if it appears (within tolerance) in at least ``self.consistency`` of the last
    ``self.peakHistory`` measurements for that channel (including the current one).
    """

    def __init__(self, config: Optional[AnalysisConfig] = None):
        # channel -> deque of list[float] (peaks for each measurement)
        if (config):
            self.config = config
        else:
            self.config = AnalysisConfig()
        self._history: Dict[int, Deque[List[float]]] = defaultdict(
            lambda: deque(maxlen=self.config.peakHistory))

    def get_analyze_config(self) -> AnalysisConfig:
        if not self.config:
            self.config = AnalysisConfig()
        return self.config

    def set_analyze_config(self, config: AnalysisConfig):
        self.config = config
        self.reset()

    def process_measurement(self, measurement: Measurement, config: WulpusConfig) -> HTTPMeasurementResponse:
        series = pd.Series(measurement['data'])

        series_ups = self._upsample(series, self.config.upsamplingFactor)
        t_ups = series_ups.index.to_numpy()
        wavelet = self._wavelet_transform(
            series_ups, config, select_level=5)
        wavelet_env = self._envelope(wavelet)

        spacer_region = self._calc_spacer_region(config)

        raw_peaks = self._calc_peaks(
            wavelet_env, config, spacer_reflection=spacer_region)
        peaks_x = list(map(lambda p: t_ups[p], raw_peaks))
        filtered_peaks = list(
            filter(lambda x: spacer_region[0] < x, peaks_x))

        rx_channels = measurement['rx']
        for ch in rx_channels:
            self._history[int(ch)].append(filtered_peaks)

        peaks_consistent = self._filter_consistent(filtered_peaks, rx_channels)
        peaks_consistent = sorted(peaks_consistent)[: self.config.nMaxPeaks]

        return HTTPMeasurementResponse(
            measurement=measurement,
            peaks=peaks_consistent,
            wavelet=wavelet_env,
            spacer_region=spacer_region)

    def reset(self):
        self._history.clear()

    def _reconstruct_band(self, coeffs, band_idx, wavelet='db4') -> np.ndarray:
        coeffs_keep = [np.zeros_like(c) for c in coeffs]
        coeffs_keep[band_idx] = coeffs[band_idx]
        return pywt.waverec(coeffs_keep, wavelet)

    def _wavelet_transform(self, series: pd.Series, config: WulpusConfig, wavelet: str = 'db4', select_level: Optional[int] = None) -> np.ndarray:
        max_useful_level = pywt.dwt_max_level(
            data_len=series.size, filter_len=wavelet)
        coeffs = pywt.wavedec(series, wavelet, level=max_useful_level)
        sigma = np.median(np.abs(coeffs[-1])) / 0.6745
        threshold = sigma * np.sqrt(2 * np.log(len(series)))
        denoised = [pywt.threshold(c, threshold, mode="soft") for c in coeffs]
        denoised_signal = pywt.waverec(denoised, wavelet)
        if select_level is not None:
            select_level = min(select_level, max_useful_level)
            return self._reconstruct_band(denoised, select_level, wavelet)
        return denoised_signal

    def _upsample(self, series: pd.Series, factor: int) -> pd.Series:
        """Upsample a 1-D series by an integer factor, preserving the original index scale.

        If the original index is numeric and monotonic, the new index spans
        from original_index[0] to original_index[-1] with evenly spaced points.
        Otherwise, it falls back to a 0..N-1 style fractional index.
        """
        n = series.size * factor
        new_values = resample(series.to_numpy(), n)
        idx = series.index
        if len(idx) >= 2 and np.issubdtype(idx.dtype, np.number):
            start = float(idx[0])
            end = float(idx[-1])
        else:
            # Default numeric span 0..(N-1)
            start = 0.0
            end = float(series.size - 1) if series.size > 1 else 0.0
        new_index = np.linspace(start, end, n)
        return pd.Series(new_values, index=new_index)

    def _envelope(self, signal: np.ndarray) -> np.ndarray:
        analytic_signal = hilbert(signal)
        return np.abs(analytic_signal)

    def _calc_peaks(self, series_envelope: np.ndarray, config: WulpusConfig, spacer_reflection: np.ndarray) -> List[float]:
        peaks, _ = find_peaks(series_envelope,
                              distance=10 * self.config.upsamplingFactor,
                              prominence=8)
        return peaks.tolist()

    def _calc_spacer_region(self, config: WulpusConfig) -> np.ndarray:
        t_spacer = float(0.0)
        for sp in self.config.spacers:
            if sp['thickness'] > 0 and sp['speedOfSound'] > 0:
                t_spacer += (sp['thickness'] / sp['speedOfSound'] / 1e3)
        if (t_spacer == 0.0):
            return np.array([0, 0])  # invalid region

        t_shift = (config.us_config.start_adcsampl -
                   config.us_config.start_ppg) * 1e-6

        t_total = (2 * t_spacer) - t_shift
        t_start_tick = t_total * config.us_config.sampling_freq
        t_pulse_duration = config.us_config.num_pulses / config.us_config.pulse_freq
        t_pulse_duration_ticks = t_pulse_duration * config.us_config.sampling_freq

        return np.array([math.floor(t_start_tick), math.ceil(t_start_tick + t_pulse_duration_ticks)])

    def _filter_consistent(self, current_peaks: List[float], rx_channels) -> List[float]:
        if not current_peaks or not rx_channels:
            return current_peaks
        consistent: List[float] = []
        for peak in current_peaks:
            for ch in rx_channels:
                history_lists = list(self._history[int(ch)])
                if not history_lists:
                    continue
                appearances = 0
                for past in history_lists:
                    if any(abs(peak - p) <= self.config.peakThreshold for p in past):
                        appearances += 1
                if appearances >= self.config.peakConsistency:
                    consistent.append(peak)
                    break  # no need to check other channels once accepted
        return consistent
