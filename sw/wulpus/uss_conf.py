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
MIN_TRANS_FREQ_HZ = 0
MAX_TRANS_FREQ_HZ = 5000000

# Number of unipolar pulses
NUM_EXC_PULSES_MIN = 0
NUM_EXC_PULSES_MAX = 30

# Oversampling rate
# Rates value
USS_CAPTURE_OVER_SAMPLE_RATES = (10, 20, 40, 80, 160)
# Corresponding register values to be sent to HW
USS_CAPT_OVER_SAMPLE_RATES_REG = (0, 1, 2, 3, 4)

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

class WulpusUssConfig():
    def __init__(self,
                 num_acqs=1000,
                 dc_dc_turn_on_time=1000, 
                 meas_period=int(MEAS_PERIOD_MIN*2),
                 trans_freq=225*10**4,
                 pulse_freq=225*10**4,
                 num_pulses=2,
                 over_sampl_rate=USS_CAPTURE_OVER_SAMPLE_RATES[0],
                 samples_size=800,
                 rx_gain=PGA_GAIN[26],
                 tx_rx_conf_len= 1,
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
        self.over_sampl_rate     = int(over_sampl_rate)
        self.samples_size        = int(samples_size)
        self.rx_gain             = rx_gain
        self.tx_rx_conf_len      = int(tx_rx_conf_len)
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
        if (self.trans_freq >= MIN_TRANS_FREQ_HZ) and (self.trans_freq <= MAX_TRANS_FREQ_HZ):
            bytes_arr += np.array([self.trans_freq]).astype('<u4').tobytes()
        else:
            raise ValueError('trans_freq equal to ' + str(self.trans_freq) + " exceeds the allowed range.")
            
        # Pulse excitation frequency
        if (self.pulse_freq >= MIN_TRANS_FREQ_HZ) and (self.pulse_freq <= MAX_TRANS_FREQ_HZ):
            bytes_arr += np.array([self.pulse_freq]).astype('<u4').tobytes()
        else:
            raise ValueError('pulse_freq equal to ' + str(self.pulse_freq) + " exceeds the allowed range.")
            
        # Number of excitation pulses
        if (self.num_pulses >= NUM_EXC_PULSES_MIN) and (self.num_pulses <= NUM_EXC_PULSES_MAX):
            bytes_arr += np.array([self.num_pulses]).astype('<u1').tobytes()
        else:
            raise ValueError('num_pulses equal to ' + str(self.num_pulses) + " exceeds the allowed range.")

        # Oversampling rate
        if (self.over_sampl_rate in USS_CAPTURE_OVER_SAMPLE_RATES):
            
            idx = USS_CAPTURE_OVER_SAMPLE_RATES.index(self.over_sampl_rate)
            bytes_arr += np.array([USS_CAPT_OVER_SAMPLE_RATES_REG[idx]]).astype('<u2').tobytes()
        else:
            raise ValueError('over_sampl_rate equal to ' + str(self.over_sampl_rate) + " exceeds the allowed range.")
            
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
        if (self.tx_rx_conf_len > 0) and (self.tx_rx_conf_len <= TX_RX_MAX_NUM_OF_CONFIGS):
            bytes_arr += np.array([self.tx_rx_conf_len]).astype('<u1').tobytes()
        else:
            raise ValueError('tx_rx_conf_len equal to ' + str(self.tx_rx_conf_len) + " is not allowed.")

        # Write TX and RX configurations
        if len(self.tx_configs) != self.tx_rx_conf_len:
             raise ValueError('Length of tx_configs (' + str(len(self.tx_configs)) + \
                              ") does not match tx_rx_conf_len equal to " + str(self.tx_rx_conf_len))
                
        if len(self.rx_configs) != self.tx_rx_conf_len:
             raise ValueError('Length of rx_configs (' + str(len(self.rx_configs)) + \
                              ") does not match tx_rx_conf_len equal to " + str(self.tx_rx_conf_len))
        
        for i in range(self.tx_rx_conf_len):
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
        
        return bytes_arr

    def get_restart_package(self):

        # Start byte fixed
        bytes_arr =  np.array([START_BYTE_RESTART]).astype('<u1').tobytes()
        
        # Add zeros to match the expected package legth if needed
        if len(bytes_arr) < PACKAGE_LEN:
            bytes_arr += np.zeros(PACKAGE_LEN - len(bytes_arr)).astype('<u1').tobytes()
        
        return bytes_arr
