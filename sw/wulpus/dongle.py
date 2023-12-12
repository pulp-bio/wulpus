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

import serial
import serial.tools.list_ports
import numpy as np

ACQ_LENGTH_SAMPLES = 400

class WulpusDongle():
    def __init__(self, port='', timeout_write=3, baudrate=4000000):
        
        # Initialization of the serial port
        self.ser = serial.Serial()
        self.ser.port = port
        self.ser.baudrate = baudrate
        self.ser.bytesize = serial.EIGHTBITS    # number of bits per bytes
        self.ser.parity = serial.PARITY_NONE    # set parity check: no parity
        self.ser.stopbits = serial.STOPBITS_ONE # number of stop bits
        self.ser.timeout = None         # read timeout
        self.ser.xonxoff = False                # disable software flow control
        self.ser.rtscts = False                 # disable hardware (RTS/CTS) flow control
        self.ser.dsrdtr = False                 # disable hardware (DSR/DTR) flow control
        self.ser.writeTimeout = timeout_write   #timeout for write

        self.acq_length = ACQ_LENGTH_SAMPLES


    def get_available_ports(self):
        ports = serial.tools.list_ports.comports()

        return sorted(ports)

    def open_serial_port(self):

        try: 
            self.ser.open()
        except:
            print("Error while trying to open serial port ", str(self.ser.port))
            return False

        return True
        
        
    def get_rf_data_and_info(self, bytes_arr):
    
        rf_arr = np.frombuffer(bytes_arr[7:], dtype='<i2')    
        tx_rx_id = bytes_arr[4]
        acq_nr = np.frombuffer(bytes_arr[5:7], dtype='<u2')[0]

        return rf_arr, acq_nr, tx_rx_id
    
    def send_config_package(self, conf_bytes_pack):
        self.ser.flushInput()  #flush input buffer, discarding all its contents
        self.ser.flushOutput() #flush output buffer, aborting current output 
                               #and discard all that is in buffer

        self.ser.write(conf_bytes_pack)

        return
    
    def receive_data(self):
        
        response_start = self.ser.readline()
        
        if len(response_start) == 0:
            return None
        elif response_start[-6:] == b'START\n':
            response = self.ser.read(self.acq_length*2 + 7)
            return self.get_rf_data_and_info(response)