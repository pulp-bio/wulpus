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

#include "uslib.h"

static msp_config_t config;
static bool config_updated = false;

void setNewUsConfig(msp_config_t *newConfig)
{
    config = *newConfig;
    config_updated = true;
    return;
}

bool confUsSubsystem(void)
{

    if (config_updated == false)
        return false;

    // Check if no active conversion is in progress
    if(UUPSCTL & USS_BUSY)
    {
        // Error: conversion is ongoing
        return false;
    }

    // Always triggered in SW
    // Future alternative - USSTRG (see datasheet)
    UUPSCTL = ASQEN + 0x00;

    // // Unlock SDHS register for configuration
    // SDHSCTL3 &= ~(TRIGEN);

    // // Configure SDHS Modulator Optimization
    // // (p. 614 of slau367p)
    // switch (config.pllOutFreq)
    // {
    //     case HSPLL_OUT_80_MHZ:
    //     case HSPLL_OUT_79_MHZ:
    //     case HSPLL_OUT_78_MHZ:
    //     case HSPLL_OUT_77_MHZ:
    //         SDHSCTL7 = MODOPTI3 + MODOPTI2; // 0xC
    //         break;
    //     case HSPLL_OUT_76_MHZ:
    //     case HSPLL_OUT_75_MHZ:
    //     case HSPLL_OUT_74_MHZ:
    //         SDHSCTL7 = MODOPTI3 + MODOPTI2 + MODOPTI0; // 0xD
    //         break;
    //     case HSPLL_OUT_73_MHZ:
    //     case HSPLL_OUT_72_MHZ:
    //     case HSPLL_OUT_71_MHZ:
    //         SDHSCTL7 = MODOPTI3 + MODOPTI2 + MODOPTI1; // 0xE
    //         break;
    //     default:
    //         SDHSCTL7 = MODOPTI3 + MODOPTI2 + MODOPTI1 + MODOPTI0; // 0xF
    //         break;
    // }

    // // Lock SDHS register for configuration
    // SDHSCTL3 |= (TRIGEN);

    // Calculate HSPLL Multiplier
    // (p. 481 of slau367p)
    // PLL out clk. freq = input clk. freq x (PLLM + 1)
    // The final output clk. freq. = PLL out clk. freq / 2

    uint16_t tempVar = (uint16_t) config.pllOutFreq;

    // Divide the frequency depending on the XTAL frequency
    switch(config.xtalFreq)
    {
        case HSPLL_XTAL_FREQ_4_MHZ:
            // x2 (see formula above) and then /4 => /2
            tempVar >>= 1;
            break;
        case HSPLL_XTAL_FREQ_8_MHZ:
            // x2 (see formula above) and then /8 => /4
            tempVar >>= 2;
            break;
        default:
            return 0;
    }

    tempVar--;
    // PLLM is 5 bits from 10 to 15.
    tempVar = (tempVar<<10);

    // If input frequency is > 6 MHz => set PLLINFREQ bit
    if(config.xtalFreq ==  HSPLL_XTAL_FREQ_8_MHZ)
    {
        HSPLLCTL = tempVar + PLLINFREQ;
    }
    else
    {
        HSPLLCTL = tempVar;
    }

    // Configure HSPLLUSSXTLCTL register based on HSPLL input frequency clock
    // type (crystal or ceramic resonator) and based on user selection to output
    // buffered HSPLL clock or not

    //  Enable USSXT buffered output?
    if(config.outEnPllXtal == true)
    {
        HSPLLUSSXTLCTL = config.xtalType;
    }
    else
    {
        HSPLLUSSXTLCTL = config.xtalType | XTOUTOFF;
    }

    // Prepare acquisition sequencer and Programmable Pulse Generator (PPG)
    // for configuration
    SAPH_AKEY = KEY;

    //// Configure bias impedance generator ////

    // Unlock SAPH and SAPH trim registers
    // Unlock trim register to be able to modify SAPHMCNF register
    SAPH_ATACTL |= (UNLOCK);

    // Clear currently configured bias impedance generator

    // As per slau367p (page 536)
    // Bias generator Impedance configuration. These bits define the
    // impedance of the buffers for RxBias and TxBias. While for resistive
    // loads the lowest impedance shows the fastest settling, this is not the
    // case for reactive loads.

    SAPH_AMCNF &= ~(BIMP_3);
    switch (config.biasImp) {
        case BIAS_IMP_500_OHM:
            SAPH_AMCNF |= (BIMP_0);
            break;
        case BIAS_IMP_900_OHM:
            SAPH_AMCNF |= (BIMP_1);
            break;
        case BIAS_IMP_1500_OHM:
            SAPH_AMCNF |= (BIMP_2);
            break;
        case BIAS_IMP_2950_OHM:
            SAPH_AMCNF |= (BIMP_3);
            break;
    }

    switch (config.chargePumpMode) {
        case CHARGE_PUMP_ALWAYS_ON:
            // RX input multiplexer charge pump is on
            // during the capture
            SAPH_AMCNF |= (CPEO);
            break;
        case CHARGE_PUMP_NORMAL_MODE:
            // Off during capture
            SAPH_AMCNF &= ~(CPEO);
            break;
    }

    // Lock trim register
    SAPH_ATACTL &= ~(UNLOCK);


    // Disable ACQ and PPG
    SAPH_AASCTL0 &= ~(ASQTEN);
    SAPH_APGCTL &= ~(PPGEN);

    //// Configure PPG (single tone generation) ////
    if (confPPG() != true)
        return 0;

    //// Configure SAPH Acquisition sequencer ////

    // Configure Bias Control registers
    // Defaulting excitation bias to 0.4V Nominal
    // Enable CH1 TX voltage
    SAPH_ABCTL = (ASQBSC_1 | EXCBIAS_2 | CH1EBSW | PGABSW);
    // Configure mux to select correct RX channel
    SAPH_AICTL0 = (DUMEN | MUXCTL | MUXSEL_0);

    // Configure SAPH Acquisition sequencer
    // CH1
    SAPH_AASCTL0 = TRIGSEL_1 + ASQCHSEL_1;
    // Standby state after the end of the sequence
    SAPH_AASCTL1 = 0 ;
    // Selects pulse polarity
    // Starts either with high or low pulse
    SAPH_AAPOL = config.pulserPolarity;

    // Select pause state
    if(config.pulserPauseState == PPG_PAUSE_STATE_LOW)
    {
        SAPH_AAPHIZ = 0;
        SAPH_AAPLEV = 0;
    }
    else if(config.pulserPauseState == PPG_PAUSE_STATE_HIGH)
    {
        SAPH_AAPHIZ = 0;
        SAPH_AAPLEV = 0x000F;
    }
    else // High impedance
    {
        SAPH_AAPHIZ = 0x000F;
        SAPH_AAPLEV = 0;
    }

    // Configure SAPH time marks
    SAPH_AATM_A = config.startPpgCnt;
    SAPH_AATM_B = config.turnOnAdcCnt;
    SAPH_AATM_C = config.startPgaInBiasCnt;
    SAPH_AATM_D = config.startAdcSamplCnt;
    SAPH_AATM_E = config.restartCaptCnt;
    SAPH_AATM_F = config.captTimeoutCnt;

    // Acquisition sequencer trigger enable
    SAPH_AASCTL0 |= (ASQTEN);

    // Lock SAPH registers
    SAPH_AKEY = 0;

    SAPH_AKEY = KEY;
    SAPH_ATACTL |= (UNLOCK);

    // Configure ULP bias configuration
    UUPSCTL &= ~(LBHDEL_3);
    switch (config.uupsBiasDelay)
    {
       case UUPS_BIAS_NO_DELAY:
           // Low power bias mode enable
           SAPH_AMCNF &= ~(LPBE);
           UUPSCTL |= (LBHDEL_0);
           break;

       case UUPS_BIAS_100_USEC:
           SAPH_AMCNF |= (LPBE);
           UUPSCTL |= (LBHDEL_1);
           break;

       case UUPS_BIAS_200_USEC:
           SAPH_AMCNF |= (LPBE);
           UUPSCTL |= (LBHDEL_2);
           break;

       case UUPS_BIAS_300_USEC:
           SAPH_AMCNF |= (LPBE);
           UUPSCTL |= (LBHDEL_3);
           break;

       default:
           // Default is no ULP delay
           SAPH_AMCNF &= ~(LPBE);
           UUPSCTL |= (LBHDEL_0);
           break;
    }

    // Lock SAPH registers and lock trim registers
    SAPH_ATACTL &= ~(UNLOCK);
    SAPH_AKEY = 0;

    // Unlock SDHS register for configuration
    SDHSCTL3 &= ~(TRIGEN);

    // Configure SDHS.CTL0, SDHS.CTL1, SDHS.CTL2, SDHS.CTL6, SDHS.CTL7,
    // SDHS.WINHITH, SDHS.WINLOTH, SDHS.DTCSA  registers
    SDHSCTL0 = TRGSRC + SHIFT_0 + OBR_0 + DFMSEL_0 + DALGN_0 + + INTDLY_0 +
           AUTOSSDIS;
    SDHSCTL1 = config.overSamplRate;

    SDHSCTL2 = DTCOFF_0 + (config.sampleSize - 1);


    //// Configure PGA Gain ////
    SDHSCTL6 = config.rxGain;

    // Configure SDHS Modulator Optimization
    // (p. 614 of slau367p)
    switch (config.pllOutFreq)
    {
        case HSPLL_OUT_80_MHZ:
        case HSPLL_OUT_79_MHZ:
        case HSPLL_OUT_78_MHZ:
        case HSPLL_OUT_77_MHZ:
            SDHSCTL7 = MODOPTI3 + MODOPTI2; // 0xC
            break;
        case HSPLL_OUT_76_MHZ:
        case HSPLL_OUT_75_MHZ:
        case HSPLL_OUT_74_MHZ:
            SDHSCTL7 = MODOPTI3 + MODOPTI2 + MODOPTI0; // 0xD
            break;
        case HSPLL_OUT_73_MHZ:
        case HSPLL_OUT_72_MHZ:
        case HSPLL_OUT_71_MHZ:
            SDHSCTL7 = MODOPTI3 + MODOPTI2 + MODOPTI1; // 0xE
            break;
        default:
            SDHSCTL7 = MODOPTI3 + MODOPTI2 + MODOPTI1 + MODOPTI0; // 0xF
            break;
    }

    // Reset SDHSCTL4 and SDHSCTL5 registers
    SDHSCTL4 = 0;
    SDHSCTL5 = 0;

    // Configure DTC destination offset address
    SDHSCTL4 &= ~(SDHSON);
    // Unlock SDHS registers
    SDHSCTL3 &= ~(TRIGEN);
    //Restore SDHSDTCDA address
    // LEA start address (0x4000)
    // Destination location = base address + DTCDA x 2
    SDHSDTCDA = ((uint32_t)((0x4005) - (0x4000))>>1);
    // Lock SDHS registers
    SDHSCTL3 |= (TRIGEN);

    return true;
}


static inline bool confPPG(void)
{
    // Refer to the slau367p (page 498)

    uint64_t temp;
    uint32_t hspllFreq;
    volatile uint16_t lper;
    volatile uint16_t per, hper;

    hspllFreq = (uint32_t)(config.pllOutFreq) * 1000000;

    // Configure Drive strength
    SAPH_AOCTL1 = ((config.driveStrength << 1) + (config.driveStrength));


    // Calculate the period
    temp = (uint64_t)((uint64_t)hspllFreq + ((uint64_t)config.pulseFreq >> 1));
    temp /= (uint64_t)(config.pulseFreq);
    per = (uint16_t) temp;


    // Calculate the ON time
    temp = (uint64_t)((uint64_t)hspllFreq * (uint64_t)config.pulsesDutyCycle);
    temp = (uint64_t)((uint64_t)temp - ((uint64_t)(config.pulseFreq) >> 1));
    temp /= (uint64_t)(config.pulseFreq);
    hper = (uint16_t) ((temp + 99)/100);

    // Calculate OFF time
    lper = per - hper;

    // Check for the maximum value
    if((hper > 255) || (lper > 255))
    {
        // PPG cannot generate the selected frequency (too low)
        return false;
    }
    else
    {
        // Start PPG Configuration
        SAPH_APGC = ((config.numPulses) |
                    ((config.numStopPulses) << 8));

        SAPH_AXPGCTL = (ETY_0 | XMOD_0);

        SAPH_APGLPER = lper;
        SAPH_APGHPER = hper;
    }

    // Configure Trigger from ACQ, channel skection by ASQ
    SAPH_APGCTL |= (TRSEL_1 + PGSEL_1);

    // Configure USS CH0 and CH1 to be configured by the PPG
    SAPH_AOSEL |= (PCH0SEL_1 | PCH1SEL_1);
    // PPG configuration done
    SAPH_APGCTL |= (PPGEN);

    return true;
}


bool triggerUsAcq(void)
{

    // Configure SAPH
    // Unlock SAPH
    SAPH_AKEY = KEY;

    // The ASQ sends a power down request to the
    // PSQ (Power Sequencer) when the OFF request is received.
    // Enbable OFF request when ASQ completes the measurement sequences
    SAPH_AASCTL1 &= ~(STDBY);
    // OFF request is generated after sequence
    SAPH_AASCTL1 |= ESOFF;

    // // Lock SAPH registers
    // SAPH_AKEY = 0;

    // // Unlock SAPH
    // SAPH_AKEY = KEY;

    // Turn on USSXTAL
    // (Step 2 of the USSXT start-up seq)
    // slau367p page 481
    HSPLLUSSXTLCTL |= USSXTEN;

    // Clear any pending USS Interrupts
    SAPH_AICR = (DATAERR | TMFTO | SEQDN | PNGDN);
    SDHSICR = (WINLO | WINHI | DTRDY | SSTRG | ACQDONE | OVF);
    UUPSICR = (PTMOUT | STPBYDB);
    HSPLLICR = (PLLUNLOCK);

    // Clear all event flags
    clearEventFlag(ALL_EVENTS_MASK);

    // Enable interrupts
    UUPSIMSC |= (PTMOUT | STPBYDB);
    HSPLLIMSC |= (PLLUNLOCK);

    SAPH_AIMSC |= (DATAERR | TMFTO | SEQDN);

    // Clear ISTOP bit before triggering capture
    SDHSICR |= (ISTOP);

    // Clear current sequence selection early bias switch selection
    SAPH_ABCTL &= ~(CH1EBSW | CH0EBSW);
    // Clear current input multiplexer channel selection
    SAPH_AICTL0 &= ~(MUXSEL_15);

    // CH0 TX , CH0 RX
    // ASQ is triggered in software
    SAPH_AASCTL0 = (TRIGSEL_0 | ASQTEN);
    SAPH_ABCTL &= ~(ASQBSC);
    SAPH_ABCTL |= (PGABSW);
    SAPH_AASCTL1 |= (CHOWN);
    // Select Rx Mux input channel_0
    SAPH_AICTL0 |= (MUXSEL_0);

    // Wait for the USSXTLCTL start-up time
    // (Step 3 of the USSXT start-up seq)
    // ~120 us delay
    timerSlowDelay(4, LPM3_bits);

    // Before powering up the USS module wait for USSXT
    // oscillator to start-up (OSCSTATE bit)
    // (Step 4 of the USSXT start-up seq)
    uint8_t ussxtl_timeout = 0;
    while((HSPLLUSSXTLCTL & OSCSTATE_1) != OSCSTATE_1)
    {
        if (ussxtl_timeout > 5)
        {
            // XTAL start-up issue
            // Power Down the XTAL
            HSPLLUSSXTLCTL &= ~(USSXTEN);
            return false;
        }

        // ~ 30 us delay
        timerSlowDelay(1, LPM3_bits);
        ussxtl_timeout++;
    }

    // (Step 5 of the USSXT start-up seq)
    // Turn on USS Power and PLL and start measurement
    UUPSCTL |= USSPWRUP;

    // Wait until UUPS module is in READY state
    // (Ideally can be done after a low-power delay)
    uint8_t uups_timeout = 0;
    while((UUPSCTL & UPSTATE_3) != UPSTATE_3)
    {
        if (uups_timeout > 5)
        {
            // UUPS start-up issue
            // Power Down the UUPS
            UUPSCTL |= USSPWRDN;
            return false;
        }

        // ~ 30 us delay
        timerSlowDelay(1, LPM3_bits);
        uups_timeout++;
    }

    // Trigger through the timer interrupt
    // This helps to synchronize the start with the other time-sensitive SW events
    // Such as switching HV MUX to RX
    startTimerFast();

    // Wait for any of the events
    waitEvent((SAPH_SEQ_ACQ_DONE_EVENT)  ||
              (UUPS_INTERRUPT_DBG_EVENT) ||
              (HS_PLL_UNLOCK_EVENT), false, LPM0_bits);

    // Configure GPIOs after conversion
    // E.g. disable OPA

    if (isEventFlagSet(HS_PLL_UNLOCK_EVENT) == true)
    {
        // Power Down the UUPS
        UUPSCTL |= USSPWRDN;
        return false;
    }
    else if (isEventFlagSet(UUPS_INTERRUPT_DBG_EVENT) == true)
    {
        return false;
    }

    // Power Down the UUPS after the acquisition is complete
    UUPSCTL |= USSPWRDN;

    // Power off USSXTAL
    HSPLLUSSXTLCTL &= ~USSXTEN;

    // Power down SDHS
    SDHSCTL4 &= ~(SDHSON);
    // Unlock SDHS registers
    SDHSCTL3 &= ~(TRIGEN);
    // Restore SDHSDTCDA address
    // LEA start address (0x4000)
    // Destination location = base address + DTCDA x 2
    SDHSDTCDA = ((uint32_t)((0x4005) - (0x4000))>>1);
    // Lock SDHS registers
    SDHSCTL3 |= (TRIGEN);

    return true;
}

void pllUnlockCallback(void)
{
    // Troubleshooting as described in slau367p (page 481)

    HSPLLIMSC &= ~(PLLUNLOCK);

    // Reset USS Module
    UUPSCTL |= (USSSWRST);

    // Release from reset
    UUPSCTL &= ~(USSSWRST);

    // Configure
    confUsSubsystem();

    return;
}


void waitTimerSlowElapse(void)
{
    waitEvent(TIMER_SLOW_CCR0_EVENT, true, LPM3_bits);
}

void confTimerSlowSwEvents(void)
{
    // Stop Slow Timer
    timerStop(TIMER_SLOW_BASE);

    // Configure measurement period
    timerSetCcReg(TIMER_SLOW_BASE,
                  config.measPeriod,
                  OFS_TAxCCR0,
                  true,
                  false);

    // Configure the time to enable DC-DC converter
    timerSetCcReg(TIMER_SLOW_BASE,
                  config.dcDcTurnOnTime,
                  OFS_TAxCCR2,
                  true,
                  false);

    // Clear interrupt flags
    timerClearCcIntFlag(TIMER_SLOW_BASE, OFS_TAxCCTL0);
    timerClearCcIntFlag(TIMER_SLOW_BASE, OFS_TAxCCTL2);

    // Enable interrupts
    timerEnableCcInt(TIMER_SLOW_BASE, OFS_TAxCCTL0);
    timerEnableCcInt(TIMER_SLOW_BASE, OFS_TAxCCTL2);

    // Resume Slow Timer
    timerStartContinuous(TIMER_SLOW_BASE);
}

void reloadTimerSlowSwEvents(void)
{
    uint16_t counter;

    counter = HWREG16(TIMER_SLOW_BASE + OFS_TAxR);

    // Reload measurement period
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCCR0) = counter + config.measPeriod;

    // Reload DC-DC turn on time
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCCR2) = counter + config.dcDcTurnOnTime;

    return;
}

void pauseTimerSlowSwEvents(void)
{
    // Disable interrupts associated with US acquisition
    // Leave only low-level delay functionality (CCR1)

    // Stop Slow Timer
    timerStop(TIMER_SLOW_BASE);

    // Clear interrupt flags
    timerClearCcIntFlag(TIMER_SLOW_BASE, OFS_TAxCCTL0);
    timerClearCcIntFlag(TIMER_SLOW_BASE, OFS_TAxCCTL2);

    // Enable interrupts
    timerDisableCcInt(TIMER_SLOW_BASE, OFS_TAxCCTL0);
    timerDisableCcInt(TIMER_SLOW_BASE, OFS_TAxCCTL2);

    // Resume Slow Timer
    timerStartContinuous(TIMER_SLOW_BASE);
}

void confTimerFastSwEvents(void)
{
    // Stop Fast Timer
    timerStop(TIMER_FAST_BASE);

    // Configure time to start acquisition
    // with respect to the SW trigger from main.c
    timerSetCcReg(TIMER_FAST_BASE,
                  ACQUIS_START_DELAY_SMCLK_CYCLES,
                  OFS_TAxCCR1,
                  false,
                  false);

    // Configure the time to enable HV MUX
    // switching to receive mode
    timerSetCcReg(TIMER_FAST_BASE,
                  config.startHvMuxRxCnt,
                  OFS_TAxCCR0,
                  false,
                  false);

    // Clear interrupt flags
    timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL0);
    timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL1);

    // Enable interrupts
    timerEnableCcInt(TIMER_FAST_BASE, OFS_TAxCCTL1);
    // Interrupt CC0 is enabled later
    return;
}

void startTimerFast(void)
{
    // Configure interrupts
    timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL0);
    timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL1);

    timerEnableCcInt(TIMER_FAST_BASE, OFS_TAxCCTL1);
    timerDisableCcInt(TIMER_FAST_BASE, OFS_TAxCCTL0);

    // Start timer in continuous mode from 0
    timerSetCcReg(TIMER_FAST_BASE, 0,
                  OFS_TAxR,
                  false, false);

    timerStartContinuous(TIMER_FAST_BASE);
}

void triggerAcqTimerFastEvent(void)
{
    // Stop fast timer
    timerStop(TIMER_FAST_BASE);

    // Disable CC1 interrupt and clear flag
    timerDisableCcInt(TIMER_FAST_BASE, OFS_TAxCCTL1);
    timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL1);

    // Instead enable high-priority CC0 interrupt
    // responsible for switching HV MUX
    timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL0);
    timerEnableCcInt(TIMER_FAST_BASE, OFS_TAxCCTL0);

    // Start timer in continuous mode from 0
    // Start timer in continuous mode from 0
    timerSetCcReg(TIMER_FAST_BASE, 0,
                  OFS_TAxR,
                  false, false);

    timerStartContinuous(TIMER_FAST_BASE);

    // Trigger ASQ
    SAPH_AASQTRIG = ASQTRIG;

    return;
}
