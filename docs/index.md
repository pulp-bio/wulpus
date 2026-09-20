# WULPUS User Guide

WULPUS is a wearable ultra-low-power open-source ultrasound probe. This guide covers everything about WULPUS, from assembly instructions to example measurements and troubleshooting.

!!! info "GUI documentation is being updated"
    Most of this guide is a faithful transfer of the previous user guide. The software and example-measurement pages still describe the **legacy Jupyter notebook GUI** in places. A follow-up update will present the current software options:

    - **[BioGUI](https://github.com/pulp-bio/biogui)** — preferred / default
    - **Web-based React GUI** in [`sw/`](https://github.com/pulp-bio/wulpus/tree/main/sw) — mainly for NDT applications
    - **Jupyter notebook interface** — legacy

## Contents

1. [System Overview](system-overview.md) — what WULPUS is, specifications, and the main hardware blocks
2. [Building Your WULPUS](building-your-wulpus.md) — PCB manufacturing, powering, firmware flashing, and the silicone package
3. [How to Use the Probe](how-to-get-started.md) — hardware requirements, GUI overview, and HV-MUX artifact mitigation
4. [Advanced Settings](advanced-settings.md) — TX/RX configurations and the ultrasound subsystem
5. [Example Experiments](example-measurements.md) — water-bath measurement
6. [Troubleshooting](troubleshooting.md) — BLE and hardware debug tree
7. [Errata](errata.md) — known hardware, firmware, and software bugs
8. [Revision History](revision.md)

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
