# Flash the Nordic nRF52832 with Segger Embedded Studio (SES)

First, complete the toolchain setup, as described in **how_to_setup_nRF52_toolchain.md**

### To flash the US firmware to the nRF52832:

### First, the hex file of the C project has to be generated:  
Open the project in SES:  
- Select File > Open Solution...
- Select the SES project:  
`nRF5_SDK_17.1.0/examples/peripheral/US_probe_dongle_firmware/pca10059/s140/ses/US_probe_dongle_firmware.emProject`
- Generate the hex file: Select Build > Build US_probe_dongle_firmware
- The resulting `US_probe_dongle_firmware.hex` is located in the folder:  
`hw/nRF52/nRF5_SDK_17.1.0/examples/peripheral/US_probe_dongle_firmware/pca10059/s140/ses/Output/Debug/Exe/`

### Second, the Softdevice S140 7.2.0 is needed:  
Download a fresh copy:  
'https://www.nordicsemi.com/Products/Development-software/S140/Download?lang=en#infotabs'
- Download and unpack the Softdevice S140 in `nRF5_SDK_17.1.0/examples/peripheral/US_probe_dongle_firmware/s140_nrf52_7.2.0_softdevice.hex`

### Third, program the application and softdevice hex files:  
- Open nRF Connect for Desktop and launch the Programmer app.
- Plug the Dongle into an USB port
- Press the reset button to put the Dongle in DFU mode. Note that the reset button is the sideways button right next to the better visible SW1 button on the Dongle:
- Select the dongle in the dropdown in the upper left corner in the Programmer app.
- Click "Add HEX file" to select the application hex file to program onto the dongle.
- Click "Add HEX file" to select the Softdevice hex file downloaded above.
- Click "Write" to write the firmware to the dongle.