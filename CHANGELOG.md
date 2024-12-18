# Changelog

All main changes to this project will be documented in this file.
For the detailed description, please explore nested folders and corresponding CHANGELOG.md files (e.g. for PCB projects or firmware). 

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.0] - 2024-02-21

### Added

- WULPUS User Manual Rev 1 (`docs/wulpus_user_manual.pdf`)
- MSP430 firmware (located at `fw/msp430`)
- WULPUS programmer PCBs' design files (located at `hw/wulpus_programmer_pcbs`).
- Generated production files (Assembly drawings, BOM, Gerber files, Drill files, Pick&Place files) for all the PCBs.
- Added the dedicated GUIs to configure the excitation/receive and measurement settings.
- Added example configurations and data.

### Fixed

- BLE Package loss by implementing buffering on the nRF52 MCU.
- Multiple GUI bugs.

### Changed

- Updated the design of the acquisition and high-voltage PCBs
- Improved the schematics of the PCB projects to simplify the outsourced assembly.
- Improved the main WULPUS GUI and an example script.

### Removed

- Removed `./sw/docs` folder with the old GUI guide. The information about the GUI is now available in the WULPUS user manual.

## [1.1.1] - 2024-12-18

### Added

- 

### Fixed

- Addressed issue #17 (Power supply spikes exceeding the resistors power rating. Enable pin control of the U1)

### Changed

- 

### Removed