/*
 * Copyright (C) 2023 ETH Zurich. All rights reserved.
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

#ifndef WULPUS_SYS_H_
#define WULPUS_SYS_H_

#include "driverlib.h"

#include "us_spi.h"
#include "us_hv_mux.h"
#include "uslib.h"

// Defines for LED on Acquisition PCB
#define GPIO_PORT_LED_MSP430  GPIO_PORT_P1
#define GPIO_PIN_LED_MSP430   BIT5

// Defines for BLE connection ready signal
#define GPIO_PORT_BLE_READY   GPIO_PORT_P4
#define GPIO_PIN_BLE_READY    GPIO_PIN4

// MACROS to help with memory access

#define READ_uint8(address)   (((uint8_t *)address)[0])

#define READ_uint16(address)  (((uint8_t *)address)[0] | (((uint16_t)((uint8_t *)address)[1])<<8))

#define READ_uint32(address)  (((uint8_t *)address)[0]|\
                              (((uint32_t)((uint8_t *)address)[1])<<8)|\
                              (((uint32_t)((uint8_t *)address)[2])<<16)|\
                              (((uint32_t)((uint8_t *)address)[3])<<24))


// Commands for indicating the configuration package or restart command
#define START_BYTE_CONF_PACK    (0xFA)
#define START_BYTE_RESTART      (0xFB)

void getDefaultUsConfig(msp_config_t * msp_config);

// Extract Uss config from the spi RX buffer
// Return 1 if config is valid
bool extractUsConfig(uint8_t * spi_rx, msp_config_t * msp_config);

//// Extra functions ////

// Check the first byte if restart should be performed
bool isRestartCondition(uint8_t * spi_rx);
// Check the first byte if a new config is requested
bool isNewConfigCondition(uint8_t * spi_rx);

// Initiate MSP430-controlled power switches
void initAllPowerSwitches(void);
// Init other GPIOs
void initOtherGpios(void);

// Function to check if BLE host is ready
bool isBleReady(void);

// Enable Rx Operational Amplifier power supply
void enableOpAmpSupply(void);
// Disable Rx Operational Amplifier power supply
void disableOpAmpSupply(void);

// Enable Rx Operational Amplifier
void enableOpAmp(void);
// Disable Rx Operational Amplifier
void disableOpAmp(void);

// Enable HV PCB power supply
void enableHvPcbSupply(void);
// Disable HV PCB power supply
void disableHvPcbSupply(void);

// Enable DC-DC converters on HV PCB
void enableHvPcbDcDc(void);
// Disable DC-DC converters on HV PCB
void disableHvPcbDcDc(void);
// Disable only HV DC-DC converter on HV PCB
void disableHvDcDc(void);

#endif /* WULPUS_SYS_H_ */
