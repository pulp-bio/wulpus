/*
 * Copyright (C) 2024 ETH Zurich. All rights reserved.
 *
 * Author: Sergei Vostrikov, ETH Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

#include <msp430.h> 
#include "wulpus_sys.h"

#include "uslib_timers_isrs.h"

//// LOCAL MACRO DEFINITIONS AND VARIABLES ////

// First two Bytes of the measurement header
// Used to indicate the start of an US frame
#define MEAS_START_OF_FRAME_MASK 0xFF
// US measurement header
static uint8_t meas_header[4] = {0};
static uint16_t meas_frame_nr = 0;

// Empty config with MSP settings for US acquisition
msp_config_t msp_config;

// TX RX active config ID
uint8_t tx_rx_id = 0;

// A routine to get configuration package from nRF
static void getConfigPack(void);

// High level functions used in main
static void configAfterPowerUp(void);
static void receiveUssConfPackage(void);
static void usAcquisitionLoop(void);

// Callbacks implementation
static void hsPllUnlockCallback(void);
static void saphSeqAcqDoneCallback(void);
static void slowTimerCc2Callback(void);
static void fastTimerCc0Callback(void);

int main(void)
{

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PMM_unlockLPM5();

    // Initial configuration
    // Sets default parameters
    configAfterPowerUp();

    while(1)
    {
        // Set default parameters
        tx_rx_id = 0;
        meas_frame_nr = 0;

        // Receive Uss configuration package from nRF
        receiveUssConfPackage();

        // Configure Uss according to the new package
        confUsSubsystem();

        // Configure the events of slow and fast timers
        confTimerSlowSwEvents();
        confTimerFastSwEvents();

        // Power up HV PCB
        enableHvPcbSupply();
        // Enable Power for OPA836
        enableOpAmpSupply();

        // Enter acquisition loop
        usAcquisitionLoop();

    }
    // Not reachable
	
	return 0;
}

void configAfterPowerUp(void)
{

    // Initialize DMA for SPI transfers and SPI for
    // nRF <-> MSP and MSP <-> HV MUX communication
    usDmaInit();
    usSpiInit();
    hvMuxInit();

    // Init BLE ready input and LED GPIOs
    initOtherGpios();
    // Init power switches
    initAllPowerSwitches();

    // Get default Ultrasound config
    getDefaultUsConfig(&msp_config);

    // Update the internal config of the ultrasound library
    setNewUsConfig(&msp_config);

    // Init timers
    timerSlowInit();
    timerFastInit();

    // Hook timers-related callback

    // Timer Slow CC2 callback enables HV DC-DC and opAmp
    TIMER_SLOW_CCR2_CALLBACK = &slowTimerCc2Callback;
    // Timer Slow CC0 callback reloads parameters of the capture
    // such as measurement period and dc-dc turn on time
    TIMER_SLOW_CCR0_CALLBACK = &reloadTimerSlowSwEvents;

    // Timer Fast CC1 callback triggers acquisition and
    // enables CC0 interrupt
    TIMER_FAST_CCR1_CALLBACK = &triggerAcqTimerFastEvent;
    // Timer Fast CC0 callback switches HV MUX to receive,
    // stops the DC-DC converter and the Fast timer
    TIMER_FAST_CCR0_CALLBACK = &fastTimerCc0Callback;

    // Hook other callbacks
    HS_PLL_UNLOCK_CALLBACK = &hsPllUnlockCallback;
    SAPH_SEQ_ACQ_DONE_CALLBACK = &saphSeqAcqDoneCallback;

    // Configure Ultrasound peripherals
    confUsSubsystem();

    // Configure the events of slow and fast timers
    confTimerSlowSwEvents();
    confTimerFastSwEvents();

    return;
}

static void receiveUssConfPackage(void)

{
    while(1)
    {
        // Sleep for 10 ms
        timerSlowDelay(327, LPM3_bits);

        // Check that nRF52 BLE connection is ready
        if (isBleReady())
        {
            // Receive configuration package from nRF
            getConfigPack();

            // Process received package and update Uss config
            if (extractUsConfig(usSpiGetRxPtr(), &msp_config))
            {
                // Update Ultrasound config
                setNewUsConfig(&msp_config);
                return;
            }
        }
    }
}

static void usAcquisitionLoop(void)
{

    bool no_error = true;

    while(1)
    {
        // Check if nRF52 BLE connection is ready
        if(isBleReady())
        {

            // Update the measurement header
            meas_header[0] = MEAS_START_OF_FRAME_MASK;
            meas_header[1] = tx_rx_id;
            meas_header[2] = (uint8_t) (meas_frame_nr & 0xFF);
            meas_header[3] = (uint8_t) (meas_frame_nr >> 8);
            memcpy((uint16_t *) 0x4000, &meas_header, 4);

            // Configure TX config (applied immediately)
            hvMuxConfTx(msp_config.txConfigs[tx_rx_id]);
            // Configure RX config (loaded into shift register but not latched)
            // Latching will occur in the timer interrupt after completion
            // of pulse generation
            hvMuxConfRx(msp_config.rxConfigs[tx_rx_id]);

            // Trigger ultrasound acquisition
            no_error = triggerUsAcq();
            if (no_error == false)
            {
                // Wait for timer to elapse
                waitTimerSlowElapse();
                continue;
            }

            // If instead aquisition sequencer finished as expected
            // and we reached this line, then
            // wait for the SPI DMA transaction to be completed

            // Wait for SPI DMA transmission to complete
            usWaitForSpiDmaRx();


            // Check the SPI RX buffer for restart command
            if (isRestartCondition(usSpiGetRxPtr()))
            {
                pauseTimerSlowSwEvents();
                return;
            }

            // Wait for timer to elapse
            waitTimerSlowElapse();

            // Increment measurement frame number
            // And TX RX configuration ID
            meas_frame_nr++;
            tx_rx_id++;
            if(tx_rx_id == msp_config.txRxConfLen)
                tx_rx_id = 0;
        }
    }
}

//// HELPER FUNCTIONS  ////

// Get configuration package from nRF
static void getConfigPack(void)
{
    // Initiate an SPI transaction to receive a config file
    // Clear TX buffer
    memset((uint16_t *) 0x4000, 0, (uint32_t)BYTES_PR_XFER_TX);
    // Start SPI transaction
    usStartSPI();

    // Enable DMA SPI interrupt
    // It will wake up the CPU from LPM0
    usSpiEnableDmaRxIsr();

    // Wait for SPI DMA RX to be completed
    usWaitForSpiDmaRx();
}

//// Callbacks implementation ////
static void hsPllUnlockCallback(void)
{
    // TI recommends to turn off USS module and check USSXT oscillator

    pllUnlockCallback();
}

static void saphSeqAcqDoneCallback(void)
{

    // Power Down the UUPS after the acquisition is complete
    UUPSCTL |= USSPWRDN;

    // Disable HV PCB DC-DC converters
    // and OpAmp

    // Disable HV and +5 V
    disableHvPcbDcDc();
    // Disable RX OPA836
    disableOpAmp();

    // if PLL unlock event occurs earlier the acquisition might be not valid
    if (isEventFlagSet(HS_PLL_UNLOCK_EVENT) == false)
    {
        // Enable DMA SPI interrupt
        // It will wake up the CPU from LPM0
        usSpiEnableDmaRxIsr();
        // Start SPI transaction
        usStartSPI();
    }
}

static void slowTimerCc2Callback(void)
{
    // Turn On DC-DCs
    enableHvPcbDcDc();
    // Enable RX OPA836
    enableOpAmp();
}

static void fastTimerCc0Callback(void)
{
    // Switch HV Mux
    hvMuxLatchOutput();
    // Disable HV DC-DC (we don't need V at this point)
    disableHvDcDc();
    // Disable Fast Timer
    timerFastStop();
}
