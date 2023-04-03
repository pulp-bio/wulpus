# WULPUS source files for nRF BLE MCU firmware projects
This directory contains the source files for 
- nRF52832 BLE MCU (`hw/nRF52/ble_peripheral`) mounted on the WULPUS acquisition PCB 
- nRF52840 Dongle (`hw/nRF52/peripheral`) used to receive the ultrasound data on a host PC

# How to get started?

1. Follow the file `hw/nRF52/how_to_setup_nRF52_toolchain.md` to install the toolchain and SDK
2. Follow the file `hw/nRF52/how_to_flash_nrf52832.md` to flash the nRF MCU on the WULPUS acquisition PCB
3. Follow the file `hw/nRF52/how_to_flash_nrf52840_dongle.md` to flash the nRF52840 Dongle

# License
The files in the `hw/nRF52/ble_peripheral` and `hw/nRF52/peripheral` directories contains third-party sources that come with their own licenses. See the respective folders and source files' headers for the licenses used.