# Errata

This chapter includes all the hardware, firmware and software bugs found *so far*.

If you find any bug not mentioned here, do not hesitate to contact us! You can find contact details on the [repository README](https://github.com/pulp-bio/wulpus#authors).

## Hardware Bugs

### EH-01 Current Measurement Resistor Sizing

HW Versions: **≤ v1.1.0**

**Problem**

In certain cases, some heavier power spikes can cause some resistors to burn up, rendering the probe unusable. Resistors at the top of the power tree (**R1** at `VUSB`, **R5** at `VBAT`, **R6** and **R9** before the voltage conversion `VSYS_IN`) are particularly susceptible.

**Cause**

All current measurement resistors on the acquisition PCB (R1, R5, R6, R8, R9, R10, R11, R13) have been sized to **1 Ohm 1/8W**.

**Solution**

We recommend replacing at least **R1 and R5** with **0.1 Ohm 1/4W** alternatives such as **ERJ-2BWFR100X** by Panasonic Electronic Components.

## Firmware Bugs

—

## Software Bugs

—
