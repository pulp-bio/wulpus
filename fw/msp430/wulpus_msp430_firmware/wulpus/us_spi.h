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

#ifndef US_SPI_H_
#define US_SPI_H_

// Number of bytes in one SPI transfer
// 4 Bytes Header + 800 Bytes US frame
#define BYTES_PR_XFER_TX 804

// Defines for data ready signal
#define GPIO_PORT_DATA_READY GPIO_PORT_P4
#define GPIO_PIN_DATA_READY GPIO_PIN0

// Function to initiate DMA
// The function initializes the DMA for the SPI peripheral.
void usDmaInit(void);

// Function to initiate the SPI peripheral
// The function initiates the SPI peripheral and the necessary
// GPIOs that are used by the SPI peripheral.
void usSpiInit(void);

// Function to start SPI transaction. Called from USS_Lib_HAL.c
// The function is called from USS_Lib_HAL.c once the US
// measurement is finished. The function initiates one SPI transfer to
// transfer the whole US frame to the nRF52. The data is handled by
// the DMA.
void usStartSPI(void);

// Wait for interrupt that indicates DMA RX complete
void usWaitForSpiDmaRx(void);

// Get pointer to SPI RX buffer
uint8_t * usSpiGetRxPtr(void);

// Enable and disable DMA receive interrupt
void usSpiEnableDmaRxIsr(void);
void usSpiDisableDmaRxIsr(void);


#endif /* US_SPI_H_ */
