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

#ifndef USLIB_USLIB_TIMERS_ISRS_H_
#define USLIB_USLIB_TIMERS_ISRS_H_


#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "timer_a.h"

//// Ultrasound Subsystem ////


//// TIMERS  ////

// Timer clocked from ACLK(LXFT)
#define TIMER_SLOW_BASE          (TIMER_A1_BASE)
#define TIMER_SLOW_CC0_VECTOR    (TIMER1_A0_VECTOR)
#define TIMER_SLOW_CC1_VECTOR    (TIMER1_A1_VECTOR)

// Timer clocked from SMCLK
#define TIMER_FAST_BASE          (TIMER_A0_BASE)
#define TIMER_FAST_CC0_VECTOR    (TIMER0_A0_VECTOR)
#define TIMER_FAST_CC1_VECTOR    (TIMER0_A1_VECTOR)

// Callbacks
extern void (*TIMER_SLOW_CCR0_CALLBACK)(void);
extern void (*TIMER_SLOW_CCR1_CALLBACK)(void);
extern void (*TIMER_SLOW_CCR2_CALLBACK)(void);

extern void (*TIMER_FAST_CCR0_CALLBACK)(void);
extern void (*TIMER_FAST_CCR1_CALLBACK)(void);
extern void (*TIMER_FAST_CCR2_CALLBACK)(void);

extern void (*HS_PLL_UNLOCK_CALLBACK)       (void);
extern void (*UUPS_PWR_UP_TIMEOUT_CALLBACK) (void);
extern void (*UUPS_INTERRUPT_DBG_CALLBACK)  (void);
extern void (*SAPH_DATA_ERROR_CALLBACK)     (void);
extern void (*SAPH_TIME_MF_TIMEOUT_CALLBACK)(void);
extern void (*SAPH_SEQ_ACQ_DONE_CALLBACK)   (void);
extern void (*SAPH_PNGND_CALLBACK)          (void);

//// For event flags ////
#define ALL_EVENTS_MASK              (0xFFFF)

#define TIMER_SLOW_CCR0_EVENT        (1<<0)
#define TIMER_SLOW_CCR1_EVENT        (1<<1)
#define TIMER_SLOW_CCR2_EVENT        (1<<2)

#define TIMER_FAST_CCR0_EVENT        (1<<6)
#define TIMER_FAST_CCR1_EVENT        (1<<7)
#define TIMER_FAST_CCR2_EVENT        (1<<8)

#define HS_PLL_UNLOCK_EVENT          (1<<10)
#define UUPS_PWR_UP_TIMEOUT_EVENT    (1<<11)
#define UUPS_INTERRUPT_DBG_EVENT     (1<<12)
#define SAPH_DATA_ERROR_EVENT        (1<<13)
#define SAPH_TIME_MF_TIMEOUT_EVENT   (1<<14)
#define SAPH_SEQ_ACQ_DONE_EVENT      ((uint32_t)(1)<<15)
#define SAPH_PNGND_EVENT             ((uint32_t)(1)<<16)


//// Timer-based High Level Routines ////

void timerSlowInit(void);
void timerSlowStop(void);
// Blocking function
void timerSlowDelay(uint16_t delay, uint16_t lpmBits);


void timerFastInit(void);
void timerFastStop(void);

//// Timer-based Low Level Routines ////
// Set capture compare register
void timerSetCcReg(uint16_t timerBase,
                   uint16_t val,
                   uint16_t ccReg,
                   bool increment,
                   bool halt);

void timerStop(uint16_t timerBase);
void timerStartContinuous(uint16_t timerBase);

void timerClearCcIntFlag(uint16_t timerBase,
                         uint16_t ccCtlReg);

void timerEnableCcInt(uint16_t timerBase,
                      uint16_t ccCtlReg);

void timerDisableCcInt(uint16_t timerBase,
                       uint16_t ccCtlReg);


////  Event flags routines  ///
void waitEvent(uint32_t eventFlag,
               bool clear_flag,
               uint16_t lpmBits);

bool isEventFlagSet(uint32_t eventFlag);
inline void setEventFlag(uint32_t eventFlag);
void clearEventFlag(uint32_t eventFlag);


#endif /* USLIB_USLIB_TIMERS_ISRS_H_ */
