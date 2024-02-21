# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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

 -