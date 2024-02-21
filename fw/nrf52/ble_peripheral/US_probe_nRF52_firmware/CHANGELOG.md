# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.0] - 2024-02-21

### Added

- 

### Fixed

- BLE Package loss by implementing buffering on the nRF52. As a consequence, an increased power consumption (+2-5 mW and more) can be observed in noisy RF environment.

### Changed

- Moved the main macro definitions to `us_defines.h`

### Removed

- Cleaned the old (not used) code