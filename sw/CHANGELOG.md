# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.0] - 2023-XX-XX

### Added

- Stop GUI when given number of acquisitions is collected.
- A STOP button to stop acquisition earlier.
- A GUI to configure/save/load TX and RX configurations of the HV multiplexer.
- A GUI to configure ultrasound subsytem's parameters, +save them to or load them from a file.
- Comments/docstrings in the source files
- Added B-mode visualization option.

### Fixed

- Bug with empty COM port list (the GUI were failing when there were no COM ports in the system).
- Now the user can rerun the main GUI cell without closing the COM port first.

### Changed

- WULPUS stops acquisitions after GUI acquired required number of samples.
- User can now directly change the sampling frequency and not the oversampling ratio.
- Band-pass filter's max/min boundaries are dynamically adapted depending on the sampling frequency.
- All the time-related parameters are now configured in us (not ticks).
- PGA gain can now be selected from a drop-down list.
- Moved visualization to a separate thread.

### Removed

- 