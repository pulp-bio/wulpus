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

from wulpus.uss_conf import *
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

        input_style = {'description_width': '60%', 'width': '30%'}

        self.entries_a = []
        self.entries_b = []

        for param in configuration_package[0]:
            if param.config_name == 'num_txrx_configs':
                continue
            value = getattr(self, param.config_name)
            widget = param.get_as_widget(value)
            widget.style = input_style
            self.entries_a.append(widget)
        for param in configuration_package[1]:
            value = getattr(self, param.config_name)
            widget = param.get_as_widget(value)
            widget.style = input_style
            self.entries_b.append(widget)
        
        for entry in self.entries_a + self.entries_b:
            entry.observe(self.change_parameter, names='value')

        self.info_label = widgets.Label(value="")

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

        self.children = [
            widgets.HBox([widgets.VBox(self.entries_a),widgets.VBox(self.entries_b)]),
            widgets.HBox([self.entry_filename, self.save_button, self.load_button]),
            self.info_label,
            self.output]
        
    def change_parameter(self, button):
        
        value = button['new']
        name = button['owner'].description
        for param in configuration_package[0]:
            if param.friendly_name == name:
                setattr(self, param.config_name, value)
                break

        for param in configuration_package[1]:
            if param.friendly_name == name:
                setattr(self, param.config_name, value)
                break

        self.convert_to_registers()

        print(f'Changed {name} to {value}')

    def save_json(self, button):
        """
        Saves the configuration to a JSON file.

        Args:
            filename (str): Path to the JSON file.
        """

        filename = self.entry_filename.value
        filename = filename.split('.')[0] + '.json'

        with open(filename, 'w') as f:
            data = {}
            
            for param in configuration_package[0]:
                data[param.config_name] = getattr(self, param.config_name)
            for param in configuration_package[1]:
                data[param.config_name] = getattr(self, param.config_name)

            json.dump(data, f, indent=4)

        self.info_label.value = f'Saved configuration to {filename}'

    def load_json(self, button):
        """
        Parses the configuration from a JSON file.

        Args:
            filename (str): Path to the JSON file.
        """

        filename = self.entry_filename.value
        filename = filename.split('.')[0] + '.json'
        try:
            with open(filename, 'r') as f:
                data = json.load(f)

                for param in configuration_package[0]:
                    setattr(self, param.config_name, data[param.config_name])
                    # set widget value
                    for entry in self.entries_a:
                        if entry.description == param.friendly_name:
                            entry.value = data[param.config_name]
                            break
                for param in configuration_package[1]:
                    setattr(self, param.config_name, data[param.config_name])
                    # set widget value
                    for entry in self.entries_b:
                        if entry.description == param.friendly_name:
                            entry.value = data[param.config_name]
                            break

        except FileNotFoundError:
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
