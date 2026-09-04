"""
   Copyright (C) 2023 ETH Zurich. All rights reserved.
   Author: Cedric Hirschi, ETH Zurich
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

# Oversampling rate
# Rates value
USS_CAPTURE_OVER_SAMPLE_RATES = (10, 20, 40, 80, 160)
# Corresponding register values to be sent to HW
USS_CAPT_OVER_SAMPLE_RATES_REG = (0, 1, 2, 3, 4)
# Corresponding acquisition rates
USS_CAPTURE_ACQ_RATES = [80e6/x for x in USS_CAPTURE_OVER_SAMPLE_RATES]

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

# Lookup table for us to ticks conversion
# Where HSPLL_CLOCK_FREQ = 80MHz
us_to_ticks = {
    "dcdc_turnon":       65535 / 2000000,   # cycles of LFXT (655 - 20ms, 65535 - 2s)
    "meas_period":       65535 / 2000000,   # same as above
    "start_hvmuxrx":     8,                 # delay in s * 8MHz
    "start_ppg":         5,                 # delay in s * (HSPLL_CLOCK_FREQ / 16) = delay in s * (80MHz / 16)
    "turnon_adc":        5,                 # same as above
    "start_pgainbias":   5,                 # same as above
    "start_adcsampl":    5,                 # same as above
    "restart_capt":      5 / 16,            # delay in s * (HSPLL_CLOCK_FREQ / 256)
    "capt_timeout":      5 / 4,             # delay in s * (HSPLL_CLOCK_FREQ / 64)
}


class _ConfigBytes():
    """
    Represents a configuration parameter.

    Attributes:
        config_name (str):      Name of the configuration parameter.
        friendly_name (str):    Reader friendly name of the configuration parameter.
        limit_type (str):       Type of the limit. Can be 'limit' or 'list'.
        min_val (int):          Minimum value of the configuration parameter or list of allowed register values.
        max_val (int):          Maximum value of the configuration parameter or list of allowed friendly values.
        format (str):           Format of the configuration parameter. (e.g. '<u2')
    """

    def __init__(self, config_name, friendly_name, limit_type, min_val, max_val, format):
        self.config_name = config_name
        self.friendly_name = friendly_name
        self.limit_type = limit_type
        self.min_val = min_val
        self.max_val = max_val
        self.format = format

    
    def get_as_bytes(self, value):

        # Get configuration parameter as bytes

        if self.limit_type == 'limit':
            # Check if value is within the allowed range
            if (value < self.min_val) or (value > self.max_val):
                raise ValueError(self.friendly_name + " equal to " + str(value) + " exceeds the allowed range [" + str(self.min_val) + ", " + str(self.max_val) + "].")
        elif self.limit_type == 'list':
            # Check if value is located in the list of allowed values
            if value not in self.min_val:
                raise ValueError(self.friendly_name + " equal to " + str(value) + " is not allowed. Allowed values are: " + str(self.min_val))
        
        return np.array([value]).astype(self.format).tobytes()
  
    
    def get_as_widget(self, value):

        # Get configuration parameter as an ipywidget

        if self.limit_type == 'limit':
            # Return a bounded integer text widget if the parameter is limited to a range

            try:
                # Convert to microseconds (if it needs to be)
                min_val = int(self.min_val / us_to_ticks[self.config_name] + 1)
                max_val = int(self.max_val / us_to_ticks[self.config_name] + 1)
            except:
                # If it is not a time parameter, just use the values as they are
                min_val = int(self.min_val)
                max_val = int(self.max_val)

            if value < min_val:
                print("Warning: " + self.friendly_name + " is set to " + str(value) + " which is below the allowed range [" + str(min_val) + ", " + str(max_val) + "].")
                value = min_val
                print("Setting " + self.friendly_name + " to " + str(value) + ".")
            elif value > max_val:
                print("Warning: " + self.friendly_name + " is set to " + str(value) + " which is above the allowed range [" + str(min_val) + ", " + str(max_val) + "].")
                value = max_val
                print("Setting " + self.friendly_name + " to " + str(value) + ".")

            return widgets.BoundedIntText(
                value=value,
                min=min_val,
                max=max_val,
                step=1,
                description=self.friendly_name,
                disabled=False
            )
        
        elif self.limit_type == 'list':
            # Return a dropdown widget if the parameter is limited to a list of values
            
            return widgets.Dropdown(
                options=self.max_val,
                value=value,
                description=self.friendly_name,
                disabled=False,
            )


# Configuration package representation
# The first list contains basic settings
# The second list contains advanced settings
# The third list contains GUI settings which are not sent to the HW
# Between the two lists there is a list of TX/RX configurations
#                     config_name,         friendly_name,                limit_type, min_val,                           max_val,                        format  
configuration_package = [
    [
        _ConfigBytes('dcdc_turnon',       'DC-DC turn on time [us]',        'limit', 0,                                 65535,                          '<u2'),
        _ConfigBytes('meas_period',       'Acquisition Period [us]',        'limit', 655,                               65535,                          '<u2'),
        _ConfigBytes('trans_freq',        'Transmitter frequency [Hz]',     'limit', 0,                                 5000000,                        '<u4'),
        _ConfigBytes('pulse_freq',        'Pulse frequency [Hz]',           'limit', 0,                                 5000000,                        '<u4'),
        _ConfigBytes('num_pulses',        'Number of pulses',               'limit', 0,                                 30,                             '<u1'),
        _ConfigBytes('sampling_freq',     'Sampling frequency [Hz]',     'list',  USS_CAPT_OVER_SAMPLE_RATES_REG,    USS_CAPTURE_ACQ_RATES,          '<u2'),
        _ConfigBytes('num_samples',       'Number of samples',              'limit', 0,                                 800,                            '<u2'),
        _ConfigBytes('rx_gain',           'Receive (RX) gain [dB]',                   'list',  PGA_GAIN_REG,                      PGA_GAIN,                       '<u1'),
        _ConfigBytes('num_txrx_configs',  'Number of TX/RX configs',        'limit', 0,                                 16,                             '<u1')
    ],
    [
        _ConfigBytes('start_hvmuxrx',     'HV-MUX RX start time [us]',      'limit', 0,                                 65535,                          '<u2'),
        _ConfigBytes('start_ppg',         'PPG start time [us]',            'limit', 0,                                 65535,                          '<u2'),
        _ConfigBytes('turnon_adc',        'ADC turn on time [us]',          'limit', 0,                                 65535,                          '<u2'),
        _ConfigBytes('start_pgainbias',   'PGA in bias start time [us]',    'limit', 0,                                 65535,                          '<u2'),
        _ConfigBytes('start_adcsampl',    'ADC sampling start time [us]',   'limit', 0,                                 65535,                          '<u2'),
        _ConfigBytes('restart_capt',      'Capture restart time [us]',      'limit', 0,                                 65535,                          '<u2'),
        _ConfigBytes('capt_timeout',      'Capture timeout time [us]',      'limit', 0,                                 65535,                          '<u2')
    ],
    [
        _ConfigBytes('num_acqs',          'Number of acquisitions',         'limit', 0,                                 10000000,                        None)
    ]
]