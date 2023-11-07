from logging import config
from textwrap import indent
from tkinter import E
from wulpus.rx_tx_conf import WulpusRxTxConfigGen, TX_RX_MAX_NUM_OF_CONFIGS, MAX_CH_ID
import ipywidgets as widgets
import json

# config helper class
class _Config():
    def __init__(self, config_id):
        self.config_id = config_id
        self.tx_channels = []
        self.rx_channels = []
        self.enabled = False

class _ConfigParser():
    def __init__(self, configs, gui):
        self.configs = configs
        self.gui = gui
        self.config_index = 0

    def save_json(self, filename):
        # count enabled configs
        enabled_configs = 0
        for config in self.configs:
            if config.enabled:
                enabled_configs += 1

        if enabled_configs == 0:
            print('No enabled configs to save.')
            self.gui.label_info.value = 'No enabled configs to save.'
            return

        filename = filename.replace('.json', '')
        filename = filename.join(['', '.json'])
        print('Saving ' + str(len(self.configs)) + ' configs to "' + filename + '"')
        self.gui.label_info.value = 'Saving configs to "' + filename + '"'
        
        with open(filename, 'w') as f:
            data = {}
            data['configs'] = []
            aligned_id = 0
            for config in self.configs:
                if config.enabled:
                    data['configs'].append({
                        'config_id': aligned_id,
                        'tx_channels': config.tx_channels,
                        'rx_channels': config.rx_channels
                    })
                    aligned_id += 1
            json.dump(data, f, indent=4)
            print('Saved ' + str(len(data['configs'])) + ' configs to "' + filename + '"')
            self.gui.label_info.value = 'Saved ' + str(len(data['configs'])) + ' configs to "' + filename + '"'

    def parse_json(self, filename):
            filename = filename.replace('.json', '')
            filename = filename.join(['', '.json'])
            print('Loading configs from ' + filename)
            self.gui.label_info.value = 'Loading configs from ' + filename

            with open(filename, 'r') as f:
                data = json.load(f)

                # check if data has configs
                if 'configs' not in data:
                    print('No configs found in file.')
                    self.gui.label_info.value = 'No configs found in file.'
                    return

                configs = data['configs']

                if len(configs) == 0:
                    print('No configs found in file.')
                    self.gui.label_info.value = 'No configs found in file.'
                    return
                
                if len(configs) > TX_RX_MAX_NUM_OF_CONFIGS:
                    print('Too many configs found in file.')
                    self.gui.label_info.value = 'Too many configs found in file.'
                    return

                for i in range(0, TX_RX_MAX_NUM_OF_CONFIGS):
                    self.configs[i].enabled = False
                    self.configs[i].tx_channels = []
                    self.configs[i].rx_channels = []
                for config in configs:
                    if 'config_id' not in config or 'tx_channels' not in config or 'rx_channels' not in config:
                        print('Invalid config found in file.')
                        self.gui.label_info.value = 'Invalid config found in file.'
                        return
                    self.configs[config['config_id']].enabled = True
                    self.configs[config['config_id']].tx_channels = config['tx_channels']
                    self.configs[config['config_id']].rx_channels = config['rx_channels']
                print('Loaded ' + str(len(configs)) + ' configs from "' + filename + '"')
                self.gui.label_info.value = 'Loaded ' + str(len(configs)) + ' configs from "' + filename + '"'

# inherit from WulpusRxTxConfigGen
class WulpusRxTxConfigGenGUI(widgets.VBox):
    def __init__(self):
        super().__init__()

        self.output = widgets.Output()

        self.configs = [_Config(i) for i in range(0, TX_RX_MAX_NUM_OF_CONFIGS)]
        self.config_index_selected = 0

        # dropdown with values from 0 to TX_RX_MAX_NUM_OF_CONFIGS - 1
        self.dropdown_configs = widgets.Dropdown(
            options = [i for i in range(0, TX_RX_MAX_NUM_OF_CONFIGS)],
            index = 0,
            description = 'Config:',
            disabled = False,
            layout = widgets.Layout(width='30%', justify_content='flex-start')
        )
        self.dropdown_configs.observe(self.on_dropdown_configs_change, names='index')
        self.check_config_enabled = widgets.Checkbox(
            value = self.configs[self.config_index_selected].enabled,
            description = 'Enabled',
            disabled = False,
            layout = widgets.Layout(width='30%', justify_content='flex-start')
        )
        self.check_config_enabled.observe(self.on_check_config_enabled_change, names='value')
        self.label_info = widgets.Label(value='', layout=widgets.Layout(width='40%', justify_content='flex-end'))
        config_hbox = widgets.HBox([self.dropdown_configs, self.check_config_enabled, self.label_info], layout=widgets.Layout(align_items='center', justify_content='space-between', width='50%'))

        self.buttons_channels_tx = [widgets.Button(description=str(i), button_style='', layout=widgets.Layout(width='12%')) for i in range(0, MAX_CH_ID+1)]
        for tx_button in self.buttons_channels_tx:
            tx_button.on_click(self.on_button_channel_tx_click)
        buttons_tx_hbox = widgets.HBox([widgets.Label(value='TX', layout=widgets.Layout(width='20px'))] + self.buttons_channels_tx, layout=widgets.Layout(width='50%', justify_content='space-between', align_items='center'))
        self.buttons_channels_rx = [widgets.Button(description=str(i), button_style='', layout=widgets.Layout(width='12%')) for i in range(0, MAX_CH_ID+1)]
        for rx_button in self.buttons_channels_rx:
            rx_button.on_click(self.on_button_channel_rx_click)
        buttons_rx_hbox = widgets.HBox([widgets.Label(value='RX', layout=widgets.Layout(width='20px'))] + self.buttons_channels_rx, layout=widgets.Layout(width='50%', justify_content='space-between', align_items='center'))
        buttons_hbox = widgets.VBox([buttons_tx_hbox, buttons_rx_hbox])

        self.entry_filename = widgets.Text(
            value='wulpus_config',
            placeholder='Type something',
            description='Filename:',
            disabled=False
        )
        self.save_button = widgets.Button(
            description='Save',
            disabled=False,
            button_style='success',
            tooltip='Save current configs to file',
            icon='save'
        )
        self.save_button.on_click(self.on_save_button_click)
        self.load_button = widgets.Button(
            description='Load',
            disabled=False,
            button_style='success',
            tooltip='Load configs from file',
            icon='upload'
        )
        self.load_button.on_click(self.on_load_button_click)
        file_buttons_hbox = widgets.HBox([self.entry_filename, self.save_button, self.load_button], layout=widgets.Layout(align_items='flex-start', justify_content='space-between', width='50%'))

        self.update_buttons()

        self.children = [config_hbox, buttons_hbox, file_buttons_hbox] # , self.output]
    
    def update_buttons(self):
        self.check_config_enabled.value = self.configs[self.config_index_selected].enabled
        for i in range(0, MAX_CH_ID+1):
            self.buttons_channels_tx[i].button_style = 'success' if i in self.configs[self.config_index_selected].tx_channels else 'danger'
            self.buttons_channels_rx[i].button_style = 'success' if i in self.configs[self.config_index_selected].rx_channels else 'danger'

    def on_dropdown_configs_change(self, change):
        with self.output:
            self.config_index_selected = change['new']
            self.check_config_enabled.value = self.configs[self.config_index_selected].enabled
            self.update_buttons()

            # print('Config selected: ' + str(self.config_index_selected))

    def on_check_config_enabled_change(self, change):
        with self.output:
            self.configs[self.config_index_selected].enabled = change['new']
            # print('Config ' + str(self.config_index_selected) + ' enabled: ' + str(change['new']))

    def on_button_channel_tx_click(self, button):
        with self.output:
            channel_id = int(button.description)
            
            # see if channel id is already in the list
            if channel_id in self.configs[self.config_index_selected].tx_channels:
                self.configs[self.config_index_selected].tx_channels.remove(channel_id)
            else:
                self.configs[self.config_index_selected].tx_channels.append(channel_id)

            self.update_buttons()
            # print('Config ' + str(self.config_index_selected) + ' TX channels: ' + str(self.configs[self.config_index_selected].tx_channels))

    def on_button_channel_rx_click(self, button):
        with self.output:
            channel_id = int(button.description)
            
            # see if channel id is already in the list
            if channel_id in self.configs[self.config_index_selected].rx_channels:
                self.configs[self.config_index_selected].rx_channels.remove(channel_id)
            else:
                self.configs[self.config_index_selected].rx_channels.append(channel_id)

            self.update_buttons()
            # print('Config ' + str(self.config_index_selected) + ' RX channels: ' + str(self.configs[self.config_index_selected].rx_channels))

    def on_save_button_click(self, button):
        with self.output:
            filename = self.entry_filename.value
            if filename == '':
                filename = 'wulpus_config'
            _ConfigParser(self.configs, self).save_json(filename)
            

    def on_load_button_click(self, button):
        with self.output:
            filename = self.entry_filename.value
            if filename == '':
                filename = 'wulpus_config'
            _ConfigParser(self.configs, self).parse_json(filename)
            self.config_index_selected = 0
            self.dropdown_configs.index = 0
            self.update_buttons()
            

    def get_tx_configs(self):
        conf_gen = WulpusRxTxConfigGen()
        config_found = False
        for config in self.configs:
            if config.enabled:
                conf_gen.add_config(config.tx_channels, config.rx_channels)
                config_found = True

        if not config_found:
            raise ValueError('No enabled config found.')

        return conf_gen.get_tx_configs()
    
    def get_rx_configs(self):
        conf_gen = WulpusRxTxConfigGen()
        config_found = False
        for config in self.configs:
            if config.enabled:
                conf_gen.add_config(config.tx_channels, config.rx_channels)
                config_found = True

        if not config_found:
            raise ValueError('No enabled config found.')

        return conf_gen.get_rx_configs()

