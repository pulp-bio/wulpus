# Flash the Nordic nRF52832 with Segger Embedded Studio (SES)

First, complete the toolchain setup, as described in **how_to_setup_nRF52_toolchain.md**

### To flash the US firmware to the nRF52832:

Open the project in SES:  
- Select File > Open Solution...
- Select the SES project:  
`nRF5_SDK_17.1.0/examples/ble_peripheral/US_probe_nRF52_firmware/pca10040/s132/ses/US_probe_nRF52_firmware.emProject`
- Connect the US probe with the j-link debugger and the 6-pin Molex connector
- Flash the code: Select Build > Build and Run
- The Softdevice (for BLE) is flashed automatically together with the rest of the code
- The nRF52832 needs a power cycle to start the execution of the code