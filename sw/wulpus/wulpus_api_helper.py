from __future__ import annotations
import numpy as np
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from wulpus.wulpus_config_models import WulpusConfig

PACKAGE_LEN = 68

TX_RX_MAX_NUM_OF_CONFIGS = 16
# TX RX is configured by activating the
# switches of HV multiplexer
# The arrays below maps transducer channels (0...7)
# to switches IDs (0..15) which we need to activate
RX_MAP = np.array([0, 2, 4, 6, 8, 10, 12, 14])
TX_MAP = np.array([1, 3, 5, 7, 9, 11, 13, 15])


def as_byte(value: int, format: str):
    return np.array([int(value)]).astype(format).tobytes()


def fill_package_to_min_len(bytes_arr: bytes):
    if len(bytes_arr) < PACKAGE_LEN:
        bytes_arr += np.zeros(PACKAGE_LEN - len(bytes_arr)
                              ).astype('<u1').tobytes()
    return bytes_arr


# Lookup table for us to ticks conversion
# Where HSPLL_CLOCK_FREQ = 80MHz
us_to_ticks = {
    # cycles of LFXT (655 - 20ms, 65535 - 2s)
    "dcdc_turnon":       65535 / 2000000,
    "meas_period":       65535 / 2000000,   # same as above
    "start_hvmuxrx":     8,                 # delay in s * 8MHz
    # delay in s * (HSPLL_CLOCK_FREQ / 16) = delay in s * (80MHz / 16)
    "start_ppg":         5,
    "turnon_adc":        5,                 # same as above
    "start_pgainbias":   5,                 # same as above
    "start_adcsampl":    5,                 # same as above
    # delay in s * (HSPLL_CLOCK_FREQ / 256)
    "restart_capt":      5 / 16,
    # delay in s * (HSPLL_CLOCK_FREQ / 64)
    "capt_timeout":      5 / 4,
}


def build_tx_rx_configs(wulpus_config: WulpusConfig):
    tx_cfgs = np.zeros(TX_RX_MAX_NUM_OF_CONFIGS, dtype='<u2')
    rx_cfgs = np.zeros(TX_RX_MAX_NUM_OF_CONFIGS, dtype='<u2')
    i = 0
    for cfg in wulpus_config.tx_rx_config:
        tx_channels = cfg.tx_channels
        rx_channels = cfg.rx_channels
        optimized_switching = cfg.optimized_switching

        # Build bitmasks
        if tx_channels == None or len(tx_channels) == 0:
            tx_cfgs[i] = 0
            tx_channels = []
        else:
            tx_cfgs[i] = np.bitwise_or.reduce(
                np.left_shift(1, TX_MAP[tx_channels]))

        if rx_channels == None or len(rx_channels) == 0:
            rx_cfgs[i] = 0
            rx_channels = []
        else:
            rx_cfgs[i] = np.bitwise_or.reduce(
                np.left_shift(1, RX_MAP[rx_channels]))

        if optimized_switching:
            rx_tx_intersect_ch = list(set(tx_channels) & set(rx_channels))
            rx_only_ch = list(set(rx_tx_intersect_ch) ^ set(rx_channels))
            tx_only_ch = list(set(rx_tx_intersect_ch) ^ set(
                tx_channels))  # kept for clarity

            if len(rx_tx_intersect_ch) > len(rx_only_ch):
                temp_switch_config = np.bitwise_or.reduce(np.left_shift(
                    1, RX_MAP[rx_tx_intersect_ch])) if rx_tx_intersect_ch else 0
                tx_cfgs[i] = np.bitwise_or(
                    tx_cfgs[i], temp_switch_config)
            elif len(rx_only_ch) > 0:
                temp_switch_config = np.bitwise_or.reduce(
                    np.left_shift(1, RX_MAP[rx_only_ch]))
                tx_cfgs[i] = np.bitwise_or(
                    tx_cfgs[i], temp_switch_config)
        i += 1
    return tx_cfgs[:i], rx_cfgs[:i]
