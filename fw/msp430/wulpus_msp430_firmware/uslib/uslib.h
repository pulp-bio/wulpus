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

#ifndef USLIB_USLIB_H_
#define USLIB_USLIB_H_

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "uslib_timers_isrs.h"

// Maximum number of the TX/RX configs
#define TX_RX_CONF_LEN_MAX    16

// Typedef for HSPLL output frequencies
typedef enum
{
    HSPLL_OUT_68_MHZ = 68,
    HSPLL_OUT_69_MHZ,
    HSPLL_OUT_70_MHZ,
    HSPLL_OUT_71_MHZ,
    HSPLL_OUT_72_MHZ,
    HSPLL_OUT_73_MHZ,
    HSPLL_OUT_74_MHZ,
    HSPLL_OUT_75_MHZ,
    HSPLL_OUT_76_MHZ,
    HSPLL_OUT_77_MHZ,
    HSPLL_OUT_78_MHZ,
    HSPLL_OUT_79_MHZ,
    HSPLL_OUT_80_MHZ,

} hspll_clk_out_freq_t;

// Typedef for HSPLL XTAL frequency 
typedef enum
{
    HSPLL_XTAL_FREQ_8_MHZ,
    HSPLL_XTAL_FREQ_4_MHZ,

} hspll_xtal_freq_t;

// Typedef for the type of the XTAL ascillator 
typedef enum
{
    HSPLL_XTAL_CRYSTAL = 0,
    HSPLL_XTAL_CERAMIC_RESONATOR = 0x0200,

} hspll_xtal_type_t;

// Typedef for the bias impedance
typedef enum
{
    BIAS_IMP_500_OHM,
    BIAS_IMP_900_OHM,
    BIAS_IMP_1500_OHM,
    BIAS_IMP_2950_OHM,

} bias_impedance_t;

// Typedef for the charge pump mode
typedef enum
{
    CHARGE_PUMP_NORMAL_MODE,
    CHARGE_PUMP_ALWAYS_ON
} mux_charge_pump_mode_t;

// PPG start pulse polarity typedef
typedef enum
{
    PPG_POLARITY_START_WITH_HIGH = 0x0000,
    PPG_POLARITY_START_WITH_LOW = 0x000F,
} ppg_pulse_polarity_t;

// PPG pause state typedef
typedef enum
{
    PPG_PAUSE_STATE_LOW,
    PPG_PAUSE_STATE_HIGH,
    PPG_PAUSE_STATE_HIGH_IMPEDANCE,

} ppg_pause_state_t;

// PPG drive strength typedef
typedef enum
{
    PPG_NORMAL_DRIVE = 0,
    PPG_MAX_DRIVE = 0x100
} ppg_drive_strength_t;


// Typedef for the bias delay of the ultrasound universal power supply
typedef enum
{
    UUPS_BIAS_NO_DELAY,
    UUPS_BIAS_100_USEC,
    UUPS_BIAS_200_USEC,
    UUPS_BIAS_300_USEC,

} uups_bias_delay_t;

// Oversampling rate of the sigma delta ADC
typedef enum
{
    SDHS_OVER_SAMPL_RATE_10,
    SDHS_OVER_SAMPL_RATE_20,
    SDHS_OVER_SAMPL_RATE_40,
    SDHS_OVER_SAMPL_RATE_80,
    SDHS_OVER_SAMPL_RATE_160,

} sdhs_over_sampl_rate_t;

// Programmable RX gain typedef
typedef enum
{
    PGA_GAIN_MINUS_6_5_DB = 17,
    PGA_GAIN_MINUS_5_5_DB,
    PGA_GAIN_MINUS_4_6_DB,
    PGA_GAIN_MINUS_4_1_DB,
    PGA_GAIN_MINUS_3_3_DB,
    PGA_GAIN_MINUS_2_3_DB,
    PGA_GAIN_MINUS_1_4_DB,
    PGA_GAIN_MINUS_0_8_DB,
    PGA_GAIN_0_1_DB,
    PGA_GAIN_1_0_DB,
    PGA_GAIN_1_9_DB,
    PGA_GAIN_2_6_DB,
    PGA_GAIN_3_5_DB,
    PGA_GAIN_4_4_DB,
    PGA_GAIN_5_2_DB,
    PGA_GAIN_6_0_DB,
    PGA_GAIN_6_8_DB,
    PGA_GAIN_7_7_DB,
    PGA_GAIN_8_7_DB,
    PGA_GAIN_9_0_DB,
    PGA_GAIN_9_8_DB,
    PGA_GAIN_10_7_DB,
    PGA_GAIN_11_7_DB,
    PGA_GAIN_12_2_DB,
    PGA_GAIN_13_0_DB,
    PGA_GAIN_13_9_DB,
    PGA_GAIN_14_9_DB,
    PGA_GAIN_15_5_DB,
    PGA_GAIN_16_3_DB,
    PGA_GAIN_17_2_DB,
    PGA_GAIN_18_2_DB,
    PGA_GAIN_18_8_DB,
    PGA_GAIN_19_6_DB,
    PGA_GAIN_20_5_DB,
    PGA_GAIN_21_5_DB,
    PGA_GAIN_22_0_DB,
    PGA_GAIN_22_8_DB,
    PGA_GAIN_23_6_DB,
    PGA_GAIN_24_6_DB,
    PGA_GAIN_25_0_DB,
    PGA_GAIN_25_8_DB,
    PGA_GAIN_26_7_DB,
    PGA_GAIN_27_7_DB,
    PGA_GAIN_28_1_DB,
    PGA_GAIN_28_9_DB,
    PGA_GAIN_29_8_DB,
    PGA_GAIN_30_8_DB,

} pga_gain_t;

// Around 9 uS
#define ACQUIS_START_DELAY_SMCLK_CYCLES    72

// MSP ultrasound sybsystem configuration struct
typedef struct
{
    hspll_clk_out_freq_t pllOutFreq;
    hspll_xtal_freq_t xtalFreq;
    hspll_xtal_type_t xtalType;
    // Enables/Disables buffered output clock
    bool outEnPllXtal;

    // Internal MUX settings
    bias_impedance_t biasImp;
    mux_charge_pump_mode_t chargePumpMode;

    uups_bias_delay_t uupsBiasDelay;

    // Six time marks events
    uint16_t startPpgCnt;
    uint16_t turnOnAdcCnt;
    uint16_t startPgaInBiasCnt;
    uint16_t startAdcSamplCnt;
    uint16_t restartCaptCnt;
    uint16_t captTimeoutCnt;

    // Extra time events (SW-managed)
    uint16_t startHvMuxRxCnt;
    uint16_t dcDcTurnOnTime;


    // Acquisition settings
    sdhs_over_sampl_rate_t overSamplRate;
    uint16_t sampleSize;
    uint8_t  rxGain;
    uint16_t measPeriod;

    // TX/RX configurations
    uint8_t  txRxConfLen;
    uint16_t txConfigs[TX_RX_CONF_LEN_MAX];
    uint16_t rxConfigs[TX_RX_CONF_LEN_MAX];

    // Pulser settings
    ppg_drive_strength_t driveStrength;

    uint32_t transFreq; // Reserved, not used
    uint32_t pulseFreq;
    uint8_t  pulsesDutyCycle; // 0-255 ( 0 - 100 %)
    uint8_t  numPulses;
    uint8_t  numStopPulses;
    ppg_pulse_polarity_t pulserPolarity;
    ppg_pause_state_t pulserPauseState;

} msp_config_t;

//// High-level Ultrasound routines /////

void setNewUsConfig(msp_config_t *newConfig);
bool confUsSubsystem(void);
static inline bool confPPG(void);
bool triggerUsAcq(void);

//// Helper-Ultrasound functions ////

void pllUnlockCallback(void);

// Slow-timer related functions
void waitTimerSlowElapse(void);
void confTimerSlowSwEvents(void);
void reloadTimerSlowSwEvents(void);
void pauseTimerSlowSwEvents(void);

// Fast timer related functions
void confTimerFastSwEvents(void);
void startTimerFast(void);
void triggerAcqTimerFastEvent(void);


#endif /* USLIB_USLIB_H_ */
