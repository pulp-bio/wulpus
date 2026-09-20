# WULPUS User Guide

![The WULPUS probe next to a coin.](figures/pictures/wulpus_coin.jpg){ width="80%" }

Integrated Systems Laboratory, Department of Information Technology and Electrical Engineering, ETH Zurich.

WULPUS is a wearable ultra-low-power open-source ultrasound probe. This guide covers everything about WULPUS, from assembly instructions to example measurements and troubleshooting.

!!! info "GUI documentation is being updated"
    Most of this guide is a faithful transfer of the previous user guide. The software and example-measurement pages still describe the **legacy Jupyter notebook GUI** in places. A follow-up update will present the current software options:

    - **[BioGUI](https://github.com/pulp-bio/biogui)** — preferred / default
    - **Web-based React GUI** in [`sw/`](https://github.com/pulp-bio/wulpus/tree/main/sw) — mainly for NDT applications
    - **Jupyter notebook interface** — legacy

## Contents

- [1 System Overview](system-overview.md)
    - [1.1 What is the WULPUS probe?](system-overview.md#what-is-the-wulpus-probe)
    - [1.2 Specifications](system-overview.md#specifications)
    - [1.3 System Components](system-overview.md#system-components)
        - [1.3.1 Acquisition PCB](system-overview.md#acquisition-pcb)
        - [1.3.2 High-Voltage PCB](system-overview.md#high-voltage-pcb)
        - [1.3.3 USB Dongle and Host PC](system-overview.md#usb-dongle-and-host-pc)
- [2 How to build your own WULPUS probe?](building-your-wulpus.md)
    - [2.1 PCB manufacturing and assembly](building-your-wulpus.md#pcb-manufacturing-and-assembly)
        - [2.1.1 Self-Assembly](building-your-wulpus.md#self-assembly)
        - [2.1.2 Full-Service Manufacturing](building-your-wulpus.md#full-service-manufacturing)
        - [2.1.3 Cost estimation](building-your-wulpus.md#cost-estimation)
    - [2.2 Powering up the US probe](building-your-wulpus.md#powering-up-the-us-probe)
    - [2.3 Programming the MSP430](building-your-wulpus.md#programming-the-msp430)
    - [2.4 Programming the nRF52 on the Acquisition PCB](building-your-wulpus.md#programming-the-nrf52-on-the-acquisition-pcb)
    - [2.5 Programming the USB Dongle](building-your-wulpus.md#programming-the-usb-dongle)
    - [2.6 Silicone Rubber Package for WULPUS Probe](building-your-wulpus.md#silicone-rubber-package)
- [3 How to use the probe?](how-to-get-started.md)
    - [3.1 Hardware requirements](how-to-get-started.md#hardware-requirements)
    - [3.2 Software requirements](how-to-get-started.md#software-requirements)
    - [3.3 GUI Overview](how-to-get-started.md#gui-overview)
    - [3.4 MUX artifact mitigation](how-to-get-started.md#mux-artifact-mitigation)
- [4 Advanced Settings](advanced-settings.md)
    - [4.1 Transmit/Receive (TX/RX) configurations](advanced-settings.md#tx-rx-configurations)
    - [4.2 Configuration of Ultrasound Subsystem](advanced-settings.md#configuration-of-ultrasound-subsystem)
- [5 Example experiments](example-measurements.md)
    - [5.1 Water bath](example-measurements.md#water-bath)
- [6 Troubleshooting](troubleshooting.md)
    - [6.1 BLE](troubleshooting.md#ble)
    - [6.2 Hardware](troubleshooting.md#hardware)
- [7 Errata](errata.md)
    - [7.1 Hardware Bugs](errata.md#hardware-bugs)
    - [7.2 Firmware Bugs](errata.md#firmware-bugs)
    - [7.3 Software Bugs](errata.md#software-bugs)
- [8 Revision history](revision.md)

## Authors

This user guide was prepared at the [Integrated Systems Laboratory (IIS)](https://iis.ee.ethz.ch/), ETH Zurich:

- Sergei Vostrikov — vsergei@iis.ee.ethz.ch
- Sebastian Frey — sefrey@iis.ee.ethz.ch
- Cedric Hirschi — cehirschi@student.ethz.ch
- Josquin Tille — jtille@student.ethz.ch
- William Bruderer — wbruderer@student.ethz.ch
- Luca Benini — lbenini@iis.ee.ethz.ch
- Andrea Cossettini — cossettini.andrea@ethz.ch

## License

Documentation images are licensed under [CC BY 4.0](images/LICENSE).
