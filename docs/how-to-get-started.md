# How to use the probe?

This chapter explains the needed steps to get started with the WULPUS probe.

The first section focuses on the hardware requirements and describes the equipment you need to take the first measurements. The second section discusses necessary software installations and the WULPUS graphical user interface (GUI). Later sections cover a test measurement and HV-MUX artifact mitigation.

## Hardware requirements {#hardware-requirements}

To conduct US measurements with WULPUS, you will need the components listed in the following (see figure below).

1. Acquisition PCB
2. High Voltage PCB
3. Your ultrasound transducer (with compatible connector)
4. nRF USB Dongle
5. Windows (or Linux) computer/laptop with USB port
6. Power supply: USB cable/battery/lab supply

!!! note
    The HV PCB is equipped with a 16-pin **DF52-16S-0.8H** connector. To connect a custom transducer to this connector, the user should utilize a compatible **DF52-16P-0.8C** mating connector, along with pre-crimped **DF52-2832PF1571-28A9-300** cables. The pre-crimped cables should then be directly soldered to the transducer terminals.

![Overview of the hardware components.](figures/pictures/hardware_requirements.png){ width="80%" }

*Overview of the hardware components. The image of the USB Dongle is adapted from [Nordic Semiconductor](https://www.nordicsemi.com/Products/Development-hardware/nRF52840-Dongle/GetStarted).*

## Software requirements {#software-requirements}

!!! warning "This section will be rewritten for the current GUI ecosystem"
    The previous user guide described only the Jupyter notebook GUI (Anaconda, `requirements.yml`, `wulpus_gui.ipynb`). That walkthrough is **not** copied here, because it is no longer the recommended setup.

    Until the follow-up update, use:

    - **[BioGUI](https://github.com/pulp-bio/biogui)** — preferred / default option for general use and HMI applications. See the [BioGUI documentation](https://pulp-bio.github.io/biogui/).
    - **Web-based React GUI** — mainly intended for NDT applications and acquisition sequences. Setup: [`sw/README.md`](https://github.com/pulp-bio/wulpus/blob/main/sw/README.md).
    - **Jupyter notebook interface** — legacy. Located in [`sw/jupyter notebook (legacy)/`](https://github.com/pulp-bio/wulpus/tree/main/sw/jupyter%20notebook%20(legacy)).

    The [repository README](https://github.com/pulp-bio/wulpus#how-to-use) summarizes the same three options.

## GUI Overview

!!! warning "GUI walkthrough pending"
    Screenshots and step-by-step controls (scan ports, start/stop measurement, plotting, saving `.npz` files) still belong to the Jupyter GUI. They will be replaced with BioGUI screenshots and explanations in a follow-up.

    Connecting the probe itself is unchanged:

    1. Plug the flashed USB dongle into the host PC.
    2. Power the WULPUS probe.
    3. Check the dongle LEDs for a successful BLE connection (see figure below). If the dongle did not connect, consult [Troubleshooting](troubleshooting.md).

![LED locations on the USB dongle and their indications.](figures/pictures/dongle_leds.png){ width="80%" }

*LED locations on the USB dongle and their indications. Note: after running at least one measurement, the green LED does not show the BLE connection status anymore. It just keeps the last state when the stop command arrived.*

!!! note
    From now on, we call **acquisition** a certain number of samples with a period between them determined by the ADC sampling frequency. Moreover, we refer to a set of acquisitions as **measurement**.

## MUX artifact mitigation {#mux-artifact-mitigation}

In order to switch between the TX and RX phases of an acquisition, the WULPUS system uses a high-voltage multiplexer. This multiplexer behaves non-ideally and introduces some charge injection during the switching event, leading to a voltage spike artifact that will be picked up by the ADC during acquisition. The more WULPUS channels are connected to receive, the larger is the artifact.

!!! warning "Screenshots show the legacy Jupyter GUI"
    The mitigation methods below are still valid. The annotated GUI screenshots come from the previous Jupyter GUI and will be updated to BioGUI later.

The figure below displays the MUX artifact during an acquisition when a 2.25 MHz linear array transducer (we use only 8 channels) was used. Eight channels are excited at transmit and the same 8 channels are activated at receive phase resulting in the largest-possible switching artifact (worst case). The signal was acquired from the vertical nylon scatters (0.1 mm diameter, 10 mm spacing) of the CIRS 040 GSE phantom. To collect the data shown here, the ultrasound-subsystem settings in figure (b) were used.

![Annotated WULPUS GUI screenshot of the HV MUX artifact.](figures/gui/hv_mux_artifact.png){ width="45%" }
![Measurement settings.](figures/gui/hv_mux_artifact_settings_1.png){ width="50%" }

*(a) Annotated WULPUS GUI screenshot. (b) Measurement settings. High voltage multiplexer switching artifact with default settings.*

There are three main ways to mitigate and adjust this artifact, which will be described individually in the following subsections.

### HV MUX start time

Since a user can configure the time of switching from TX to RX state (see [Advanced Settings](advanced-settings.md#configuration-of-ultrasound-subsystem) for details), to mitigate the artifact, we can reduce the parameter **HV-MUX RX start time** from the default value of 500 µs to e.g. 494 µs. We should not make this value too low, otherwise switching will intersect with the pulsing event, and the transducers will not be excited.

![Annotated WULPUS GUI screenshot with early MUX switching.](figures/gui/hv_mux_artifact_2.png){ width="45%" }
![Measurement settings for early MUX switching.](figures/gui/hv_mux_art_settings_2.png){ width="50%" }

*(a) Annotated WULPUS GUI screenshot. (b) Measurement settings. High voltage multiplexer switching artifact with early multiplexer switching (**HV-MUX RX start time** = 494 µs).*

The figures demonstrate that early switching shifts the artifact to the left, while the back-scattered echo signals remain on their places on the timeline.

### ADC sampling start time

Another way of adjusting the artifact is to tune the **ADC sampling start time**, such that the ADC only starts sampling after the whole or most of the artifact has passed. Below we compare the acquisitions with two different ADC sample start timings, with the default and with a delayed start.

![Annotated WULPUS GUI screenshot with delayed ADC sampling.](figures/gui/hv_mux_artifact_3.png){ width="45%" }
![Measurement settings for delayed ADC sampling.](figures/gui/hv_mux_art_settings_3.png){ width="50%" }

*(a) Annotated WULPUS GUI screenshot. (b) Measurement settings. High voltage multiplexer switching artifact with early multiplexer switching and delayed ADC sampling start (**ADC sampling start time** = 509 µs).*

The starting time is set to 509 µs. As the **ADC sampling start time** is set to a later time, the MUX artifact moves to the left. This method doesn't reduce the dimensions of the artifact (duration or amplitude), but tries to reduce the space the artifact takes up in the number of samples we set up. The downside is that if important data were overlaid over the artifact and we removed the artifact entirely by starting the ADC even later, we may omit the important data. This would e.g. be the case if we are measuring shallow arteries, where the first wall has reflections which lie in the duration of the artifact.

### Switching optimization

The other method of reducing the effect of the artifact is by actually trying to reduce the dimensions of the artifact.

Since the cause of the artifact is the switching, we have to work on it. The simplest idea may be to reduce the amount of times we switch the channels of the multiplexer by analyzing the sets of transmitting/receiving channels and their intersections. This is what we are trying to do with the **Optimized switching** method of the RX/TX config GUI (see figure below).

![Location of the optimized switching choice in the RX/TX configs GUI](figures/gui/rxtx-config-gui-switching.png)

*Location of the optimized switching choice in the RX/TX configs GUI.*

!!! note
    The RX/TX configuration GUI and its functions are discussed in detail in [Programming TX/RX configurations via GUI](advanced-settings.md#programming-tx-rx-configurations-via-gui).

Through a rather simple algorithm, WULPUS analyzes the configuration sets for TX/RX channels, and tries to pre-configure the states of the switches, so that the amount of switching during a transition from transmit to receive is minimized. The figure below shows an example MUX artifact with optimized switching enabled.

![Annotated WULPUS GUI screenshot with optimized switching.](figures/gui/hv_mux_artifact_4.png){ width="45%" }
![Measurement settings with optimized switching.](figures/gui/hv_mux_art_settings_4.png){ width="50%" }

*(a) Annotated WULPUS GUI screenshot. (b) Measurement settings. High voltage multiplexer switching artifact with all previous settings and enabled **Optimize Switching** option.*

As we can see, employing an algorithm for switching optimization further reduces the artifact. At this point, we can safely increase the PGA receive gain to amplify the echo signal. The figure below demonstrates the amplified echoes obtained under the cumulative acquisition settings. Although the artifact is still present in the raw data, the band-pass filtered signal shows four target echoes with high signal to noise ratio.

![Annotated WULPUS GUI screenshot with increased receive gain.](figures/gui/hv_mux_artifact_5.png){ width="45%" }
![Measurement settings with increased receive gain.](figures/gui/hv_mux_art_settings_5.png){ width="50%" }

*(a) Annotated WULPUS GUI screenshot. (b) Measurement settings. High voltage multiplexer switching artifact with all previous settings, enabled **Optimize Switching** option and increased receive gain (18.2 dB).*

Through an employment and combination of the described methods, the switching artifact of the HV multiplexer can be greatly reduced, allowing for accurate data collection.
