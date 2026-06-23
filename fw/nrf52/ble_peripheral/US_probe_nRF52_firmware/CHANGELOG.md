# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.3] - 2026-04-02

### Added

- Optional accelerometer streaming on the nRF52, enabled only when requested by the user configuration.
- `fw/nrf52/ble_peripheral/US_probe_nRF52_firmware/iis2dh.c`: Added IIS2DH accelerometer driver, runtime enable/disable handling, and ACC/non-ACC frame finalization.
- `fw/nrf52/ble_peripheral/US_probe_nRF52_firmware/iis2dh.h`: Added IIS2DH register definitions, types, and public driver API.

### Changed

- `fw/nrf52/ble_peripheral/US_probe_nRF52_firmware/main.c`: Added runtime accelerometer state handling and deferred mode application from the main loop.
- `fw/nrf52/ble_peripheral/US_probe_nRF52_firmware/us_ble.c`: Added config-packet parsing for user-requested accelerometer streaming and deferred update signaling.
- `fw/nrf52/ble_peripheral/US_probe_nRF52_firmware/us_spi.c`: Moved frame completion responsibility to the ACC/non-ACC post-processing path.
- `fw/nrf52/ble_peripheral/US_probe_nRF52_firmware/pca10040/s132/ses/US_probe_nRF52_firmware.emProject`: Added IIS2DH source files and TWI driver sources to the SES project.


## [1.1.0] - 2024-02-21

### Added

- 

### Fixed

- BLE Package loss by implementing buffering on the nRF52. As a consequence, an increased power consumption (+2-5 mW and more) can be observed in noisy RF environment.

### Changed

- Moved the main macro definitions to `us_defines.h`

### Removed

- Cleaned the old (not used) code