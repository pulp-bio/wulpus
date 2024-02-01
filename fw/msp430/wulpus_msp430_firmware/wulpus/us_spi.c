/*
 * Copyright (C) 2023 ETH Zurich. All rights reserved.
 *
 * Authors: Sergei Vostrikov, ETH Zurich
 *          Sebastian Frey, ETH Zurich
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

#include "driverlib.h"
#include "us_spi.h"

// Buffers for US data
uint8_t s_rx_buf_1[BYTES_PR_XFER_TX] = {0};

static uint8_t dmaRxIsrFlag = 0;


// DMA interrupt service routine
#pragma vector=DMA_VECTOR
__interrupt void ISR_DMA (void)
{
    // Exit LPM0 state
    __bic_SR_register_on_exit(LPM0_bits);
    DMA_clearInterrupt(DMA_CHANNEL_1);
    dmaRxIsrFlag = 1;
}

// Function to start SPI transaction. Called from USS_Lib_HAL.c
// The function is called from USS_Lib_HAL.c once the US
// measurement is finished. The function initiates one SPI transfer to
// transfer the whole US frame to the nRF52. The data is handled by
// the DMA.
void usStartSPI(void)
{
    // Double buffering not yet implemented

    // Fill in first byte to SPI TX buffer to be ready when the transaction starts
    uint8_t first_byte;
    memcpy(&first_byte, (uint16_t *) 0x4000, 1);
    UCA1TXBUF = first_byte;

    // Set Source address of DMA channel 0 to US data, start at second byte
    DMA_disableTransfers(DMA_CHANNEL_0);
    DMA_setSrcAddress(DMA_CHANNEL_0,
                      (uint32_t) 0x4001,
                      DMA_DIRECTION_INCREMENT);
    DMA_enableTransfers(DMA_CHANNEL_0);

    // Set Destination address of DMA channel 1 to s_rx_buf_1
    DMA_disableTransfers(DMA_CHANNEL_1);
    DMA_setDstAddress(DMA_CHANNEL_1,
                      (uint32_t) s_rx_buf_1,
                      DMA_DIRECTION_INCREMENT);
    DMA_enableTransfers(DMA_CHANNEL_1);

    return;
}

// Wait for interrupt that indicates DMA RX complete
void usWaitForSpiDmaRx(void)
{
    // Save global interrupt status
    uint16_t gieStatus = ( __get_SR_register() & GIE);

    // Generate "Data ready" signal for SPI master which will initiate the SPI transfer
    GPIO_setOutputHighOnPin(GPIO_PORT_DATA_READY, GPIO_PIN_DATA_READY);

    // Check if dmaRXIsrFlag is raised
    while(!dmaRxIsrFlag)
    {
        // Enter LPM0 with global interrupts enabled
        __bis_SR_register(LPM0_bits + GIE);
        __disable_interrupt();
    }

    // Disable DMA SPI interrupt
    usSpiDisableDmaRxIsr();

    // Clear flag
    dmaRxIsrFlag = 0;

    // Clear "Data ready" signal
    GPIO_setOutputLowOnPin(GPIO_PORT_DATA_READY, GPIO_PIN_DATA_READY);

    // Restore global interrupts status
    if(GIE == gieStatus)
    {
        __bis_SR_register(GIE);
    }
}

uint8_t * usSpiGetRxPtr(void)
{
    return (uint8_t *) s_rx_buf_1;
}

void usSpiEnableDmaRxIsr(void)
{
    DMA_enableInterrupt(DMA_CHANNEL_1);
    return;
}


void usSpiDisableDmaRxIsr(void)
{
    DMA_disableInterrupt(DMA_CHANNEL_1);
    return;
}


// Function to initiate DMA
// @details The function initializes the DMA for the SPI peripheral.
void usDmaInit(void)
{
    //Initialize and Setup DMA Channel 0 for TX buffers
    // Configure DMA channel 0
    // Configure channel for repeated single transfers
    // Configure SPI TX interrupt flag as DMA trigger
    // Transfer Byte-to-Byte
    // Trigger upon Rising Edge of Trigger Source Signal
    DMA_initParam param_ch_0 = {0};
    param_ch_0.channelSelect = DMA_CHANNEL_0;
    param_ch_0.transferModeSelect = DMA_TRANSFER_REPEATED_SINGLE;
    param_ch_0.transferSize = sizeof(s_rx_buf_1)-1; // Minus 1 because the first byte is transfered manually
    param_ch_0.triggerSourceSelect = DMA_TRIGGERSOURCE_17; //UCA1TXIFG
    param_ch_0.transferUnitSelect = DMA_SIZE_SRCBYTE_DSTBYTE;
    param_ch_0.triggerTypeSelect = DMA_TRIGGER_RISINGEDGE;
    DMA_init(&param_ch_0);

    // Configure DMA channel 0
    // Use SPI TX register as destination
    // Don't increment address after transfer
    DMA_setDstAddress(DMA_CHANNEL_0,
                      (uint32_t) &UCA1TXBUF,
                      DMA_DIRECTION_UNCHANGED);

    // Initialize and Setup DMA Channel 1 for RX buffers
    // Configure DMA channel 1
    // Configure channel for repeated single transfers
    // Configure SPI RX interrupt flag as DMA trigger
    // Transfer Byte-to-Byte
    // Trigger upon Rising Edge of Trigger Source Signal
    DMA_initParam param_ch_1 = {0};
    param_ch_1.channelSelect = DMA_CHANNEL_1;
    param_ch_1.transferModeSelect = DMA_TRANSFER_REPEATED_SINGLE;
    param_ch_1.transferSize = sizeof(s_rx_buf_1);
    param_ch_1.triggerSourceSelect = DMA_TRIGGERSOURCE_16; //UCA1RXIFG
    param_ch_1.transferUnitSelect = DMA_SIZE_SRCBYTE_DSTBYTE;
    param_ch_1.triggerTypeSelect = DMA_TRIGGER_RISINGEDGE;
    DMA_init(&param_ch_1);

    // Configure DMA channel 1
    // Use SPI RX register as source
    // Don't increment address after transfer
    DMA_setSrcAddress(DMA_CHANNEL_1,
                      (uint32_t) &UCA1RXBUF,
                      DMA_DIRECTION_UNCHANGED);
}



// Function to initiate the SPI peripheral
// The function initiates the SPI peripheral and the necessary
// GPIOs that are used by the SPI peripheral.
void usSpiInit(void)
{
    // Configure SPI pins
    // Select Port 1
    // Set Pin 2 to input peripheral function, SPI MOSI (SIMO)
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P1,
        GPIO_PIN2,
        GPIO_PRIMARY_MODULE_FUNCTION);

    // Select Port 1
    // Set Pin 3 to output peripheral function, SPI MISO (SOMI)
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN3,
        GPIO_PRIMARY_MODULE_FUNCTION);

    // Select Port 2
    // Set Pin 0 to input peripheral function, SPI CLK
    // Set Pin 1 to input peripheral module Function, SPI CS
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P2,
        GPIO_PIN0 + GPIO_PIN1,
        GPIO_PRIMARY_MODULE_FUNCTION);

    // Select Port 4
    // Set Pin 0 to output SPI "Data ready signal"
    GPIO_setAsOutputPin(GPIO_PORT_DATA_READY, GPIO_PIN_DATA_READY);

    // Initialize SPI slave: MSB first, inactive high clock polarity and 4 wire SPI
    EUSCI_A_SPI_initSlaveParam param = {0};
    param.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_A_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    param.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    param.spiMode = EUSCI_A_SPI_4PIN_UCxSTE_ACTIVE_LOW;
    EUSCI_A_SPI_initSlave(EUSCI_A1_BASE, &param);

    // Use Slave Select
    EUSCI_A_SPI_select4PinFunctionality(EUSCI_A1_BASE, EUSCI_A_SPI_ENABLE_SIGNAL_FOR_4WIRE_SLAVE);

    //Enable SPI Module
    EUSCI_A_SPI_enable(EUSCI_A1_BASE);
}

