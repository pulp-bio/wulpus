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

from wulpus_jptnbk.uss_conf import *
import ipywidgets as widgets
import json

DEFAULT_FILENAME = 'uss_config'


class WulpusUssConfigGUI(widgets.VBox, WulpusUssConfig):
    """
    A GUI for managing the USS configuration parameters of Wulpus.

    Attributes:
        num_txrx_configs (int): Number of TX RX configurations.
        tx_configs (list): List of TX configurations. Each TX configuration is a list of TX channel IDs.
        rx_configs (list): List of RX configurations. Each RX configuration is a list of RX channel IDs.
    """

    def __init__(self, config: WulpusUssConfig):
        super().__init__()
        self.__dict__.update(config.__dict__)

        self.output = widgets.Output()

        self.init_inputs()

        self.info_label = widgets.Label(value="")

        # File management entries

        self.entry_filename = widgets.Text(
            value=DEFAULT_FILENAME,
            description='Filename:',
            disabled=False
        )

        self.save_button = widgets.Button(
            description='Save',
            disabled=False,
            tooltip='Save the configuration to a JSON file',
            icon='save'
        )
        self.save_button.on_click(self.save_json)

        self.load_button = widgets.Button(
            description='Load',
            disabled=False,
            tooltip='Load the configuration from a JSON file',
            icon='upload'
        )
        self.load_button.on_click(self.load_json)

        # Arrange the widgets in a VBox
        self.children = [
            widgets.HBox([widgets.VBox(self.entries_left),
                         widgets.VBox(self.entries_right)]),
            widgets.HBox(
                [self.entry_filename, self.save_button, self.load_button]),
            self.info_label,
            self.output]

    def get_param(self, param_name):
        """
        Returns the value of a parameter.

        Args:
            param_name (str): Name of the parameter.

        Returns:
            The _ConfigBytes object of the parameter.
        """

        for param in configuration_package[0]:
            # Check if the parameter is a basic setting

            if param.config_name == param_name:
                # Return the parameter
                return param

        for param in configuration_package[1]:
            # Check if the parameter is an advanced setting

            if param.config_name == param_name:
                # Return the parameter
                return param

        for param in configuration_package[2]:
            # Check if the parameter is a GUI setting

            if param.config_name == param_name:
                # Return the parameter
                return param

        # Parameter not found
        return None

    def init_inputs(self):
        """
        Initializes the input widgets.
        """

        input_style = {'description_width': '60%', 'width': '30%'}

        self.entries_left = []
        self.entries_right = []

        entries_acq = []
        entries_exc = []
        entries_adv = []

        entries_acq.append(widgets.HTML(value="<b>Measurement settings</b>"))
        entries_acq.append(self.get_param(
            'num_acqs').get_as_widget(self.num_acqs))
        entries_acq.append(self.get_param(
            'meas_period').get_as_widget(self.meas_period))
        entries_acq.append(self.get_param(
            'sampling_freq').get_as_widget(self.sampling_freq))
        entries_acq.append(self.get_param(
            'num_samples').get_as_widget(self.num_samples))
        entries_acq.append(self.get_param(
            'rx_gain').get_as_widget(self.rx_gain))

        entries_exc.append(widgets.HTML(value="<b>Excitation settings</b>"))
        # TODO: Add DutyCycle input here
        entries_exc.append(self.get_param(
            'pulse_freq').get_as_widget(self.pulse_freq))
        entries_exc.append(self.get_param(
            'num_pulses').get_as_widget(self.num_pulses))

        entries_adv.append(widgets.HTML(value="<b>Advanced settings</b>"))
        entries_adv.append(self.get_param(
            'start_hvmuxrx').get_as_widget(self.start_hvmuxrx))
        entries_adv.append(self.get_param(
            'dcdc_turnon').get_as_widget(self.dcdc_turnon))
        # entries_adv.append(widgets.HTML(value="</br>"))                                             # Placeholder to get alignment right
        entries_adv.append(self.get_param(
            'start_ppg').get_as_widget(self.start_ppg))
        entries_adv.append(self.get_param(
            'turnon_adc').get_as_widget(self.turnon_adc))
        entries_adv.append(self.get_param(
            'start_pgainbias').get_as_widget(self.start_pgainbias))
        entries_adv.append(self.get_param(
            'start_adcsampl').get_as_widget(self.start_adcsampl))
        entries_adv.append(self.get_param(
            'restart_capt').get_as_widget(self.restart_capt))
        entries_adv.append(self.get_param(
            'capt_timeout').get_as_widget(self.capt_timeout))

        # Disable capture restart, capture timeout and number of samples (per index is sloppy, but works for now)
        entries_acq[4].disabled = True      # num_samples
        entries_adv[7].disabled = True      # restart_capt
        entries_adv[8].disabled = True      # capt_timeout

        self.entries_left = entries_acq + entries_exc
        self.entries_right = entries_adv

        for entry in self.entries_left + self.entries_right:
            # If it is a label, apply label style and skip
            if isinstance(entry, widgets.HTML):
                continue

            entry.style = input_style
            entry.observe(self.change_parameter, names='value')

    def change_parameter(self, button):

        # Update the configuration when a widget value changes

        value = button['new']
        name = button['owner'].description
        for param in configuration_package[0]:
            # Check if the parameter is a basic setting

            if param.friendly_name == name:
                # Update the value of the parameter
                setattr(self, param.config_name, value)
                break

        for param in configuration_package[1]:
            # Check if the parameter is an advanced setting

            if param.friendly_name == name:
                # Update the value of the parameter
                setattr(self, param.config_name, value)
                break

        for param in configuration_package[2]:
            # Check if the parameter is a GUI setting

            if param.friendly_name == name:
                # Update the value of the parameter
                setattr(self, param.config_name, value)
                break

        # Update register saveable values
        self.convert_to_registers()

    def save_json(self, button):
        """
        Saves the configuration to a JSON file.

        Args:
            filename (str): Path to the JSON file.
        """

        # make sure the filename has the correct extension
        filename = self.entry_filename.value
        filename = filename.split('.')[0] + '.json'

        with open(filename, 'w') as f:
            data = {}

            for param in configuration_package[0]:
                # save basic settings
                data[param.config_name] = getattr(self, param.config_name)
            for param in configuration_package[1]:
                # save advanced settings
                data[param.config_name] = getattr(self, param.config_name)
            for param in configuration_package[2]:
                # save GUI settings
                data[param.config_name] = getattr(self, param.config_name)

            # Write the JSON file
            json.dump(data, f, indent=4)

        self.info_label.value = f'Saved configuration to {filename}'

    def load_json(self, button):
        """
        Parses the configuration from a JSON file.

        Args:
            filename (str): Path to the JSON file.
        """

        # make sure the filename has the correct extension
        filename = self.entry_filename.value
        filename = filename.split('.')[0] + '.json'

        try:
            with open(filename, 'r') as f:
                # Read the JSON file
                data = json.load(f)

                for param in configuration_package[0]:
                    # load basic settings
                    try:
                        value = data[param.config_name]
                    except KeyError:
                        # If the parameter is not in the JSON file,
                        # just keep the current value
                        continue

                    # update widget value
                    for entry in self.entries_left + self.entries_right:
                        if entry.description == param.friendly_name:
                            if param.limit_type == 'limit':
                                # Check if value is within the allowed range
                                if value < entry.min:
                                    print("Warning: " + param.friendly_name + " is set to " + str(
                                        value) + " which is below the allowed range [" + str(entry.min) + ", " + str(entry.max) + "].")
                                    value = entry.min
                                    print(
                                        "         Setting " + param.friendly_name + " to " + str(value) + ".")
                                elif value > entry.max:
                                    print("Warning: " + param.friendly_name + " is set to " + str(
                                        value) + " which is above the allowed range [" + str(entry.min) + ", " + str(entry.max) + "].")
                                    value = entry.max
                                    print(
                                        "         Setting " + param.friendly_name + " to " + str(value) + ".")
                            elif param.limit_type == 'list':
                                # Check if value is located in the list of allowed values
                                if value not in entry.options:
                                    print("Warning: " + param.friendly_name + " is set to " + str(
                                        value) + " which is not allowed. Allowed values are: " + str(entry.options))
                                    # Get nearest allowed value
                                    value = min(entry.options,
                                                key=lambda x: abs(x-value))
                                    print(
                                        "         Setting " + param.friendly_name + " to " + str(value) + ".")

                            setattr(self, param.config_name, value)

                            entry.value = value
                            break

                for param in configuration_package[1]:
                    # Check if the parameter is an advanced setting

                    try:
                        setattr(self, param.config_name,
                                data[param.config_name])
                    except KeyError:
                        # If the parameter is not in the JSON file,
                        # just keep the current value
                        continue

                    # update widget value
                    for entry in self.entries_left + self.entries_right:
                        if entry.description == param.friendly_name:
                            entry.value = data[param.config_name]
                            break

                for param in configuration_package[2]:
                    # Check if the parameter is a GUI setting

                    try:
                        setattr(self, param.config_name,
                                data[param.config_name])
                    except KeyError:
                        # If the parameter is not in the JSON file,
                        # just keep the current value
                        continue

                    # update widget value
                    for entry in self.entries_left + self.entries_right:
                        if entry.description == param.friendly_name:
                            entry.value = data[param.config_name]
                            break

        except FileNotFoundError:
            # Filename not found

            self.info_label.value = f'File {filename} not found'
            return

        self.convert_to_registers()

        self.info_label.value = f'Loaded configuration from {filename}'

    def with_file(self, filename):
        """
        Loads the configuration from a JSON file.

        Args:
            filename (str): Path to the JSON file.
        """

        self.entry_filename.value = filename
        self.load_json(None)
