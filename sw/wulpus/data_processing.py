from __future__ import annotations

import math
from collections import defaultdict, deque
from typing import TYPE_CHECKING, Deque, Dict, List, Optional, Union

import numpy as np
import pandas as pd
import pywt
from scipy.signal import find_peaks, hilbert, resample
from wulpus.wulpus_config_models import WulpusConfig
from wulpus.wulpus_http_models import HTTPMeasurementResponse

if TYPE_CHECKING:
    from wulpus.wulpus import Measurement


class MeasurementProcessor:
    """Stateful processor that computes peaks and keeps recent history to filter for consistency.

    Consistency definition: a peak (sample index, possibly fractional) is considered consistent for a
    channel if it appears (within tolerance) in at least ``min_consistency`` of the last
    ``history_len`` measurements for that channel (including the current one).
    """

    UPSAMPLING_FACTOR = 10
    HISTORY_LEN = 10
    MIN_CONSISTENCY = 5
    # samples tolerance in original (non-upsampled) index space
    PEAK_TOLERANCE = 5.0
    MAX_RETURN = 5

    def __init__(self):
        # channel -> deque of list[float] (peaks for each measurement)
        self._history: Dict[int, Deque[List[float]]] = defaultdict(
            lambda: deque(maxlen=self.HISTORY_LEN))

    def process_measurement(self, measurement: Measurement, config: WulpusConfig) -> HTTPMeasurementResponse:
        series = pd.Series(measurement['data'])

        series_ups = self._upsample(series, self.UPSAMPLING_FACTOR)
        t_ups = series_ups.index.to_numpy()
        wavelet = self._wavelet_transform(
            series_ups, config, select_level=5)
        wavelet_env = self._envelope(wavelet)

        spacer_reflection = self._calc_spacer_region(8.0, config)

        raw_peaks = self._calc_peaks(
            wavelet_env, config, spacer_reflection=spacer_reflection)
        peaks_x = list(map(lambda p: t_ups[p], raw_peaks))
        filtered_peaks = list(
            filter(lambda x: spacer_reflection[0] < x, peaks_x))

        rx_channels = measurement['rx']
        for ch in rx_channels:
            self._history[int(ch)].append(filtered_peaks)

        peaks_consistent = self._filter_consistent(filtered_peaks, rx_channels)
        peaks_consistent = sorted(peaks_consistent)[: self.MAX_RETURN]

        return HTTPMeasurementResponse(measurement=measurement, peaks=peaks_consistent, wavelet=wavelet_env)

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
        peaks, _ = find_peaks(series_envelope, distance=20 *
                              self.UPSAMPLING_FACTOR, prominence=8)
        return peaks.tolist()

    def _calc_spacer_region(self, spacer_depth: float, config: WulpusConfig) -> np.ndarray:
        youngs_modulus = 4.3 * 1e9
        density = 1.31 * 1000
        poisson_ratio = 0.38
        c_long_solid = np.sqrt(youngs_modulus * (1 - poisson_ratio) /
                               (density * (1 + poisson_ratio) * (1 - 2 * poisson_ratio)))
        t_spacer = spacer_depth / 1000 / c_long_solid
        t_shift = (config.us_config.start_adcsampl -
                   config.us_config.start_ppg) * 1e-6
        t_spacer_start = (t_spacer * 2 - t_shift)
        t_spacer_start_tick = t_spacer_start * config.us_config.sampling_freq
        t_pulse_duration = config.us_config.num_pulses / config.us_config.pulse_freq
        t_pulse_duration_ticks = t_pulse_duration * config.us_config.sampling_freq
        return np.array([math.floor(t_spacer_start_tick), math.ceil(t_spacer_start_tick + t_pulse_duration_ticks)])

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
                    if any(abs(peak - p) <= self.PEAK_TOLERANCE for p in past):
                        appearances += 1
                if appearances >= self.MIN_CONSISTENCY:
                    consistent.append(peak)
                    break  # no need to check other channels once accepted
        return consistent
