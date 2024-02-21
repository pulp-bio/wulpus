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

#ifndef US_HV_MUX_H_
#define US_HV_MUX_H_

#include "driverlib.h"

// Delay in MCLK cycles
#define DELAY_CYCLES    2


#define LE_PIN_PORT     GPIO_PORT_P5
#define LE_PIN          GPIO_PIN7


void hvMuxInit(void);
void hvMuxConfTx(uint16_t tx_config);
void hvMuxConfRx(uint16_t rx_config);

// Latch outputs
// Transition from High to Low  transfers the contents of the
// shift registers into the latches and turn on switches
void hvMuxLatchOutput(void);

#endif /* US_HV_MUX_H_ */
