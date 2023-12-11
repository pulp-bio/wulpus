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

import copy
from scipy import signal as ss
from scipy.signal import hilbert
import ipywidgets as widgets
import matplotlib.pyplot as plt
import numpy as np
import time
from threading import Thread
import os.path

from wulpus.uss_conf import WulpusUssConfig

# plt.ioff()

V_TISSUE = 1540 # m/s

LOWER_BOUNDS_MM = 7 # data below this depth will be discarded

LINE_N_SAMPLES = 400

FILE_NAME_BASE = 'data_'

box_layout = widgets.Layout(display='flex',
                flex_flow='column',
                align_items='center',
                width='50%')

class WulpusGuiSingleCh(widgets.VBox):
     
    def __init__(self, com_link, uss_conf, max_vis_fps = 20):
        super().__init__()
        
        # Communication link
        self.com_link = com_link
        
        # Ultrasound Subsystem Configurator
        self.uss_conf = uss_conf
        
        # Allocate memory to store the data and other parameters
        self.data_arr = np.zeros((self.com_link.acq_length, uss_conf.num_acqs), dtype='<i2')
        self.data_arr_bmode = np.zeros((8, self.com_link.acq_length), dtype='<i2')
        self.acq_num_arr = np.zeros(uss_conf.num_acqs, dtype='<u2')
        self.tx_rx_id_arr = np.zeros(uss_conf.num_acqs, dtype=np.uint8)
        
        # For visualization FPS control
        self.vis_fps_period = 1/max_vis_fps
        self.last_timestamp = time.time()
        
        # Extra variables to control visualization
        self.rx_tx_conf_to_display = 0
        
        # For Signal Processing
        self.f_low_cutoff = self.uss_conf.sampling_freq / 2 * 0.1
        self.f_high_cutoff = self.uss_conf.sampling_freq / 2 * 0.9
        self.design_filter(self.uss_conf.sampling_freq,
                           self.f_low_cutoff,
                           self.f_high_cutoff)
 
        # Define widgets
        # Serial port related
        self.ser_scan_button = widgets.Button(description="Scan ports",
                                              disabled=False,
                                              style= {'description_width': 'initial'})
        
        self.ser_open_button = widgets.Button(description="Open port",
                                              disabled=True)
        
        self.port_opened = False
        self.acquisition_running = False
        
        ports = self.com_link.get_available_ports()

        if len(ports) == 0:
            self.ports_dd = widgets.Dropdown(options=['No ports found'],
                                             value='No ports found',
                                             description='Serial port:',
                                             disabled=True,
                                             style= {'description_width': 'initial'})
        else:
            self.ports_dd = widgets.Dropdown(options=ports,
                                             value=ports[0],
                                             description='Serial port:',
                                             disabled=True,
                                             style= {'description_width': 'initial'})
            self.click_scan_ports(self.ser_scan_button)

        # Visualization-related
        self.raw_data_check = widgets.Checkbox(value=True,
                                               description='Show Raw Data',
                                               disabled=True)
    
        self.filt_data_check = widgets.Checkbox(value=False,
                                                description='Show Filtered Data',
                                                disabled=True)
        
        self.env_data_check = widgets.Checkbox(value=False,
                                               description='Show Envelope',
                                               disabled=True)
        
        self.bmode_check = widgets.Checkbox(value=False,
                                            description='Show B-Mode',
                                            disabled=True)
        
        opt = [str(x) for x in range(self.uss_conf.num_txrx_configs)] 
        self.tx_rx_sel_dd = widgets.Dropdown(options=opt,
                                             value=opt[0],
                                             description='Active RX config:',
                                             disabled=True,
                                             style= {'description_width': 'initial'})
        
        self.band_pass_frs = widgets.FloatRangeSlider(value=[self.f_low_cutoff/10**6,\
                                                             self.f_high_cutoff/10**6],
                                                      min=self.f_low_cutoff/10**6,
                                                      max=self.f_high_cutoff/10**6,
                                                      step=0.1,
                                                      description='Band pass (MHz):',
                                                      disabled=True,
                                                      continuous_update=False,
                                                      orientation='horizontal',
                                                      readout=True,
                                                      readout_format='.1f',
                                                      style= {'description_width': 'initial'})
        
        # Progress bar, Start and Stop button, save data checkbox and label
        self.frame_progr_bar = widgets.IntProgress(value=0,
                                                   min=0,
                                                   max=self.uss_conf.num_acqs,
                                                   step=1,
                                                   description='Progress:',
                                                   bar_style='success', # 'success', 'info', 'warning', 'danger' or ''
                                                   orientation='horizontal',
                                                   style= {'description_width': 'initial'})
        
        self.start_stop_button = widgets.Button(description="Start acquisition",
                                                disabled=True)
        
        self.save_data_check = widgets.Checkbox(value=True,
                                                description='Save Data as .npz',
                                                disabled=True)
        
        self.save_data_label = widgets.Label(value='')

        # Setup Visualization
        self.output = widgets.Output()
        self.one_time_fig_config()

        
        # Construct GUI grid
        controls_1 = widgets.VBox([self.raw_data_check, self.filt_data_check, self.env_data_check, self.bmode_check])
 
        controls_2 = widgets.VBox([widgets.HBox([self.ser_open_button, self.ser_scan_button]) , self.ports_dd, self.tx_rx_sel_dd, self.band_pass_frs])
            
        controls = widgets.HBox([controls_1, controls_2])
         
        out_box = widgets.Box([self.output])
        
        progr_ctl_box_1 = widgets.VBox([self.start_stop_button, self.frame_progr_bar])
        progr_ctl_box_2 = widgets.VBox([self.save_data_check, self.save_data_label])
        progr_ctl_box = widgets.HBox([progr_ctl_box_1, progr_ctl_box_2])
        

        main_box = widgets.VBox([controls, out_box, progr_ctl_box])
 

        # Connect callbacks 
    
        # To serial port related buttons
        self.ser_scan_button.on_click(self.click_scan_ports)
        self.ser_open_button.on_click(self.click_open_port)
    
        # To checkboxes
        self.raw_data_check.observe(self.turn_on_off_raw_data_plot, 'value')
        self.filt_data_check.observe(self.turn_on_off_filt_data_plot, 'value')
        self.env_data_check.observe(self.turn_on_off_env_plot, 'value')
        self.bmode_check.observe(self.toggle_bmode, 'value')
        
        # To TX RX select dropdown
        self.tx_rx_sel_dd.observe(self.select_rx_conf_to_plot, 'value')
        
        # To progress slider
        self.band_pass_frs.observe(self.update_band_pass_range, 'value')
        
        # To start stop acqusition button
        self.start_stop_button.on_click(self.click_start_stop_acq)
         
        # add to children
        self.children = [main_box]
        
    def one_time_fig_config(self):
        
        with self.output:
            self.fig, self.ax = plt.subplots(constrained_layout=True, figsize=(8, 4), ncols=1, nrows=1)
        
        if self.bmode_check.value:
            self.setup_bmode_plot()
        else:
            self.setup_amode_plot()

        self.fig.canvas.toolbar_position = 'bottom'
     
    def setup_amode_plot(self):
        self.ax.clear()

        self.raw_data_line, = self.ax.plot(np.zeros(LINE_N_SAMPLES), 
                                           color='blue',
                                           marker='o',
                                           markersize=1,
                                           label='Raw data')
            
        self.filt_data_line, = self.ax.plot(np.zeros(LINE_N_SAMPLES), 
                                            color='green',
                                            marker='o',
                                            markersize=1,
                                            label='Filtered data')
        
        self.envelope_line, = self.ax.plot(np.zeros(LINE_N_SAMPLES), 
                                           color='red',
                                           marker='o',
                                           markersize=1,
                                           label='Envelope')
        
        self.ax.legend(loc='upper right')
        
        self.ax.set_xlabel('Samples')
        self.ax.set_ylabel('ADC digital code')
        self.ax.set_title('A-mode data')

        self.filt_data_line.set_visible(self.filt_data_check.value)
        self.envelope_line.set_visible(self.env_data_check.value)

        self.ax.set_ylim(-3000, 3000)
        self.ax.grid(True)

    def setup_bmode_plot(self):
        self.ax.clear()

        self.bmode_image = self.ax.imshow(np.zeros((8, LINE_N_SAMPLES)),
                                          aspect='auto')

        self.ax.set_xlabel('Depth (mm)')
        self.ax.set_ylabel('Channel number')
        self.ax.set_title('B-mode data')
        # self.bmode_image.set_clim(0, 2)
        self.bmode_image.set_clim(0, 200)
        
        meas_time = LINE_N_SAMPLES / self.uss_conf.sampling_freq
        meas_depth = meas_time * V_TISSUE * 1000 / 2
        self.bmode_image.set_extent((LOWER_BOUNDS_MM, meas_depth, 0.5, 7.5))

    # Callbacks
    
    def click_scan_ports(self, b):
        # Update drop-down for ports and make it enabled
        self.found_devices = self.com_link.get_available_ports()

        if len(self.found_devices) == 0:
            self.ports_dd.options = ['No ports found']
            self.ports_dd.value = 'No ports found'
            self.ports_dd.disabled = True
            self.ser_open_button.disabled = True
        else:
            self.ports_dd.options = [device.description for device in self.found_devices]
            self.ports_dd.value = self.found_devices[0].description
            self.ports_dd.disabled = False
            self.ser_open_button.disabled = False
        
    def click_open_port(self, b):
        
        if not self.port_opened and len(self.ports_dd.options) > 0:
            device = self.found_devices[self.ports_dd.index]
            self.com_link.ser.port = device.device
            if not self.com_link.open_serial_port():
                return
            
            b.description = "Close port"
            self.port_opened = True
            self.start_stop_button.disabled = False
            
        else :
            self.com_link.ser.close()
            b.description = "Open port"
            self.port_opened = False
            self.start_stop_button.disabled = True
        
    
    def turn_on_off_raw_data_plot(self, change):
        
        self.raw_data_line.set_visible(change.new)
        
    def turn_on_off_filt_data_plot(self, change):
        
        self.filt_data_line.set_visible(change.new)
        
    def turn_on_off_env_plot(self, change):
        
        self.envelope_line.set_visible(change.new)

    def toggle_bmode(self, change):
 
        if change.new:
            self.setup_bmode_plot()
            self.tx_rx_sel_dd.disabled = True
        else:
            self.setup_amode_plot()
            self.tx_rx_sel_dd.disabled = False
            
        self.fig.canvas.draw()
        self.fig.canvas.flush_events()
        
    def select_rx_conf_to_plot(self, change):
        
        self.rx_tx_conf_to_display = int(change.new)
 
    def update_band_pass_range(self, change):
        
        self.design_filter(self.uss_conf.sampling_freq,
                           change.new[0]*10**6,
                           change.new[1]*10**6)
        
        
    def click_start_stop_acq(self, b):

        if not self.acquisition_running:
            # Enable the widgets active during acquisition
            self.raw_data_check.disabled  = False
            self.filt_data_check.disabled = False
            self.env_data_check.disabled  = False
            self.bmode_check.disabled     = False
            self.tx_rx_sel_dd.disabled    = False
            self.band_pass_frs.disabled   = False
            self.save_data_check.disabled = False
            
            # Disable serial port related widgets
            self.ser_open_button.disabled = True
            
            # Clean Save data label
            self.save_data_label.value = ''
            
            
            # Change state of the button
            b.description = "Stop acquisition"

            # Declare that acquisition is running
            self.acquisition_running = True

            # Run data acquisition loop
            self.current_data = None
            self.acquisition_thread = Thread(target=self.run_acquisition_loop)
            self.acquisition_thread.start()
            
        else:
            # Stop acquisition, thread will stop by itself
            self.acquisition_running = False
            
            # Change state of the button
            b.description = "Start acquisition"

            # Disable the widgets when not acquiring
            self.raw_data_check.disabled  = True
            self.filt_data_check.disabled = True
            self.env_data_check.disabled  = True
            self.bmode_check.disabled     = True
            self.tx_rx_sel_dd.disabled    = True
            self.band_pass_frs.disabled   = True
            self.save_data_check.disabled = True
            
            # Enable serial port related widgets again
            self.ser_open_button.disabled = False
        
    def run_acquisition_loop(self):

#         self.fig.show()
        
        # Clean data buffer
        acq_length = self.com_link.acq_length
        number_of_acq = self.uss_conf.num_acqs
        self.data_arr = np.zeros((acq_length, number_of_acq), dtype='<i2')
        self.acq_num_arr = np.zeros(number_of_acq, dtype='<u2')
        self.tx_rx_id_arr = np.zeros(number_of_acq, dtype=np.uint8)
        # Acquisition counter
        self.data_cnt=0
        
        # Send a restart command (if system is already running)
        self.com_link.send_config_package(self.uss_conf.get_restart_package())
        
        # Wait 2.5 seconds (much larger than max measurement period = 2s)
        time.sleep(2.5)
        
        # Generate and send a configuration package
        try:
            self.com_link.send_config_package(self.uss_conf.get_conf_package())
        except ValueError as e:
            self.save_data_label.value = str(e)
            self.acquisition_running = False
            if self.ser_open_button.disabled:
                self.click_start_stop_acq(self.start_stop_button)
            return

        self.visualize = True
        self.current_data = None
        self.current_amode_data = None
        t2 = Thread(target=self.visualization, args=(number_of_acq,))
        t2.start()
        
        # Readout data in a loop
        while self.data_cnt < number_of_acq and self.acquisition_running:
            # Receive the data
            data = self.com_link.receive_data()
            if data is not None:

                self.current_data = data

                if data[2] == self.rx_tx_conf_to_display and not self.bmode_check.value:
                    self.current_amode_data = data[0]
                
                # Save data
                self.data_arr[:, self.data_cnt] = data[0]

                # and other params
                self.acq_num_arr[self.data_cnt] = data[1]
                self.tx_rx_id_arr[self.data_cnt] = data[2]

                # Save data to specific z
                self.data_arr_bmode[self.tx_rx_id_arr[self.data_cnt]] = self.get_envelope(
                    self.filter_data(data[0]))
                
                self.data_cnt = self.data_cnt + 1

        self.visualize = False
        t2.join()

        self.com_link.send_config_package(self.uss_conf.get_restart_package())    
                
        # Save data to file if needed
        if (self.save_data_check.value):
            self.save_data_to_file()
                
        # Stop acquisition
        if self.ser_open_button.disabled:
            self.click_start_stop_acq(self.start_stop_button)

        # self.click_open_port(self.ser_open_button) # if you want to close the port after acquisition
    
    def visualization(self, number_of_acq):

        self.frame_progr_bar.max = number_of_acq
        
        while self.visualize:
            # Update the visualization

            # B-mode
            if self.bmode_check.value:
                if self.current_data is None:
                    continue
                try:
                    # self.bmode_image.set_data(np.log10(np.add(self.data_arr_bmode, 0.1)))                                # log scale
                    self.bmode_image.set_data(self.data_arr_bmode[:,10*LOWER_BOUNDS_MM:])                                # linear scale
                except:
                    # B-mode graph is not initialized yet
                    pass

            # Check the id of RX TX config
            else:
                if self.current_amode_data is None:
                    continue
                filt_data = None

                # Raw RF data
                if (self.raw_data_check.value):
                    self.raw_data_line.set_ydata(self.current_amode_data)
                    
                # Filtered data 
                if (self.filt_data_check.value):
                    filt_data = self.filter_data(self.current_amode_data)
                    self.filt_data_line.set_ydata(filt_data)
                    
                # Envelope
                if (self.env_data_check.value):
                    if filt_data is None:
                        filt_data = self.filter_data(self.current_amode_data)
                    self.envelope_line.set_ydata(self.get_envelope(filt_data))

            self.fig.canvas.draw()
            # This will run the GUI event
            # loop until all UI events
            # currently waiting have been processed
            self.fig.canvas.flush_events()
            
            # Update progress bar
            self.frame_progr_bar.description = 'Progress: ' + str(self.data_cnt) + '/' + str(number_of_acq)   
            self.frame_progr_bar.value = self.data_cnt
            # self.save_data_label.value = 'FPS: ' + str(1/(time.time() - self.last_timestamp))
            self.last_timestamp = time.time()

            
    # Design bandpass filter
    def design_filter(self, 
                      f_sampling, 
                      f_low_cutoff, 
                      f_high_cutoff, 
                      trans_width=0.2*10**6, 
                      n_taps = 31):
        
        temp = [0, f_low_cutoff - trans_width, \
                f_low_cutoff, f_high_cutoff, \
                f_high_cutoff + trans_width, \
                f_sampling/2]
        
        self.filt_b = ss.remez(n_taps, 
                               temp,
                               [0, 1,  0], 
                               Hz=f_sampling, 
                               maxiter=2500)
        self.filt_a = 1
        
    
    def filter_data(self, data_in):
        return ss.filtfilt(self.filt_b, self.filt_a, data_in)
    
    def get_envelope(self, data_in):
        return np.abs(hilbert(data_in))
    
    def save_data_to_file(self):
        
        # Check filename
        for i in range(100):
            filename = FILE_NAME_BASE + str(i) + '.npz'
            if not os.path.isfile(filename):
                break
                
        # Save numpy data array to file
        np.savez(filename[:-4], 
                 data_arr=self.data_arr,
                 acq_num_arr=self.acq_num_arr,
                 tx_rx_id_arr=self.tx_rx_id_arr)
                
        self.save_data_label.value = 'Data saved in ' + filename