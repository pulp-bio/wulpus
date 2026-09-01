"""
   Copyright (C) 2023 ETH Zurich. All rights reserved.
   Author: Sergei Vostrikov, ETH Zurich
           Cedric Hirschi, ETH Zurich
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
from serial.tools.list_ports import comports
from serial.tools.list_ports_common import ListPortInfo
import numpy as np

ACQ_LENGTH_SAMPLES = 400

class WulpusDongle():
    """
    Class representing the Wulpus dongle.
    """


    def __init__(self, port:str = '', timeout_write:int = 3, baudrate:int = 4000000):
        """
        Constructor.

        Arguments
        ---------
        port : str
            Serial port to use.
        timeout_write : int
            Timeout for write operations.
        baudrate : int
            Baudrate to use.
        """
        
        # Initialization of the serial port
        self.__ser__ = serial.Serial()

        self.__ser__.port = port                    # serial port
        self.__ser__.baudrate = baudrate            # baudrate
        self.__ser__.bytesize = serial.EIGHTBITS    # number of bits per bytes
        self.__ser__.parity = serial.PARITY_NONE    # set parity check: no parity
        self.__ser__.stopbits = serial.STOPBITS_ONE # number of stop bits
        self.__ser__.timeout = None                 # read timeout
        self.__ser__.xonxoff = False                # disable software flow control
        self.__ser__.rtscts = False                 # disable hardware (RTS/CTS) flow control
        self.__ser__.dsrdtr = False                 # disable hardware (DSR/DTR) flow control
        self.__ser__.writeTimeout = timeout_write   # timeout for write

        self.acq_length = ACQ_LENGTH_SAMPLES


    def get_available(self):
        """
        Get a list of available devices.
        """

        ports = comports()

        return sorted(ports)


    def open(self, device:ListPortInfo = None):
        """
        Open the device connection.
        """

        if self.__ser__.is_open:
            return True
        
        if device is not None:
            self.__ser__.port = device.device

        if self.__ser__.port == '':
            print("Error: no serial port specified.")
            return False

        try: 
            self.__ser__.open()
        except:
            print("Error while trying to open serial port ", str(self.__ser__.port))
            return False

        return True
    

    def close(self):
        """
        Close the device connection.
        """

        if not self.__ser__.is_open:
            return True

        try:
            self.__ser__.close()
        except:
            print("Error while trying to close serial port ", str(self.__ser__.port))
            return False

        return True
    
    
    def send_config(self, conf_bytes_pack:bytes):
        """
        Send a configuration package to the device.
        """

        if not self.__ser__.is_open:
            print("Error: serial port is not open.")
            return False

        self.__ser__.flushInput()  #flush input buffer, discarding all its contents
        self.__ser__.flushOutput() #flush output buffer, aborting current output 
                               #and discard all that is in buffer

        self.__ser__.write(conf_bytes_pack)

        return True
    

    def __get_rf_data_and_info__(self, bytes_arr:bytes):
    
        rf_arr = np.frombuffer(bytes_arr[7:], dtype='<i2')    
        tx_rx_id = bytes_arr[4]
        acq_nr = np.frombuffer(bytes_arr[5:7], dtype='<u2')[0]

        return rf_arr, acq_nr, tx_rx_id
    
    
    def receive_data(self):
        """
        Receive a data package from the device.
        """

        if not self.__ser__.is_open:
            print("Error: serial port is not open.")
            return None
        
        response_start = self.__ser__.readline()
        
        if len(response_start) == 0:
            return None
        elif response_start[-6:] == b'START\n':
            response = self.__ser__.read(self.acq_length*2 + 7)
            return self.__get_rf_data_and_info__(response)
        else:
            return None
        