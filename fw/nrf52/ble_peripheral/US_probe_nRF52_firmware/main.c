/*
 * Copyright (C) 2023 ETH Zurich. All rights reserved.
 *
 * Authors: Sebastian Frey, ETH Zurich
 *          Sergei Vostrikov, ETH Zurich
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


/** @file main.c
 *
 * @brief    nRF52 Acquisition PCB firmware main file
 *
 * This file contains the source code for the main file of the firmware
 * that runs on the nRF52 MCU on the Acquisition PCB. The purpose of this 
 * firmware is to receive US frames from the MSP430 through SPI and to 
 * send that data further to the receiver dongle through a BLE connection.
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
#include "us_ble.h"
#include "us_defines.h"


// Define GPIOs
#define LED_NRF52 23
#define PIN_DATA_READY 29
#define PIN_BLE_CONN_READY 25

// Buffers to store US data
ArrayList_type m_rx_buf_1[NUMBER_OF_XFERS] = {0};
ArrayList_type m_rx_buf_2[NUMBER_OF_XFERS] = {0};

// Buffer to store commands from python
ArrayList_type m_tx_buf_1[NUMBER_OF_XFERS] = {0};

// Flag to implement double buffering
volatile bool flag_use_buf_1 = true;

// Timer and counter to control SPI transactions
extern nrf_drv_timer_t timer_timer;
extern nrf_drv_timer_t timer_counter;

// To check if SPI data can be relayed to BLE dongle
volatile bool ble_connected = false;

// A flag to signal that MSP config is received
volatile bool msp_conf_received = false;

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    nrf_pwr_mgmt_run();
}



void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    // Check if the interrupt is from the data ready pin. If yes, start the SPI transactions
    if (pin == PIN_DATA_READY)
    {
        // Switch between buffers each time
        if(flag_use_buf_1)
        {
            flag_use_buf_1 = false;
            NRF_SPIM0->RXD.PTR = (uint32_t)&m_rx_buf_1;
        }
        else
        {
            flag_use_buf_1 = true;
            NRF_SPIM0->RXD.PTR = (uint32_t)&m_rx_buf_2;
        }
        // Enable timer and counter to start the four SPI transactions
        nrf_drv_timer_enable(&timer_timer);
        nrf_drv_timer_enable(&timer_counter);
    }
}
/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT pin for output,
 * and configures GPIOTE to give an interrupt on pin change.
 */
static void gpio_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    // Configure NRF52 LED on Acquisition PCB
    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    err_code = nrf_drv_gpiote_out_init(LED_NRF52, &out_config);
    APP_ERROR_CHECK(err_code);

    // BLE connection ready GPIO out
    nrf_drv_gpiote_out_config_t out_config_ble_ready = GPIOTE_CONFIG_OUT_SIMPLE(false);
    err_code = nrf_drv_gpiote_out_init(PIN_BLE_CONN_READY, &out_config_ble_ready);
    APP_ERROR_CHECK(err_code);

    // SPI data ready GPIO in
    nrf_drv_gpiote_in_config_t in_config_data_ready = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config_data_ready.pull = NRF_GPIO_PIN_NOPULL;
    err_code = nrf_drv_gpiote_in_init(PIN_DATA_READY, &in_config_data_ready, in_pin_handler);
    APP_ERROR_CHECK(err_code);
    
    // Enable interrupt for data ready
    nrf_drv_gpiote_in_event_enable(PIN_DATA_READY, true);

    

}

/**@brief Application main function.
 */
int main(void)
{

    // Initialize
    timers_init();
    power_management_init();
    us_ble_init();
    gpio_init();
    us_spi_init();


    // Tell MSP430 that the BLE connection is not ready yet
    nrf_drv_gpiote_out_clear(PIN_BLE_CONN_READY);

    // Start BLE advertising
    advertising_start();

    while(ble_connected==false)
    {
        // Wait for BLE to be connected
    }

    // Wait some more time until the BLE connection parameters are updated,
    // and until the switch to 2 Mbps (LE 2M PHY) is done
    nrf_delay_ms(2000);

    while(msp_conf_received==false)
    {
        // Wait for MSP430 config to be received from host PC
    }

    // Now the BLE connection is ready to send US data
    nrf_drv_gpiote_out_set(PIN_BLE_CONN_READY);

    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
    }
}
