"""
   Copyright (C) 2023 ETH Zurich. All rights reserved.
   Author: Sergei Vostrikov, ETH Zurich
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   SPDX-License-Identifier: Apache-2.0
"""

import numpy as np
import ipywidgets as widgets

# CONSTANTS

# Protocol related
START_BYTE_CONF_PACK = 250
START_BYTE_RESTART   = 251

# Measurement period in cycles of LFXT
# 20 ms
MEAS_PERIOD_MIN = 655
# 2 s
MEAS_PERIOD_MAX = 65535


# Tansducer frequency and excitation
MIN_TRANS_FREQ = 0
MAX_TRANS_FREQ = 5000000

# Number of unipolar pulses
NUM_EXC_PULSES_MIN = 0
NUM_EXC_PULSES_MAX = 30

# Oversampling rate
# Rates value
USS_CAPTURE_OVER_SAMPLE_RATES = (10, 20, 40, 80, 160)
# Corresponding register values to be sent to HW
USS_CAPT_OVER_SAMPLE_RATES_REG = (0, 1, 2, 3, 4)
# Corresponding acquisition rates
USS_CAPTURE_ACQ_RATES = [80e6/x for x in USS_CAPTURE_OVER_SAMPLE_RATES]

# Sample size
SAMPLE_SIZE_MAX = 800

# PGA RX gain
# Gain in dB
PGA_GAIN = (-6.5, -5.5, -4.6, -4.1, -3.3, -2.3, -1.4, -0.8,
            0.1, 1.0, 1.9, 2.6, 3.5, 4.4, 5.2, 6.0, 6.8, 7.7,
            8.7, 9.0, 9.8, 10.7, 11.7, 12.2, 13, 13.9, 14.9,
            15.5, 16.3, 17.2, 18.2, 18.8, 19.6, 20.5, 21.5, 
            22, 22.8, 23.6, 24.6, 25.0, 25.8, 26.7, 27.7,
            28.1, 28.9, 29.8, 30.8)

# Register value to write to HW
PGA_GAIN_REG = tuple(np.arange(17, 64))

# TX RX Configs
TX_RX_MAX_NUM_OF_CONFIGS = 16

# Maximum length of the configuration package
PACKAGE_LEN  = 68

# Lookup table for us to ticks conversion
# Where HSPLL_CLOCK_FREQ = 80MHz
us_to_ticks = {
    "dcdc_turnon":       80,                # delay in s * 80MHz (TBC)
    "meas_period":       65535 / 2000000,   # cycles of LFXT (655 - 20ms, 65535 - 2s)
    "start_hvmuxrx":    8,                  # delay in s * 8MHz
    "start_ppg":         5,                 # delay in s * (HSPLL_CLOCK_FREQ / 16) = delay in s * (80MHz / 16)
    "turnon_adc":        5,                 # same as above
    "start_pgainbias":   5,                 # same as above
    "start_adcsampl":    5,                 # same as above
    "restart_capt":      5 / 16,            # delay in s * (HSPLL_CLOCK_FREQ / 256)
    "capt_timeout":      5 / 4,             # delay in s * (HSPLL_CLOCK_FREQ / 64)
}

class _ConfigBytes():
    def __init__(self, config_name, friendly_name, limit_type, min_val, max_val, format):
        self.config_name = config_name
        self.friendly_name = friendly_name
        self.limit_type = limit_type
        self.min_val = min_val
        self.max_val = max_val
        self.format = format

    # get as bytes
    def get_as_bytes(self, value):
        if self.limit_type == 'limit':
            if (value < self.min_val) or (value > self.max_val):
                raise ValueError(self.friendly_name + " equal to " + str(value) + " exceeds the allowed range [" + str(self.min_val) + ", " + str(self.max_val) + "].")
        elif self.limit_type == 'list':
            if value not in self.min_val:
                raise ValueError(self.friendly_name + " equal to " + str(value) + " is not allowed. Allowed values are: " + str(self.min_val))
        return np.array([value]).astype(self.format).tobytes()
    
    def get_as_widget(self, value):
        if self.limit_type == 'limit':
            try:
                min_val = self.min_val / us_to_ticks[self.config_name]
                max_val = self.max_val / us_to_ticks[self.config_name]
            except:
                min_val = self.min_val
                max_val = self.max_val
            return widgets.BoundedIntText(
                value=value,
                min=min_val,
                max=max_val,
                step=1,
                description=self.friendly_name,
                disabled=False
            )
        elif self.limit_type == 'list':
            return widgets.Dropdown(
                options=self.max_val,
                value=value,
                description=self.friendly_name,
                disabled=False,
            )
        
configuration_package = [
    [
        _ConfigBytes('dcdc_turnon',       'DC-DC turn on time [us]',       'limit', 0,   65535,                      '<u2'),
        _ConfigBytes('meas_period',       'Measuring period [us]',       'limit', 655, 65535,                      '<u2'),
        _ConfigBytes('trans_freq',        'Transmitter frequency [Hz]',        'limit', 0,   5000000,                    '<u4'),
        _ConfigBytes('pulse_freq',        'Pulse frequency [Hz]',        'limit', 0,   5000000,                    '<u4'),
        _ConfigBytes('num_pulses',        'Number of pulses',        'limit', 0,   30,                         '<u1'),
        _ConfigBytes('oversampling_rate', 'Acquisition frequency', 'list',  USS_CAPT_OVER_SAMPLE_RATES_REG, USS_CAPTURE_OVER_SAMPLE_RATES, '<u2'),
        _ConfigBytes('num_samples',       'Number of samples',       'limit', 0,   800,                        '<u2'),
        _ConfigBytes('rx_gain',           'RX gain [dB]',           'list',  PGA_GAIN_REG, PGA_GAIN,                   '<u1'),
        _ConfigBytes('num_txrx_configs',  'Number of TX/RX configs',  'limit', 0,   16,                         '<u1')
    ],
    [
        _ConfigBytes('start_hvmuxrx',     'HV-MUX RX start time [us]',     'limit', 0,   65535,                      '<u2'),
        _ConfigBytes('start_ppg',         'PPG start time [us]',         'limit', 0,   65535,                      '<u2'),
        _ConfigBytes('turnon_adc',        'ADC turn on time [us]',        'limit', 0,   65535,                      '<u2'),
        _ConfigBytes('start_pgainbias',   'PGA in bias start time [us]',   'limit', 0,   65535,                      '<u2'),
        _ConfigBytes('start_adcsampl',    'ADC sampling start time [us]',    'limit', 0,   65535,                      '<u2'),
        _ConfigBytes('restart_capt',      'Capture restart time [us]',      'limit', 0,   65535,                      '<u2'),
        _ConfigBytes('capt_timeout',      'Capture timeout time [us]',      'limit', 0,   65535,                      '<u2')
    ]
]

class WulpusUssConfig():
    def __init__(self,
                 num_acqs=100,
                 dcdc_turnon=80, 
                 meas_period=321965,
                 trans_freq=225e4,
                 pulse_freq=225e4,
                 num_pulses=2,
                 sampling_freq=USS_CAPTURE_ACQ_RATES[0],
                 num_samples=800,
                 rx_gain=PGA_GAIN[-10],
                 num_txrx_configs= 1,
                 tx_configs = [0], 
                 rx_configs = [0], 
                 start_hvmuxrx=500, 
                 start_ppg=500,     
                 turnon_adc=5,   
                 start_pgainbias=5,
                 start_adcsampl=503,
                 restart_capt=3000,
                 capt_timeout=3000):
        
        # check if sampling frequency is valid
        if sampling_freq not in USS_CAPTURE_ACQ_RATES:
            raise ValueError('Sampling frequency of ' + str(sampling_freq) + ' is not allowed.\nAllowed values are: ' + str(USS_CAPTURE_ACQ_RATES))
        # check if rx gain is valid
        if rx_gain not in PGA_GAIN:
            raise ValueError('RX gain of ' + str(rx_gain) + ' is not allowed.\nAllowed values are: ' + str(PGA_GAIN))
        
        self.num_acqs            = int(num_acqs)
        self.dcdc_turnon  = int(dcdc_turnon)
        self.meas_period         = int(meas_period)
        self.trans_freq          = int(trans_freq)
        self.pulse_freq          = int(pulse_freq)
        self.num_pulses          = int(num_pulses)
        self.sampling_freq     = int(sampling_freq)
        self.oversampling_rate = int(USS_CAPTURE_OVER_SAMPLE_RATES[USS_CAPTURE_ACQ_RATES.index(self.sampling_freq)])
        self.num_samples        = int(num_samples)      
        self.rx_gain             = rx_gain
        self.num_txrx_configs      = int(num_txrx_configs)
        self.tx_configs          = np.array(tx_configs).astype('<u2')
        self.rx_configs          = np.array(rx_configs).astype('<u2')
        
        # Advanced settings
        self.start_hvmuxrx   = start_hvmuxrx
        self.start_ppg         = start_ppg
        self.turnon_adc       = turnon_adc
        self.start_pgainbias = start_pgainbias
        self.start_adcsampl   = start_adcsampl
        self.restart_capt      = restart_capt
        self.capt_timeout      = capt_timeout

        self.convert_to_registers() # convert to register saveable values
        _ = self.get_conf_package() # check if configuration package is valid
        print("Configuration package is valid.")

    def convert_to_registers(self):

        # convert to register saveable values
        
        self.dcdc_turnon_reg = int(self.dcdc_turnon * us_to_ticks["dcdc_turnon"])
        self.meas_period_reg = int(self.meas_period * us_to_ticks["meas_period"])
        self.trans_freq_reg = int(self.trans_freq)
        self.pulse_freq_reg = int(self.pulse_freq)
        self.num_pulses_reg = int(self.num_pulses)
        self.oversampling_rate_reg = int(USS_CAPT_OVER_SAMPLE_RATES_REG[USS_CAPTURE_ACQ_RATES.index(self.sampling_freq)])
        self.num_samples_reg = int(self.num_samples)
        self.rx_gain_reg = int(PGA_GAIN_REG[PGA_GAIN.index(self.rx_gain)])
        self.num_txrx_configs_reg = int(self.num_txrx_configs)
        self.start_hvmuxrx_reg = int(self.start_hvmuxrx * us_to_ticks["start_hvmuxrx"])
        self.start_ppg_reg = int(self.start_ppg * us_to_ticks["start_ppg"])
        self.turnon_adc_reg = int(self.turnon_adc * us_to_ticks["turnon_adc"])
        self.start_pgainbias_reg = int(self.start_pgainbias * us_to_ticks["start_pgainbias"])
        self.start_adcsampl_reg = int(self.start_adcsampl * us_to_ticks["start_adcsampl"])
        self.restart_capt_reg = int(self.restart_capt * us_to_ticks["restart_capt"])
        self.capt_timeout_reg = int(self.capt_timeout * us_to_ticks["capt_timeout"])

    def get_conf_package(self):

        # Start byte fixed
        bytes_arr =  np.array([START_BYTE_CONF_PACK]).astype('<u1').tobytes()


        self.convert_to_registers()


        for param in configuration_package[0]:
            value = getattr(self, param.config_name + "_reg")
            bytes_arr += param.get_as_bytes(value)
                
            
        
        for i in range(self.num_txrx_configs):
            bytes_arr += self.tx_configs[i].astype('<u2').tobytes()
            bytes_arr += self.rx_configs[i].astype('<u2').tobytes()


        for param in configuration_package[1]:
            value = getattr(self, param.config_name + "_reg")
            bytes_arr += param.get_as_bytes(value)


        # Add zeros to match the expected package legth if needed
        if len(bytes_arr) < PACKAGE_LEN:
            bytes_arr += np.zeros(PACKAGE_LEN - len(bytes_arr)).astype('<u1').tobytes()

        # for byte in bytes_arr:
        #     # print hex value
        #     print(f'{byte:02x}', end=' ')
        # print()
        
        return bytes_arr
    
    def get_restart_package(self):
            
        # Start byte fixed
        bytes_arr =  np.array([START_BYTE_RESTART]).astype('<u1').tobytes()
        
        # Add zeros to match the expected package legth if needed
        if len(bytes_arr) < PACKAGE_LEN:
            bytes_arr += np.zeros(PACKAGE_LEN - len(bytes_arr)).astype('<u1').tobytes()
        
        return bytes_arr

class _WulpusUssConfig():
    """
    Old API, use WulpusUssConfig instead.
    """
    def __init__(self,
                 num_acqs=1000,
                 dc_dc_turn_on_time=1000, 
                 meas_period=int(MEAS_PERIOD_MIN*2),
                 trans_freq=225*10**4,
                 pulse_freq=225*10**4,
                 num_pulses=2,
                 oversampling_rate=USS_CAPTURE_OVER_SAMPLE_RATES[0],
                 samples_size=800,
                 rx_gain=PGA_GAIN[26],
                 num_txrx_configs= 1,
                 tx_configs = [0], # TBD
                 rx_configs = [0], # TBD
                 start_hv_mux_rx_cnt=4000, # 500 uS
                 start_ppg_cnt=2500,       # ~500 uS
                 turn_on_adc_cnt=25,   
                 start_pga_in_bias_cnt=25,
                 start_adc_sampl_cnt=2514,
                 restart_capt_cnt=937,
                 capt_timeout_cnt=3750):
        
        
        self.num_acqs            = int(num_acqs)
        self.dc_dc_turn_on_time  = int(dc_dc_turn_on_time)
        self.meas_period         = int(meas_period)
        self.trans_freq          = int(trans_freq)
        self.pulse_freq          = int(pulse_freq)
        self.num_pulses          = int(num_pulses)
        self.oversampling_rate     = int(oversampling_rate)
        self.samples_size        = int(samples_size)
        self.rx_gain             = rx_gain
        self.num_txrx_configs      = int(num_txrx_configs)
        self.tx_configs          = np.array(tx_configs).astype('<u2')
        self.rx_configs          = np.array(rx_configs).astype('<u2')
        
        # Advanced settings
        self.start_hv_mux_rx_cnt   = start_hv_mux_rx_cnt
        self.start_ppg_cnt         = start_ppg_cnt
        self.turn_on_adc_cnt       = turn_on_adc_cnt
        self.start_pga_in_bias_cnt = start_pga_in_bias_cnt
        self.start_adc_sampl_cnt   = start_adc_sampl_cnt
        self.restart_capt_cnt      = restart_capt_cnt
        self.capt_timeout_cnt      = capt_timeout_cnt
        
        return
    
    # Construct and output configuration package
    # For WULPUS Ultrasound subsystem
    def get_conf_package(self):

        # Start byte fixed
        bytes_arr =  np.array([START_BYTE_CONF_PACK]).astype('<u1').tobytes()
        
        # DC DC turn on time
        if (self.dc_dc_turn_on_time > 0) and (self.dc_dc_turn_on_time <= 65535):
            bytes_arr += np.array([self.dc_dc_turn_on_time]).astype('<u2').tobytes()
        else:
            raise ValueError('dc_dc_turn_on_time equal to ' + str(self.dc_dc_turn_on_time) + " exceeds the allowed range.")
            
        # Measurement period
        if (self.meas_period >= MEAS_PERIOD_MIN) and (self.meas_period <= MEAS_PERIOD_MAX):
            bytes_arr += np.array([self.meas_period]).astype('<u2').tobytes()
        else:
            raise ValueError('meas_period equal to ' + str(self.meas_period) + " exceeds the allowed range.")
            
        # Transducer frequency
        if (self.trans_freq >= MIN_TRANS_FREQ) and (self.trans_freq <= MAX_TRANS_FREQ):
            bytes_arr += np.array([self.trans_freq]).astype('<u4').tobytes()
        else:
            raise ValueError('trans_freq equal to ' + str(self.trans_freq) + " exceeds the allowed range.")
            
        # Pulse excitation frequency
        if (self.pulse_freq >= MIN_TRANS_FREQ) and (self.pulse_freq <= MAX_TRANS_FREQ):
            bytes_arr += np.array([self.pulse_freq]).astype('<u4').tobytes()
        else:
            raise ValueError('pulse_freq equal to ' + str(self.pulse_freq) + " exceeds the allowed range.")
            
        # Number of excitation pulses
        if (self.num_pulses >= NUM_EXC_PULSES_MIN) and (self.num_pulses <= NUM_EXC_PULSES_MAX):
            bytes_arr += np.array([self.num_pulses]).astype('<u1').tobytes()
        else:
            raise ValueError('num_pulses equal to ' + str(self.num_pulses) + " exceeds the allowed range.")

        # Oversampling rate
        if (self.oversampling_rate in USS_CAPTURE_OVER_SAMPLE_RATES):
            
            idx = USS_CAPTURE_OVER_SAMPLE_RATES.index(self.oversampling_rate)
            bytes_arr += np.array([USS_CAPT_OVER_SAMPLE_RATES_REG[idx]]).astype('<u2').tobytes()
        else:
            raise ValueError('oversampling_rate equal to ' + str(self.oversampling_rate) + " exceeds the allowed range.")
            
        # Sample size
        if (self.samples_size > 0) and (self.samples_size <= SAMPLE_SIZE_MAX):
            bytes_arr += np.array([self.samples_size]).astype('<u2').tobytes()
        else:
            raise ValueError('samples_size equal to ' + str(self.samples_size) + " is not allowed.")
            
        # RX gain
        if (self.rx_gain in PGA_GAIN):
            
            idx = PGA_GAIN.index(self.rx_gain)
            bytes_arr += np.array([PGA_GAIN_REG[idx]]).astype('<u1').tobytes()
        else:
            raise ValueError('rx_gain equal to ' + str(self.rx_gain) + " is not allowed.")

        # TX RX config length
        if (self.num_txrx_configs > 0) and (self.num_txrx_configs <= TX_RX_MAX_NUM_OF_CONFIGS):
            bytes_arr += np.array([self.num_txrx_configs]).astype('<u1').tobytes()
        else:
            raise ValueError('num_txrx_configs equal to ' + str(self.num_txrx_configs) + " is not allowed.")

        # Write TX and RX configurations
        if len(self.tx_configs) != self.num_txrx_configs:
             raise ValueError('Length of tx_configs (' + str(len(self.tx_configs)) + \
                              ") does not match num_txrx_configs equal to " + str(self.num_txrx_configs))
                
        if len(self.rx_configs) != self.num_txrx_configs:
             raise ValueError('Length of rx_configs (' + str(len(self.rx_configs)) + \
                              ") does not match num_txrx_configs equal to " + str(self.num_txrx_configs))
        
        for i in range(self.num_txrx_configs):
            bytes_arr += self.tx_configs[i].astype('<u2').tobytes()
            bytes_arr += self.rx_configs[i].astype('<u2').tobytes()
            
            
        # Add advanced settings
        bytes_arr += np.array([self.start_hv_mux_rx_cnt]).astype('<u2').tobytes()
        bytes_arr += np.array([self.start_ppg_cnt]).astype('<u2').tobytes()
        bytes_arr += np.array([self.turn_on_adc_cnt]).astype('<u2').tobytes()
        bytes_arr += np.array([self.start_pga_in_bias_cnt]).astype('<u2').tobytes()
        bytes_arr += np.array([self.start_adc_sampl_cnt]).astype('<u2').tobytes()
        bytes_arr += np.array([self.restart_capt_cnt]).astype('<u2').tobytes()
        bytes_arr += np.array([self.capt_timeout_cnt]).astype('<u2').tobytes()

        # Add zeros to match the expected package legth if needed
        if len(bytes_arr) < PACKAGE_LEN:
            bytes_arr += np.zeros(PACKAGE_LEN - len(bytes_arr)).astype('<u1').tobytes()

        for byte in bytes_arr:
            # print hex value
            print(f'{byte:02x}', end=' ')
        print()
        
        return bytes_arr

    def get_restart_package(self):

        # Start byte fixed
        bytes_arr =  np.array([START_BYTE_RESTART]).astype('<u1').tobytes()
        
        # Add zeros to match the expected package legth if needed
        if len(bytes_arr) < PACKAGE_LEN:
            bytes_arr += np.zeros(PACKAGE_LEN - len(bytes_arr)).astype('<u1').tobytes()
        
        return bytes_arr