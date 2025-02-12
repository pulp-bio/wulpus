# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.1] - 2025-02-10

### Added

### Fixed

### Changed

- Updated stackup to a default PCBWay stackup (4 layers, 1mm thickness, 1oz inner copper, 1oz outer copper, 50% inner layer residual copper ratio) 
- Adjusted RF trace width according to the stackup to keep 50ohm impedance
- Updated the gerber files and stackup information

### Removed

## [1.2.0] - 2025-01-27

### Added

### Fixed

- Placed a weak pull-down resistor (R2) to fix the disabled state of TLV62569 (U1) by default
- Replaced R1, R5, R6, R8, R9, R10, R11, R13 with 0.1Ohm, 250mW resistors
- Addressed issue #21 (Missing ground plane for microstrip)

### Changed

- Replaced chip antenna to ignion NN03-320
- Changed matching curcuit accordingly

### Removed

- Mounting hole M3 (to make space for RF routing and matching network)

## [1.1.0] - 2024-02-21

### Added

- Board-to-wire connector J5, giving access to VCC, GND and two GPIO's of each MCU
- One button for each MCU (not mounted by default)
- Generated production files (Assembly drawing, BOM, Gerber files, Drill files, Pick&Place files)

### Fixed

- Marked various components as "Do not mount" (R33, R21, R32, R2, R3, C5, C46)

### Changed

- Replaced battery connector (new DF52-2S-0.8H(21))
- Replaced R35 with 0 Ohm jumper

### Removed
