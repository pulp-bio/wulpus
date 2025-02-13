# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.1] - 2025-02-12

### Added

### Fixed

### Changed

- Updated Altium stackup to a default PCBWay stackup (4 layers, 1mm thickness, 1oz inner copper, 1oz outer copper, 50% inner layer residual copper ratio) 
- Regenerated fabrication files and stackup information

### Removed

## [1.1.0] - 2024-02-21

### Added

- Generated production files (Assembly drawing, BOM, Gerber files, Drill files, Pick&Place files)

### Fixed

- Marked various components as "Do not mount" (C9, C10, C11, C20, R9, C16)

### Changed

- Replaced high voltage multiplexer HV2708 to HV2707
- Replaced MOSFET MCP1402 to MCP1416
- Replaced R13, R14, R19 with 0 Ohm jumper
- Replaced R15 with 10 nF CAP
- Replaced C17 with 100 Ohm resistor

### Removed

 -