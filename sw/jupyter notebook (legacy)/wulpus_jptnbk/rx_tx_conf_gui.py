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

from wulpus_jptnbk.rx_tx_conf import WulpusRxTxConfigGen, TX_RX_MAX_NUM_OF_CONFIGS, MAX_CH_ID
import ipywidgets as widgets
import json

DEFAULT_FILENAME = 'tx_rx_configs'  # default filename for saving/loading configs
GUI_WIDTH = '70%'                   # width of the GUI


class _Config():
    """
    Represents a single configuration with its ID, TX and RX channels, and enabled status.

    Attributes:
        config_id (int): The ID of the configuration.
        tx_channels (list): The TX channels of the configuration.
        rx_channels (list): The RX channels of the configuration.
        enabled (bool): Whether the configuration is enabled.
        optimized_switching (bool): Whether to use an algorithm to minimize switching artifacts.
    """

    def __init__(self, config_id):
        self.config_id = config_id
        self.tx_channels = []
        self.rx_channels = []
        self.enabled = False
        self.optimized_switching = False


class _ConfigParser():
    """
    Manages a list of configurations, including saving them to and loading them from a JSON file.

    Attributes:
        configs (list): The list of configurations.
        gui (object): The GUI object.
        config_index (int): The current index in the list of configurations.
    """

    def __init__(self, configs, gui):
        self.configs = configs
        self.gui = gui
        self.config_index = 0

    # save configs to a json file
    def save_json(self, filename):
        """
        Saves the configurations to a JSON file.

        Args:
            filename (str): The filename of the JSON file, with or without the .json extension.
        """

        # count enabled configs
        enabled_configs = 0
        for config in self.configs:
            if config.enabled:
                enabled_configs += 1

        # check if there are enabled configs
        if enabled_configs == 0:
            # current status is always displayed in a label
            self.gui.label_info.value = 'No enabled config(s) to save.'
            return

        # make sure filename ends with .json
        filename = filename.split('.')[0]
        filename = filename.join(['', '.json'])
        self.gui.label_info.value = 'Saving config(s) to "' + filename + '"'

        # open file and write configs
        with open(filename, 'w') as f:
            # define data structure
            data = {}
            data['configs'] = []

            # align all IDs beginning with such that there are no gaps
            aligned_id = 0
            for config in self.configs:
                # only write enabled configs
                if config.enabled:
                    data['configs'].append({
                        'config_id': aligned_id,
                        'tx_channels': config.tx_channels,
                        'rx_channels': config.rx_channels,
                        'optimized_switching': config.optimized_switching
                    })
                    aligned_id += 1

            # write data to file
            json.dump(data, f, indent=4)
            self.gui.label_info.value = 'Saved ' + \
                str(len(data['configs'])) + ' config(s) to "' + filename + '"'

    def parse_json(self, filename):
        """
        Parses configurations from a JSON file.

        Args:
            filename (str): The filename of the JSON file, with or without the .json extension.
        """

        # make sure filename ends with .json
        filename = filename.split('.')[0]
        filename = filename.join(['', '.json'])
        self.gui.label_info.value = 'Loading config(s) from ' + filename

        try:
            # open file and write configs
            with open(filename, 'r') as f:
                # load data from file
                data = json.load(f)

                # check if data has configs
                if 'configs' not in data:
                    self.gui.label_info.value = 'No config(s) found in file.'
                    return
                configs = data['configs']

                # check if there is at least one config (should always be the case if GUI is used)
                if len(configs) == 0:
                    print('No configs found in file.')
                    self.gui.label_info.value = 'No config(s) found in file.'
                    return

                # check if there are too many configs (should never be the case if GUI is used)
                if len(configs) > TX_RX_MAX_NUM_OF_CONFIGS:
                    print('Too many configs found in file.')
                    self.gui.label_info.value = 'Too many config(s) found in file.'
                    return

                # reset all configs
                for i in range(0, TX_RX_MAX_NUM_OF_CONFIGS):
                    self.configs[i].enabled = False
                    self.configs[i].optimized_switching = False
                    self.configs[i].tx_channels = []
                    self.configs[i].rx_channels = []

                # parse configs
                for config in configs:
                    # check if config has all required fields (should always be the case if GUI is used)
                    if 'config_id' not in config or 'tx_channels' not in config or \
                            'rx_channels' not in config or 'optimized_switching' not in config:
                        print('Invalid config found in file.')
                        self.gui.label_info.value = 'Invalid config found in file.'
                        return

                    # add config to list
                    self.configs[config['config_id']].enabled = True
                    self.configs[config['config_id']
                                 ].optimized_switching = config['optimized_switching']
                    self.configs[config['config_id']
                                 ].tx_channels = config['tx_channels']
                    self.configs[config['config_id']
                                 ].rx_channels = config['rx_channels']

                self.gui.label_info.value = 'Loaded ' + \
                    str(len(configs)) + ' config(s) from "' + filename + '"'

        except FileNotFoundError:
            # if file not found, display error message and return
            self.gui.label_info.value = 'File "' + filename + '" not found.'

# inherit from WulpusRxTxConfigGen


class WulpusRxTxConfigGenGUI(widgets.VBox):
    """
    A GUI for managing the TX and RX configurations of Wulpus.

    Attributes:
        output (object): The output widget.
        configs (list): The list of configurations.
        config_index_selected (int): The current index in the list of configurations.
        dropdown_configs (object): The dropdown widget for selecting a configuration.
        check_config_enabled (object): The checkbox widget for enabling/disabling a configuration.
        check_config_optimized_switching (object): The checkbox widget for enabling/disabling switching optimization.
        label_info (object): The label widget for displaying information.
        buttons_channels_tx (list): The list of buttons for selecting TX channels.
        buttons_channels_rx (list): The list of buttons for selecting RX channels.
        entry_filename (object): The text widget for entering a filename.
        save_button (object): The button widget for saving configurations to a file.
        load_button (object): The button widget for loading configurations from a file.
    """

    def __init__(self):
        super().__init__()

        # Visualization of the GUI
        self.output = widgets.Output()

        # list of configs
        self.configs = [_Config(i) for i in range(0, TX_RX_MAX_NUM_OF_CONFIGS)]
        self.config_index_selected = 0

        # dropdown of all configs in the list
        self.dropdown_configs = widgets.Dropdown(
            options=[i for i in range(0, TX_RX_MAX_NUM_OF_CONFIGS)],
            index=0,
            description='Config:',
            disabled=False
        )
        self.dropdown_configs.observe(
            self.on_dropdown_configs_change, names='index')
        # checkbox for enabling/disabling the chosen config
        self.check_config_enabled = widgets.Checkbox(
            value=self.configs[self.config_index_selected].enabled,
            description='Enabled',
            disabled=False
        )
        self.check_config_enabled.observe(
            self.on_check_config_enabled_change, names='value')
        # checkbox for enabling/disabling the optimised switching for chosen config
        self.check_config_optimized_switching = widgets.Checkbox(
            value=self.configs[self.config_index_selected].optimized_switching,
            description='Optimize Switching',
            disabled=False
        )
        self.check_config_optimized_switching.observe(
            self.on_check_optimized_switching_change, names='value')
        # label for displaying information
        self.label_info = widgets.Label(value='')
        config_hbox = widgets.HBox([self.dropdown_configs, self.check_config_enabled, self.check_config_optimized_switching],
                                   layout=widgets.Layout(align_items='flex-start', justify_content='flex-start', width=GUI_WIDTH))

        # buttons for selecting TX and RX channels
        self.buttons_channels_tx = [widgets.Button(description=str(
            i), button_style='', layout=widgets.Layout(width='12%')) for i in range(0, MAX_CH_ID+1)]
        for tx_button in self.buttons_channels_tx:
            tx_button.on_click(self.on_button_channel_tx_click)
        buttons_tx_hbox = widgets.HBox([widgets.Label(value='TX', layout=widgets.Layout(
            width='25px'))] + self.buttons_channels_tx, layout=widgets.Layout(width=GUI_WIDTH, justify_content='space-between', align_items='center'))
        self.buttons_channels_rx = [widgets.Button(description=str(
            i), button_style='', layout=widgets.Layout(width='12%')) for i in range(0, MAX_CH_ID+1)]
        for rx_button in self.buttons_channels_rx:
            rx_button.on_click(self.on_button_channel_rx_click)
        buttons_rx_hbox = widgets.HBox([widgets.Label(value='RX', layout=widgets.Layout(
            width='25px'))] + self.buttons_channels_rx, layout=widgets.Layout(width=GUI_WIDTH, justify_content='space-between', align_items='center'))
        buttons_hbox = widgets.VBox([buttons_tx_hbox, buttons_rx_hbox])

        # text field and buttons for saving/loading configs to/from file
        self.entry_filename = widgets.Text(
            value=DEFAULT_FILENAME,
            placeholder='Type something',
            description='Filename:',
            disabled=False
        )
        self.save_button = widgets.Button(
            description='Save',
            disabled=False,
            tooltip='Save current configs to file',
            icon='save'
        )
        self.save_button.on_click(self.on_save_button_click)
        self.load_button = widgets.Button(
            description='Load',
            disabled=False,
            tooltip='Load configs from file',
            icon='upload'
        )
        self.load_button.on_click(self.on_load_button_click)
        file_buttons_hbox = widgets.HBox([self.entry_filename, self.save_button, self.load_button], layout=widgets.Layout(
            align_items='flex-start', justify_content='flex-start', width=GUI_WIDTH))

        # initialize GUI
        self.update_buttons()
        # , self.output] # uncomment to display console output
        self.children = [config_hbox, buttons_hbox,
                         file_buttons_hbox, self.label_info]

    def update_buttons(self):
        # update visualization of buttons
        self.check_config_enabled.value = self.configs[self.config_index_selected].enabled
        self.check_config_optimized_switching.value = self.configs[
            self.config_index_selected].optimized_switching
        for i in range(0, MAX_CH_ID+1):
            self.buttons_channels_tx[i].button_style = 'success' if i in self.configs[
                self.config_index_selected].tx_channels else 'danger'
            self.buttons_channels_rx[i].button_style = 'success' if i in self.configs[
                self.config_index_selected].rx_channels else 'danger'

    def on_dropdown_configs_change(self, change):
        # update currently chosen config from dropdown
        with self.output:
            self.config_index_selected = change['new']
            self.check_config_enabled.value = self.configs[self.config_index_selected].enabled
            self.update_buttons()

    def on_check_config_enabled_change(self, change):
        # update enabled status of currently chosen config from checkbox
        with self.output:
            self.configs[self.config_index_selected].enabled = change['new']

    def on_check_optimized_switching_change(self, change):
        # update optimized switching setting status of currently chosen config from checkbox
        with self.output:
            self.configs[self.config_index_selected].optimized_switching = change['new']

    def on_button_channel_tx_click(self, button):
        # update TX channels of currently chosen config from button
        with self.output:
            channel_id = int(button.description)

            # see if channel id is already in the list, if so, remove it, otherwise add it
            if channel_id in self.configs[self.config_index_selected].tx_channels:
                self.configs[self.config_index_selected].tx_channels.remove(
                    channel_id)
            else:
                self.configs[self.config_index_selected].tx_channels.append(
                    channel_id)

            self.update_buttons()

    def on_button_channel_rx_click(self, button):
        # update RX channels of currently chosen config from button
        with self.output:
            channel_id = int(button.description)

            # see if channel id is already in the list, if so, remove it, otherwise add it
            if channel_id in self.configs[self.config_index_selected].rx_channels:
                self.configs[self.config_index_selected].rx_channels.remove(
                    channel_id)
            else:
                self.configs[self.config_index_selected].rx_channels.append(
                    channel_id)

            self.update_buttons()

    def on_save_button_click(self, button):
        # save configs to file
        with self.output:
            # make sure filename isn't empty
            filename = self.entry_filename.value
            if filename == '':
                filename = DEFAULT_FILENAME

            # save configs to file using _ConfigParser helper class
            _ConfigParser(self.configs, self).save_json(filename)

    def on_load_button_click(self, button):
        # load configs from file
        with self.output:
            # make sure filename isn't empty
            filename = self.entry_filename.value
            if filename == '':
                filename = DEFAULT_FILENAME

            # load configs from file using _ConfigParser helper class
            _ConfigParser(self.configs, self).parse_json(filename)

            # update GUI
            self.config_index_selected = 0
            self.dropdown_configs.index = 0
            self.update_buttons()

    def add_config(self, tx_channels, rx_channels):
        """
        Adds a configuration to the list of configurations.

        Args:
            tx_channels (list): The TX channels of the configuration.
            rx_channels (list): The RX channels of the configuration.
        """

        # check if there is a free config
        for config in self.configs:
            if not config.enabled:
                config.enabled = True
                config.tx_channels = tx_channels
                config.rx_channels = rx_channels
                return

        # if not, display error message
        self.label_info.value = 'No free config found.'

    def with_file(self, filename):
        """
        With this method, the filename can be set directly for saving/loading configs, without having to enter it in the GUI.

        Args:
            filename (str): The filename of the JSON file, with or without the .json extension.
        """

        # load configs from file using _ConfigParser helper class
        _ConfigParser(self.configs, self).parse_json(filename)

        return self

    def get_tx_configs(self):
        """
        Returns the TX configurations as a list of dictionaries.

        Returns:
            list: The TX configurations.
        """

        # parse configs using main WulpusRxTxConfigGen class
        conf_gen = WulpusRxTxConfigGen()

        # add configs and check if at least one config is enabled
        config_found = False
        for config in self.configs:
            if config.enabled:
                conf_gen.add_config(
                    config.tx_channels, config.rx_channels, config.optimized_switching)
                config_found = True
        if not config_found:
            raise ValueError('No enabled config found.')

        # return TX configs
        return conf_gen.get_tx_configs()

    def get_rx_configs(self):
        """
        Returns the RX configurations as a list of dictionaries.

        Returns:
            list: The RX configurations.
        """

        # parse configs using main WulpusRxTxConfigGen class
        conf_gen = WulpusRxTxConfigGen()

        # add configs and check if at least one config is enabled
        config_found = False
        for config in self.configs:
            if config.enabled:
                conf_gen.add_config(
                    config.tx_channels, config.rx_channels, config.optimized_switching)
                config_found = True
        if not config_found:
            raise ValueError('No enabled config found.')

        # return RX configs
        return conf_gen.get_rx_configs()
