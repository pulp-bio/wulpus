import numpy as np

from wulpus.wulpus_api_helper import (as_byte, build_tx_rx_configs,
                                      fill_package_to_min_len, us_to_ticks)
from wulpus.wulpus_config_models import (PGA_GAIN_REG,
                                         PGA_GAIN_VALUE, USS_CAPT_OVER_SAMPLE_RATES_REG,
                                         USS_CAPTURE_ACQ_RATES_VALUE, WulpusConfig)

START_BYTE_CONF_PACK = 250
START_BYTE_RESTART = 251

DATA_FILE_EXTENSION = '.zip'
CONFIG_FILE_EXTENSION = '.json'

def gen_restart_package():
    bytes_arr = np.array([START_BYTE_RESTART]).astype('<u1').tobytes()
    bytes_arr = fill_package_to_min_len(bytes_arr)
    return bytes_arr


def gen_conf_package(system_config: WulpusConfig):
    config = system_config.us_config
    txrx_config = system_config.tx_rx_config
    # Convert all into correct byte format
    dcdc_turnon_reg = as_byte(config.dcdc_turnon * us_to_ticks["dcdc_turnon"],
                              '<u2')
    meas_period_reg = as_byte(config.meas_period * us_to_ticks["meas_period"],
                              '<u2')
    trans_freq_reg = as_byte(config.trans_freq,
                             '<u4')
    pulse_freq_reg = as_byte(config.pulse_freq,
                             '<u4')
    num_pulses_reg = as_byte(config.num_pulses,
                             '<u1')

    sampling_freq_reg = as_byte(USS_CAPT_OVER_SAMPLE_RATES_REG[USS_CAPTURE_ACQ_RATES_VALUE.index(config.sampling_freq)],
                                '<u2')
    num_samples_reg = as_byte(config.num_samples * 2,
                              '<u2')
    rx_gain_reg = as_byte(PGA_GAIN_REG[PGA_GAIN_VALUE.index(config.rx_gain)],
                          '<u1')

    num_txrx_configs_reg = as_byte(config.num_txrx_configs,
                                   '<u1')

    start_hvmuxrx_reg = as_byte(config.start_hvmuxrx * us_to_ticks["start_hvmuxrx"],
                                '<u2')
    start_ppg_reg = as_byte(config.start_ppg * us_to_ticks["start_ppg"],
                            '<u2')
    turnon_adc_reg = as_byte(config.turnon_adc * us_to_ticks["turnon_adc"],
                             '<u2')
    start_pgainbias_reg = as_byte(config.start_pgainbias * us_to_ticks["start_pgainbias"],
                                  '<u2')
    start_adcsampl_reg = as_byte(config.start_adcsampl * us_to_ticks["start_adcsampl"],
                                 '<u2')
    restart_capt_reg = as_byte(config.restart_capt * us_to_ticks["restart_capt"],
                               '<u2')
    capt_timeout_reg = as_byte(config.capt_timeout * us_to_ticks["capt_timeout"],
                               '<u2')

    # Compose configuration package (order is important)
    bytes_arr = np.array([START_BYTE_CONF_PACK]).astype('<u1').tobytes()
    for param in [dcdc_turnon_reg, meas_period_reg, trans_freq_reg, pulse_freq_reg, num_pulses_reg, sampling_freq_reg, num_samples_reg, rx_gain_reg, num_txrx_configs_reg]:
        bytes_arr += param

    tx_configs, rx_configs = build_tx_rx_configs(system_config)
    for i in range(config.num_txrx_configs):
        bytes_arr += tx_configs[i].astype('<u2').tobytes()
        bytes_arr += rx_configs[i].astype('<u2').tobytes()

    for advanced_param in [start_hvmuxrx_reg, start_ppg_reg, turnon_adc_reg, start_pgainbias_reg, start_adcsampl_reg, restart_capt_reg, capt_timeout_reg]:
        bytes_arr += advanced_param

    bytes_arr = fill_package_to_min_len(bytes_arr)

    return bytes_arr
