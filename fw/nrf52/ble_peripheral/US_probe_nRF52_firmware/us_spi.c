/*
 * Copyright (C) 2023 ETH Zurich. All rights reserved.
 *
 * Authors: Sebastian Frey, ETH Zurich
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

/** @file us_spi.c
 *
 * @brief    nRF52 Acquisition PCB firmware SPI source code
 * 
 * This file contains the source code for the SPI connection
 * between the MSP430 and the nRF52. One US frame is transfered
 * in four SPI transactions.
 *
*/


#include "nrf_drv_gpiote.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_drv_timer.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"

#include "us_spi.h"
#include "us_defines.h"

#include "us_defines.h"
#include "us_spi.h"


// Define SPI pins
#define PIN_SPI_SS 7
#define PIN_SPI_MISO 9
#define PIN_SPI_MOSI 10
#define PIN_SPI_SCK 8

extern ArrayList_type m_rx_buf_1[NUMBER_OF_XFERS];
extern ArrayList_type m_rx_buf_2[NUMBER_OF_XFERS];
extern ArrayList_type m_tx_buf_1[NUMBER_OF_XFERS];

// Flag to implement double buffering
extern bool flag_use_buf_1;

// Flag to know if BLE is connected (-> and therefore US measurements can start)
extern volatile bool ble_connected;

// Function to send one BLE packet
void send_packet(uint8_t* start_address, uint16_t length);

//Time(in microseconds) between consecutive compare events. (257 us is absolute min for 255Bytes at 8Mbps)
uint32_t time_us = 300; 
uint32_t time_ticks;

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0);

// TIMER0 Used to start SPI transfers at regular intervals
const nrf_drv_timer_t timer_timer = NRF_DRV_TIMER_INSTANCE(3);
// TIMER1 Used in Counter mode to count number of completed transfers and stop the SPI after NUMBER_OF_XFERS number of transfers
const nrf_drv_timer_t timer_counter = NRF_DRV_TIMER_INSTANCE(4);

// Task and event addresses for SPI transactions
uint32_t start_spi_task_addr;
uint32_t spi_end_evt_addr;
uint32_t timer0_timeout_cc0_evt_addr;
uint32_t counter1_count_task_addr;
uint32_t counter1_cc0_evt_addr;



void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    // Never called if NRF_DRV_SPI_FLAG_NO_XFER_EVT_HANDLER flag is used
}


/**@brief Function to initialize SPI
 *
 * @details This function initializes The SPI peripheral. It will also set up 
 * the SPI transfer with EasyDMA. The prepared SPI transfer can later be 
 * initiated with PPI.
 */
void spi_init()
{
    uint32_t err_code;
    nrf_drv_spi_config_t config = NRF_DRV_SPI_DEFAULT_CONFIG;
    config.ss_pin   = PIN_SPI_SS;
    config.miso_pin = PIN_SPI_MISO;
    config.mosi_pin = PIN_SPI_MOSI;
    config.sck_pin  = PIN_SPI_SCK;
    config.frequency = NRF_DRV_SPI_FREQ_8M;
    config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
    config.mode = NRF_DRV_SPI_MODE_1;
    err_code = nrf_drv_spi_init(&spi, &config, spi_event_handler, NULL);
    APP_ERROR_CHECK(err_code);
    
    // Setting up an SPI transfer using EasyDMA
    nrf_drv_spi_xfer_desc_t xfer = NRF_DRV_SPI_XFER_TRX((uint8_t *)m_tx_buf_1, BYTES_PR_XFER_TX, (uint8_t *)m_rx_buf_1, BYTES_PR_XFER_RX);
    
    uint32_t flags = NRF_DRV_SPI_FLAG_HOLD_XFER           |
                     NRF_DRV_SPI_FLAG_RX_POSTINC          |
                     NRF_DRV_SPI_FLAG_REPEATED_XFER       |
                     NRF_DRV_SPI_FLAG_NO_XFER_EVT_HANDLER;
    
    err_code = nrf_drv_spi_xfer(&spi, &xfer, flags);
    APP_ERROR_CHECK(err_code);
    
    // Get SPI START task address and END event address
    start_spi_task_addr = nrf_drv_spi_start_task_get(&spi);
    spi_end_evt_addr = nrf_drv_spi_end_event_get(&spi);
}

// @brief Handler for timer events.
void timer_timeout_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    // Handler must be declared and passed to nrf_drv_timer_init(), 
    // but is not used if nrf_drv_timer_extended_compare() is called with enable_int flag = false
}



/**@brief Called when the SPI transfers are done. Here, the data is sent through BLE to the dongle.
 *
 * @details This timer event handler is called when all four SPI transfers are done. It will then stop
 * timer_timer and timer_counter to stop the SPI transfers. Then, this function sends the received 
 * US frame to the dongle
 *
 */
void counter_cc0_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    // Stop timers and hence, stop SPI transfers.
    nrf_drv_timer_disable(&timer_timer);
    nrf_drv_timer_disable(&timer_counter);


    if(ble_connected)
    {
        // Relay the received SPI data to the BLE dongle
        uint32_t err_code;
        uint16_t length;

        length = BYTES_PR_XFER_RX;

        //nrf_drv_gpiote_out_set(LED_NRF52);
        if(flag_use_buf_1)
        {
            // flag_use_buf_1 is true -> data in m_rx_buf_2 should be sent to BLE dongle
            send_packet(&m_rx_buf_2[0].buffer[0], length+1);
            send_packet(&m_rx_buf_2[1].buffer[0], length);
            send_packet(&m_rx_buf_2[2].buffer[0], length);
            send_packet(&m_rx_buf_2[3].buffer[0], length);
        }
        else
        {
            // flag_use_buf_1 is false -> data in m_rx_buf_1 should be sent to BLE dongle
            send_packet(&m_rx_buf_1[0].buffer[0], length+1);
            send_packet(&m_rx_buf_1[1].buffer[0], length);
            send_packet(&m_rx_buf_1[2].buffer[0], length);
            send_packet(&m_rx_buf_1[3].buffer[0], length);
        }
        //nrf_drv_gpiote_out_clear(LED_NRF52);
        //nrf_drv_gpiote_out_toggle(LED_NRF52);
    }
}

/**@brief Function to initialize timer and counter for SPI transfers
 *
 * @details The timer and counter are initialized and connected through PPI.
 * The counter is used to count the SPI transfers (four per US frame) and the 
 * timer is used to start new SPI transfers in the set interval.
 *
 */
void timer_init()
{
    uint32_t err_code = NRF_SUCCESS;
    
    // Init timer to start SPI transfers on CC0 event
    nrf_drv_timer_config_t timer_timout_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    err_code = nrf_drv_timer_init(&timer_timer, &timer_timout_cfg, timer_timeout_event_handler);
    APP_ERROR_CHECK(err_code);

    
    time_ticks = nrf_drv_timer_us_to_ticks(&timer_timer, time_us);
    nrf_drv_timer_extended_compare(&timer_timer, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
    
    timer0_timeout_cc0_evt_addr = nrf_drv_timer_event_address_get(&timer_timer, NRF_TIMER_EVENT_COMPARE0);
    
    
    // Init Counter to count SPI transfers
    nrf_drv_timer_config_t timer_counter_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_counter_cfg.mode = NRF_TIMER_MODE_COUNTER;
    err_code = nrf_drv_timer_init(&timer_counter, &timer_counter_cfg, counter_cc0_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(&timer_counter, NRF_TIMER_CC_CHANNEL0, NUMBER_OF_XFERS, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    
    counter1_count_task_addr = nrf_drv_timer_task_address_get(&timer_counter, NRF_TIMER_TASK_COUNT);
}

/**@brief Initialize the PPI channels
 *
 * @details This functions connects the timer and counter to the SPI peripheral.
 *
 */
void ppi_init()
{
    uint32_t err_code;
    nrf_ppi_channel_t ppi_ch_timer_cc0_start_spi;
    nrf_ppi_channel_t ppi_ch_spi_end_counter1_count;
    

     //* Init timer0 timout to start SPI
    err_code = nrf_drv_ppi_channel_alloc(&ppi_ch_timer_cc0_start_spi);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_ppi_channel_assign(ppi_ch_timer_cc0_start_spi, timer0_timeout_cc0_evt_addr, start_spi_task_addr);
    APP_ERROR_CHECK(err_code);
    

     //* Init SPI END event causes COUNTER 1 to increment
    err_code = nrf_drv_ppi_channel_alloc(&ppi_ch_spi_end_counter1_count);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_ppi_channel_assign(ppi_ch_spi_end_counter1_count, spi_end_evt_addr, counter1_count_task_addr);
    APP_ERROR_CHECK(err_code);
    
    // Enable both configured PPI channels
    err_code = nrf_drv_ppi_channel_enable(ppi_ch_timer_cc0_start_spi);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_ppi_channel_enable(ppi_ch_spi_end_counter1_count);
    APP_ERROR_CHECK(err_code);
}


/**@brief Initialize SPI with the timer, counter and the PPI
 *
 *
 */
void us_spi_init(void)
{
    spi_init();
    timer_init();
    ppi_init();

}



