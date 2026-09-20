# Advanced Settings

This chapter describes the advanced settings of the WULPUS platform which can be tuned for a specific transducer and application.

!!! warning "Screenshots show the legacy Jupyter GUI"
    TX/RX and ultrasound-subsystem screenshots on this page still come from the original Jupyter notebook GUI. The configuration parameters themselves are unchanged; the GUI walkthrough will be updated to BioGUI in a follow-up.

## Transmit/Receive (TX/RX) configurations {#tx-rx-configurations}

### Theoretical overview

To perform an acquisition, WULPUS needs to know on which channels to transmit (receive) the pulses. This is done by supplying an array of TX (RX) channels. These two inputs (TX and RX array) determine which elements of the transducer array are active during acquisition.

**TX channels**

Multiple TX channels can be activated at a time, reflecting the elements of the transducer array that will transmit a signal simultaneously. This array of TX channels allows for a broader or more focused area of signal transmission, depending on the experiment's requirements.

**RX channels**

Multiple RX channels can be input at a time, reflecting the elements of the transducer array that will be connected in parallel to receive a signal simultaneously.

!!! note
    Since WULPUS has a single ADC channel, multiple channels can not be sampled simultaneously. Instead, when activating a few channels in parallel for the reception, the echo signals from all the connected channels (transducer elements) will be summed up in the analog domain and then sampled by the ADC.

One set of TX/RX channels can be either TX or RX channels or both simultaneously. This set is also called "configuration set".

WULPUS supports multiple (16 max.) configuration sets. These configurations are stored and will be executed in a round-robin fashion. This means that WULPUS will cycle through the configurations sequentially, using each set for signal transmission and/or reception before moving to the next. This approach is beneficial for conducting experiments that require data from e.g. multiple transducer channels or for quickly iterating through different test scenarios without manual reconfiguration.

When setting up configurations, one must consider their experimental setup and the specific TX/RX channels that need to be activated. For instance, if the experiment involves scanning an area with a transducer array, the user would enable multiple TX channels to cover the area and select an appropriate RX channel to capture the reflected signal.

### Programming TX/RX configurations via GUI {#programming-tx-rx-configurations-via-gui}

A user can use either the Python API or a small GUI to program TX/RX configurations. A GUI-based example is provided below. The configuration shown matches the `Transmit only` example visualized later in [Example configurations](#example-configurations).

![The TX/RX configuration GUI.](figures/gui/rxtx_configs_gui.png)

*The TX/RX configuration GUI. In this example, configuration set 0 is selected, with channels 0, 1, 6 and 7 set to transmit, and no channels selected for receive.*

!!! note
    In the Python API, we are always talking about channels 0–7 (zero-indexed), whereas in the hardware design files we talk about channels 1–8.

    In this user guide, we use the same numbering as in the Python API.

**Configuration selection and activation**

In the top part of the GUI, we have a dropdown list `Config` of all available configuration sets. Choose the configuration set you want to edit here and enable it via the `Enable` checkbox. A user can program multiple configurations which will then be executed in a round robin fashion starting from the config with id=0 and increasing for the next acquisition event(s).

**Channel selection**

The middle part presents a series of buttons for TX and RX channels. These buttons can be toggled to activate (green color) or deactivate (red color) the needed `TX`/`RX` channels in the configuration set.

**Saving configurations to a file**

The lower part of the GUI contains file management controls. A text box is provided to specify a `Filename` for saving configurations. `Save` and `Load` buttons facilitate the storage and retrieval of the configuration settings. The currently enabled list of configuration sets will be saved in a human readable `.json` format.

!!! note
    Disabled configuration sets will not be saved.

### Example configurations {#example-configurations}

Three examples of using the TX/RX configurations are visualized below.

![Three examples of using TX/RX configurations.](figures/example_meas/txrx_demo.png)

*Three examples of using TX/RX configurations where the transmitting channels are indicated in red, the receiving channels in green.*

In the first example denoted as **Transmit only** (left), WULPUS is configured to send out pulses on channels `[0,1,6,7]` and not receive on any channel. This configuration corresponds to the GUI settings demonstrated above. In the second **Receive only** example (center), WULPUS is configured to receive on channel `3` only. In the third example called **Transmit and receive** (right), WULPUS is configured to send and receive on channel `4` simultaneously.

!!! note
    In reality, the system doesn't transmit and receive exactly at the same time. "Simultaneously" means that the same channels are selected for TX and RX within the same excitation/readout cycle.

These three configuration sets could be assigned to different config ids (e.g. to 0, 1, 2) and put in one configuration file. If all configurations are enabled, WULPUS would then execute them in a round-robin manner:

```
Transmit only -> Receive only -> Transmit and receive ->
 -> Transmit only -> Receive only -> Transmit and receive -> ...
```

!!! note
    A user can program a sequence of a maximum of 16 configuration sets.

### "B-mode" data acquisition and visualization {#b-mode}

A user can potentially program 8 TX/RX configurations where each `n`'th config (with an id = `n`) receives the echo signal from the `n`'th channel of the linear array transducer (`n` ranges from 0 to 7). In this case, if the channels are ordered (pin mapping from the transducer's to WULPUS's channels is correct), the GUI can visualize the ultrasound echo data as a 2D image. An example of such an image is shown below.

!!! note
    Although we often call it "B-mode", the GUI does not perform beamforming. Instead, the raw ultrasound data is band-pass filtered, and an envelope (A-scan) is extracted. Later, when all 8 envelopes (A-scans) are collected, they are stacked into a 2D matrix which is visualized.

![The main GUI in B-mode while imaging a carotid artery.](figures/example_meas/B_mode_carotid_GUI.png){ width="80%" }

*The main GUI in B-mode while imaging a carotid artery. A user can activate this mode by clicking on the Show B-mode checkbox.*

Precisely, the GUI takes data from the `n`'th config set and plots it along the x-axis (depth) at coordinate y=`n` (there are eight discrete y-coordinates). A user thus needs to program eight TX/RX config sets, where for each config set one channel is activated to RX. For the best image quality, we suggest activating all the channels during TX.

For the example measurement shown above, the employed TX/RX configs were the following:

| **Config set no.** | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| **TX channels** | 0–7 | 0–7 | 0–7 | 0–7 | 0–7 | 0–7 | 0–7 | 0–7 |
| **RX channels** | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |

*RX/TX configs for B-mode.*

## Configuration of Ultrasound Subsystem {#configuration-of-ultrasound-subsystem}

!!! note
    As defined earlier, we call **acquisition** a certain number of samples with a period between them determined by the ADC sampling frequency. Moreover, we call a set of acquisitions a **measurement**.

After power-up, the WULPUS probe waits for the configuration package to arrive from the host PC (from the Python GUI). This configuration package contains TX/RX configurations (see [Programming TX/RX configurations via GUI](#programming-tx-rx-configurations-via-gui)) along with the settings of the ultrasound subsystem responsible for managing all the steps of the acquisition (e.g. pulse generation, start ADC sampling etc). The table below contains the full list of the settings available for the user.

**Ultrasound subsystem configuration parameters**

| Parameter name | Display name (in GUI) | Description/Note |
| --- | --- | --- |
| ***Acquisition settings*** | | |
| `num_acqs` | Number of acquisitions | *N* acquisitions to collect |
| `meas_period` | Measuring period [us] | Time between acquisitions |
| `sampling_freq` | Sampling frequency [Hz] | ADC Sampling frequency |
| `num_samples` | Number of samples | *K* samples per acquisition |
| `rx_gain` | RX gain [dB] | Gain of the MSP430 PGA |
| ***Excitation settings*** | | |
| `pulse_freq` | Pulse frequency [Hz] | Frequency of the pulses |
| `num_pulses` | Number of pulses | *L* pulses to generate |
| ***Advanced settings (special time events)*** | | |
| `start_hvmuxrx` | HV-MUX RX start time [us] | MUX switches from TX to RX |
| `dcdc_turnon` | DC-DC turn on time [us] | DC-DC converter is enabled |
| `start_ppg` | PPG start time [us] | Pulse Generation starts |
| `turnon_adc` | ADC turn on time [us] | ADC is turned on |
| `start_pgainbias` | PGA in bias start time [us] | Input biasing is enabled |
| `start_adcsampl` | ADC sampling start time [us] | ADC starts sampling |
| `restart_capt` | Capture restart time [us] | (reserved, not used) |
| `capt_timeout` | Capture timeout time [us] | (reserved, not used) |

In total, there are three groups of configuration settings which are described in detail below.

**Measurement settings**

The first group is responsible for configuring general measurement settings. Particularly, `meas_period` defines the period between the consecutive acquisitions and is inversely proportional to the Acquisition Per Second (APS) rate. In its turn, `num_acqs` defines the number of acquisitions to be collected by the GUI before stopping the measurement.

!!! note
    `num_acqs` is relevant only for the Python GUI. The GUI is responsible for starting the WULPUS probe and later stopping it (thereby terminating the data collection) when the programmed number of acquisitions is received.

The ADC sampling frequency can be adjusted by changing the parameter `sampling_freq`.

!!! note
    The number of samples per acquisition is currently fixed to 400 samples and can not be changed via the GUI.

**Excitation settings**

The second group of settings determines the excitation pattern for the ultrasound transducer. The WULPUS system produces unipolar excitation pulses with an amplitude of 15 V (from 0 V to +15 V). The frequency of the pulses along with the number of pulses can be changed via GUI.

!!! note
    The duty cycle of the pulses is currently fixed to 50% in the API but can potentially be adjusted at runtime.

**Advanced settings**

The third group is responsible for setting the precise time marks of the acquisition's events. These timings have to fulfill certain requirements for a successful acquisition. Since the understanding of these timings is important to get the full potential out of these settings, the principle of how they work is explained further.

![Timing diagram for the internal events of the MSP430 MCU during a single ultrasound acquisition.](figures/advanced_settings/wulpus-timing-diagram.png)

*Timing diagram for the internal events of the MSP430 MCU during a single ultrasound acquisition. Colors relate internal events to the clock domain: blue denotes events whose time stamps are coupled with the high-speed PLL (HSPLL) clock domain of the MSP430, red corresponds to sub-system master clock (SMCLK), green stands for low frequency crystal (LFXT).*

A short overview of the timings during one acquisition is displayed in the figure above. Preparation for the acquisition starts with the event L<sub>A</sub> responsible for turning on the high-voltage DC-DC converter. This converter supplies the +15 V power domain used to generate excitation pulses. Next, during the L<sub>B</sub> event the MSP430 MCU wakes up, prepares the ultrasound subsystem for the acquisition and activates the universal ultrasound power supply (UUPS) module. This module requires some time to get ready. Therefore, the firmware also initializes a timer (clocked by the SMCLK domain), sets a delay equal to approx. 9 µs and starts the timer. When the timer elapses, an event S<sub>A</sub> happens. Up until this moment, the UUPS module is typically in the READY state (but if not, we wait further until it is ready). Next, we configure the SMCLK-based timer with the timestamp S<sub>B</sub> which will be later responsible for switching the HV multiplexer from the transmit to receive state. Then, we immediately trigger an ultrasound acquisition. Later steps are automatically performed by the dedicated hardware acquisition sequencer (ASQ) of the MSP430 without involving the CPU at all. This means that the hardware automatically applies an RX bias (H<sub>C</sub>), turns on the sigma-delta high-speed ADC (H<sub>B</sub>), triggers the programmable pulse generator (H<sub>A</sub>) and starts sampling the input signal (H<sub>D</sub>). The only exception is the event S<sub>B</sub> when the CPU wakes up from the timer event to switch the HV multiplexer to the receive state. Finally, the acquisition finishes when all the required samples are acquired, and all the US-related peripheral modules are turned off until the L<sub>A</sub> and L<sub>B</sub> events happen again in the next acquisition.

A user can flexibly configure the timings of the internal events. The mapping of the internal parameters to the Python API is summarized below.

| Event | Clock | Parameter in GUI | Function |
| --- | --- | --- | --- |
| L<sub>A</sub> | LFXT | DC-DC turn on time | Turn on HV DC-DC |
| L<sub>B</sub> | LFXT | Measurement Period | Turn on UUPS |
| S<sub>A</sub> | SMCLK | — | Trigger Acquisition Sequencer |
| S<sub>B</sub> | SMCLK | HV-MUX RX start time | Switch HV MUX to receive |
| H<sub>A</sub> | HSPLL | PPG start time | Trigger pulse generator |
| H<sub>B</sub> | HSPLL | ADC turn on time | Turn on sigma-delta ADC |
| H<sub>C</sub> | HSPLL | PGA in bias start time | Apply receive bias |
| H<sub>D</sub> | HSPLL | ADC sampling start time | Start sampling the input signal |

*The six timing points and their corresponding parameters.*

!!! note
    The S<sub>A</sub> event is fixed in firmware with respect to L<sub>B</sub> so that S<sub>A</sub> − L<sub>B</sub> = 9 µs.

!!! note
    L<sub>B</sub> is a central event of the whole acquisition, since it triggers the SMCLK-based timer/events which is responsible for triggering the acquisition sequencer operating in the HSPLL domain (see the timing diagram).

!!! note
    While changing the Measurement Period (L<sub>B</sub> event), a user should also adapt the DC-DC turn on time (L<sub>A</sub> event) so that the HV DC-DC converter has enough time to start up and generate the +15 V power domain.

While adapting the advanced settings, a user must follow the recommendations below (otherwise, the system will not work):

- The DC-DC converter has to be turned on and settled before starting pulsing.
- The pulsing has to be finished before the MUX switches to RX.
- The PGA input (receive) bias has to be settled before the ADC is triggered.
- The ADC has to be turned on and ready before the ADC is triggered.
- The MUX has to be settled to RX early enough to capture early echo-signals.
- The previous acquisition must be finished before the next acquisition starts.

Since the WULPUS probe uses multiple clock domains originating from different sources (see schematics for details), some variations between different WULPUS probes may occur such as slight misalignment of the HV MUX artifact caused by the switching event S<sub>B</sub> (see the timing diagram). For the best results, we recommend starting from the default settings and tuning the **HV MUX RX start time** parameter. The default timings are displayed below.

![Default settings of the WULPUS GUI.](figures/advanced_settings/gui_uss_default.png)

*Default settings of the WULPUS GUI.*
