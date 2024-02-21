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

#include "uslib_timers_isrs.h"

// FRAM variables
#pragma PERSISTENT(TIMER_SLOW_CCR0_CALLBACK)
#pragma PERSISTENT(TIMER_SLOW_CCR1_CALLBACK)
#pragma PERSISTENT(TIMER_SLOW_CCR2_CALLBACK)

#pragma PERSISTENT(TIMER_FAST_CCR0_CALLBACK)
#pragma PERSISTENT(TIMER_FAST_CCR1_CALLBACK)
#pragma PERSISTENT(TIMER_FAST_CCR2_CALLBACK)

#pragma PERSISTENT(HS_PLL_UNLOCK_CALLBACK)
#pragma PERSISTENT(UUPS_PWR_UP_TIMEOUT_CALLBACK)
#pragma PERSISTENT(UUPS_INTERRUPT_DBG_CALLBACK)
#pragma PERSISTENT(SAPH_DATA_ERROR_CALLBACK)
#pragma PERSISTENT(SAPH_TIME_MF_TIMEOUT_CALLBACK)
#pragma PERSISTENT(SAPH_SEQ_ACQ_DONE_CALLBACK)
#pragma PERSISTENT(SAPH_PNGND_CALLBACK)


void (*TIMER_SLOW_CCR0_CALLBACK) (void)=0;
void (*TIMER_SLOW_CCR1_CALLBACK) (void)=0;
void (*TIMER_SLOW_CCR2_CALLBACK) (void)=0;

void (*TIMER_FAST_CCR0_CALLBACK) (void)=0;
void (*TIMER_FAST_CCR1_CALLBACK) (void)=0;
void (*TIMER_FAST_CCR2_CALLBACK) (void)=0;

void (*HS_PLL_UNLOCK_CALLBACK)       (void)=0;
void (*UUPS_PWR_UP_TIMEOUT_CALLBACK) (void)=0;
void (*UUPS_INTERRUPT_DBG_CALLBACK)  (void)=0;
void (*SAPH_DATA_ERROR_CALLBACK)     (void)=0;
void (*SAPH_TIME_MF_TIMEOUT_CALLBACK)(void)=0;
void (*SAPH_SEQ_ACQ_DONE_CALLBACK)   (void)=0;
void (*SAPH_PNGND_CALLBACK)          (void)=0;

uint32_t usEventFlags = 0;

void timerSlowInit(void)
{
    // Clear
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCTL) |= TACLR;
    // Clock from ACLK, divider = 1, counts up to 0xFFFF
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCTL) =
    (TASSEL__ACLK | ID__1 | MC__CONTINUOUS);
    // Extra divider = 1
    HWREG16(TIMER_SLOW_BASE + OFS_TAxEX0) = (TAIDEX_0);
    // Enable Capture compare interrupt 1
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCCTL1) =
            CCIE;

    // Check the fault flag of LXFT oscillator
    // Unlock CS regs
    CSCTL0_H = CSKEY >> 8;
    // Clear LXFT fault condition flags
    CSCTL5 &= ~LFXTOFFG;
    SFRIFG1 &= ~OFIFG;

    while (SFRIFG1 & OFIFG)
    {
        // Clear flags
        CSCTL5 &= ~LFXTOFFG;
        SFRIFG1 &= ~OFIFG;

        // Delay ~100 ms
        timerSlowDelay(3277, LPM3_bits);
    }

    // Lock CS regs
    CSCTL0_H = 0;

}

void timerSlowStop()
{
    timerStop(TIMER_SLOW_BASE);
    return;
}


void timerSlowDelay(uint16_t delay, uint16_t lpmBits)
{
    // Save GIE status
    uint16_t gieStatus = ( __get_SR_register() & GIE);

    // Write delay value to the capture compare reg 1
    timerSetCcReg(TIMER_SLOW_BASE,
                  delay,
                  OFS_TAxCCR1,
                  true,
                  true);

    // Clear pending interrupt flag
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCCTL1) &= ~(CCIFG);

    // Enable CC1 interrupt
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCCTL1) |= (CCIE);

    __disable_interrupt();
    while(isEventFlagSet(TIMER_SLOW_CCR1_EVENT) == false)
    {
        __bis_SR_register(lpmBits + GIE);
        __disable_interrupt();
    }

    // Disable CC1 interrupt
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCCTL1) &= ~(CCIE);

    // Clear interrupt flag
    HWREG16(TIMER_SLOW_BASE + OFS_TAxCCTL1) &= ~(CCIFG);
    // Clear event flag
    clearEventFlag(TIMER_SLOW_CCR1_EVENT);

    // Restore GIE status
    if(gieStatus == GIE)
    {
        __bis_SR_register(GIE);
    }

    return;
}

void timerFastInit(void)
{
    // Clear
    HWREG16(TIMER_FAST_BASE + OFS_TAxCTL) |= TACLR;
    // Clock from ACLK, divider = 1, counts up to 0xFFFF
    HWREG16(TIMER_FAST_BASE + OFS_TAxCTL) =
    (TASSEL__SMCLK | ID__1 | MC__STOP);
    // Extra divider = 1
    HWREG16(TIMER_FAST_BASE + OFS_TAxEX0) = (TAIDEX_0);

    return;
}

void timerFastStop(void)
{
    timerStop(TIMER_FAST_BASE);
    return;
}


void timerSetCcReg(uint16_t timerBase,
                   uint16_t val,
                   uint16_t ccReg,
                   bool increment,
                   bool halt)
{
    // Halt timer
    if (halt)
    {
        timerStop(timerBase);
    }

    if (increment)
    {
        HWREG16(timerBase + ccReg) =
        HWREG16(timerBase + OFS_TAxR) + val;
    }
    else
    {
        HWREG16(timerBase + ccReg) = val;
    }

    if (halt)
    {
        timerStartContinuous(timerBase);
    }
    return;
}

void timerStop(uint16_t timerBase)
{
    HWREG16(timerBase + OFS_TAxCTL) &= ~(MC_3);
    return;
}

void timerStartContinuous(uint16_t timerBase)
{
    HWREG16(timerBase + OFS_TAxCTL) |= MC__CONTINUOUS;
    return;
}

void timerClearCcIntFlag(uint16_t timerBase,
                         uint16_t ccCtlReg)
{
    HWREG16(timerBase + ccCtlReg) &= ~(CCIFG);
    return;
}

void timerEnableCcInt(uint16_t timerBase,
                      uint16_t ccCtlReg)
{
    HWREG16(timerBase + ccCtlReg) |= (CCIE);
    return;
}

void timerDisableCcInt(uint16_t timerBase,
                       uint16_t ccCtlReg)
{
    HWREG16(timerBase + ccCtlReg) &= ~(CCIE);
    return;
}

//// Event Flags ////
void waitEvent(uint32_t eventFlag, bool clear_flag, uint16_t lpmBits)
{
    // Save GIE status
    uint16_t gieStatus = ( __get_SR_register() & GIE);

    __disable_interrupt();
    while((isEventFlagSet(eventFlag) == false))
    {
        __bis_SR_register(lpmBits + GIE);
        __disable_interrupt();
    }

    if (clear_flag == true)
    {
        // Clear event flag
        clearEventFlag(eventFlag);
    }

    // Restore global interrupts status
    if(gieStatus == GIE)
    {
        __bis_SR_register(GIE);
    }
    return;
}

bool isEventFlagSet(uint32_t eventFlag)
{
    return (usEventFlags & eventFlag) ? true : false;
}

inline void setEventFlag(uint32_t eventFlag)
{
    usEventFlags |= eventFlag;
    return;
}

void clearEventFlag(uint32_t eventFlag)
{
    usEventFlags &= ~(eventFlag);
    return;
}

//// INTERRUPTS ////

#pragma vector = TIMER_SLOW_CC0_VECTOR
__interrupt void timerSlowCc0Int(void)
{

    // Set Event flag
    setEventFlag(TIMER_SLOW_CCR0_EVENT);

    // Clear Interrupt flag
    timerClearCcIntFlag(TIMER_SLOW_BASE, OFS_TAxCCTL0);

    if (TIMER_SLOW_CCR0_CALLBACK)
    {
        TIMER_SLOW_CCR0_CALLBACK();
    }

    LPM4_EXIT;
}

#pragma vector = TIMER_SLOW_CC1_VECTOR
__interrupt void timerSlowCc1Int(void)
{
    // Any access, read or write, of the TBIV register automatically resets the
    // highest "pending" interrupt flag.
    switch(__even_in_range(HWREG16(TIMER_SLOW_BASE + OFS_TAxIV),TAIV__TAIFG))
    {
        case  0: break;                            // No interrupt
        case  2:                                   // CCR1
            // Set Event flag
            setEventFlag(TIMER_SLOW_CCR1_EVENT);

            // Clear Interrupt flag
            timerClearCcIntFlag(TIMER_SLOW_BASE, OFS_TAxCCTL1);

            if (TIMER_SLOW_CCR1_CALLBACK)
            {
                TIMER_SLOW_CCR1_CALLBACK();
            }

            break;
        case  4:                                  // CCR2
            // Set Event flag
            setEventFlag(TIMER_SLOW_CCR2_EVENT);

            // Clear Interrupt flag
            timerClearCcIntFlag(TIMER_SLOW_BASE, OFS_TAxCCTL2);

            if (TIMER_SLOW_CCR2_CALLBACK)
            {
                TIMER_SLOW_CCR2_CALLBACK();
            }

            break;
        case  6: break;                            // CCR3 not used
        case  8: break;                            // CCR4 not used
        case 10: break;                            // CCR5 not used
        case 12: break;                            // CCR6 not used
        case 14: break;                            // overflow
        default: break;
    }

    LPM4_EXIT;
}

#pragma vector = TIMER_FAST_CC0_VECTOR
__interrupt void timerFastCc0Int(void)
{

    // Set Event flag
    setEventFlag(TIMER_FAST_CCR0_EVENT);

    // Clear Interrupt flag
    timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL0);

    if (TIMER_FAST_CCR0_CALLBACK)
    {
        TIMER_FAST_CCR0_CALLBACK();
    }

    LPM4_EXIT;
}

#pragma vector = TIMER_FAST_CC1_VECTOR
__interrupt void timerFastCc1Int(void)
{
    // Any access, read or write, of the TBIV register automatically resets the
    // highest "pending" interrupt flag.
    switch(__even_in_range(HWREG16(TIMER_FAST_BASE + OFS_TAxIV),TAIV__TAIFG))
    {
        case  0: break;                            // No interrupt
        case  2:                                   // CCR1
            // Set Event flag
            setEventFlag(TIMER_FAST_CCR1_EVENT);

            // Clear Interrupt flag
            timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL1);

            if (TIMER_FAST_CCR1_CALLBACK)
            {
                TIMER_FAST_CCR1_CALLBACK();
            }

            break;
        case  4:                                  // CCR2
            // Set Event flag
            setEventFlag(TIMER_FAST_CCR2_EVENT);

            // Clear Interrupt flag
            timerClearCcIntFlag(TIMER_FAST_BASE, OFS_TAxCCTL2);

            if (TIMER_FAST_CCR2_CALLBACK)
            {
                TIMER_FAST_CCR2_CALLBACK();
            }

            break;
        case  6: break;                            // CCR3 not used
        case  8: break;                            // CCR4 not used
        case 10: break;                            // CCR5 not used
        case 12: break;                            // CCR6 not used
        case 14: break;                            // overflow
        default: break;
    }

    LPM4_EXIT;
}

#pragma vector = HSPLL_VECTOR
__interrupt void hsPllInt(void)
{
    switch (__even_in_range(HSPLLIIDX,IIDX_1))
    {
        case IIDX_1: // HSPLL was unlocked

            // Set Event flag
            setEventFlag(HS_PLL_UNLOCK_EVENT);

            if(HS_PLL_UNLOCK_CALLBACK)
            {
                HS_PLL_UNLOCK_CALLBACK();
            }
            break;
        default: break;
    }

    LPM3_EXIT;
}

#pragma vector=UUPS_VECTOR
__interrupt void uupsInt(void)
{
    switch (__even_in_range(UUPSIIDX, IIDX_3))
    {
        case IIDX_1: // Power Up Timeout for UUPS 

            // Set Event flag
            setEventFlag(UUPS_PWR_UP_TIMEOUT_EVENT);

            if (UUPS_PWR_UP_TIMEOUT_CALLBACK)
            {
                UUPS_PWR_UP_TIMEOUT_CALLBACK();
            }
            break;

        case IIDX_3: // USS is interrupted by the debug mode

            // Set Event flag
            setEventFlag(UUPS_INTERRUPT_DBG_EVENT);

            if (UUPS_INTERRUPT_DBG_CALLBACK)
            {
                UUPS_INTERRUPT_DBG_CALLBACK();
            }
            break;
        default: break;
    }
    // Wake-up CPU on exit
    LPM3_EXIT;
}

#pragma vector=SAPH_VECTOR
__interrupt void ISR_SAPH(void)
{
    switch (__even_in_range(SAPHIIDX,IIDX_4))
    {
        case IIDX_1: // Data Error Abort

            // Set Event flag
            setEventFlag(SAPH_DATA_ERROR_EVENT);

            if (SAPH_DATA_ERROR_CALLBACK)
            {
                SAPH_DATA_ERROR_CALLBACK();
            }
            break;
        case IIDX_2:  // Time mark 4 time out

            // Set Event flag
            setEventFlag(SAPH_TIME_MF_TIMEOUT_EVENT);

            if (SAPH_TIME_MF_TIMEOUT_CALLBACK)
            {
                SAPH_TIME_MF_TIMEOUT_CALLBACK();
            }
            break;
        case IIDX_3: // Sequence acquisition done

            // Set Event flag
            setEventFlag(SAPH_SEQ_ACQ_DONE_EVENT);

            if (SAPH_SEQ_ACQ_DONE_CALLBACK)
            {
                SAPH_SEQ_ACQ_DONE_CALLBACK();
            }
            break;
        case IIDX_4: // PNGDN interrupt "pulses complete"

            // Set Event flag
            setEventFlag(SAPH_PNGND_EVENT);

            // PNGDN interrupt "pulses complete"
            // Not used by default
            if (SAPH_PNGND_CALLBACK)
            {
                SAPH_PNGND_CALLBACK();
            }
            break;
        default: break;
    }

    LPM3_EXIT;
}
