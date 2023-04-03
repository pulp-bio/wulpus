/**
 * Copyright (c) 2017 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

 /**
 *
 * Project: Wearable Ultra Low-Power UltraSound probe (WULPUS)
 * Modified by:  Sebastian Frey (ETH Zurich), 
 *               Sergei Vostrikov (ETH Zurich)
 * 
 */

/** @file main.c
 *
 * @brief    Dongle firmware main file
 *
 * This file contains the source code for the main file of the firmware
 * that runs on the receiver dongle. The purpose of this firmware is to
 * receive US frames from the Acquisition PCB through BLE and to send that
 * data further to a python script through a serial connection.
 * 
 */

#include "nrf_ble_gatt.h"
#include "nrf_pwr_mgmt.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "ble_nus_c.h"
#include "bsp_btn_ble.h"
#include "nrf_drv_clock.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include "us_defines.h"
#include "us_serial_connection.h"
#include "us_ble.h"



#define LED_BLE_NUS_CONN (BSP_BOARD_LED_0)
#define LED_BLE_NUS_RX   (BSP_BOARD_LED_1)
#define LED_CDC_ACM_CONN (BSP_BOARD_LED_2)
#define LED_CDC_ACM_RX   (BSP_BOARD_LED_3)

#define LED_BLINK_INTERVAL 800
 

APP_TIMER_DEF(m_blink_ble);
APP_TIMER_DEF(m_blink_cdc);


#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump. Can be used to identify stack location on stack unwind. */

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */

// Buffers to store US data
ArrayList_type p_rx_data_1[NUMBER_OF_XFERS] = {0};
ArrayList_type p_rx_data_2[NUMBER_OF_XFERS] = {0};

// Flag to implement double buffering
volatile bool flag_use_buf_1 = true;

// Flag to indicate that an US frame is ready to be sent to python
bool send_us_frame_to_vcom = false;

//static bool m_usb_connected = false;
bool m_usb_connected = false;


/**
 * @brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of an assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**
 * @brief App timer handler for blinking the LEDs.
 *
 * @param p_context LED to blink.
 */
void blink_handler(void * p_context)
{
    //bsp_board_led_invert((uint32_t) p_context);
}


/**@snippet [Handling events from the ble_nus_c module] */

/**
 * @brief Function for handling shutdown events.
 *
 * @param[in]   event       Shutdown type.
 */
static bool shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            // Prepare wakeup buttons.
            err_code = bsp_btn_ble_sleep_mode_prepare();
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }

    return true;
}

NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler, APP_SHUTDOWN_HANDLER_PRIORITY);




/** @brief Function for initializing the timer module. */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(&m_blink_ble, APP_TIMER_MODE_REPEATED, blink_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(&m_blink_cdc, APP_TIMER_MODE_REPEATED, blink_handler);
    APP_ERROR_CHECK(err_code);
}



/**
 * @brief Function for putting the chip into sleep mode.
 *
 * @note This function does not return.
 */
static void sleep_mode_enter(void)
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



/**@brief Function for handling events from the BSP module.
 *
 * @param[in] event  Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
            break;

        case BSP_EVENT_DISCONNECT:
            //err_code = sd_ble_gap_disconnect(m_ble_nus_c.conn_handle,
            //                                 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            err_code = us_sd_ble_gap_disconnect(BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}


/**@brief Function for initializing buttons and leds. */
static void buttons_leds_init(void)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);
}



/** @brief Function for placing the application in low power state while waiting for events. */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    //UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
    power_manage();
}


/** @brief Application main function. */
int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);
    ret_code_t ret;

    // Initialize.
    timers_init();


    us_serial_init();
    us_ble_init();

    ret = app_usbd_power_events_enable();
    APP_ERROR_CHECK(ret);
    
    bool m_send_flag;
    m_send_flag = true;

    // Enter main loop.
    while(true)
    {
        while (app_usbd_event_queue_process())
        {
            /* Nothing to do */
        }
        us_virtual_com_port_queue_process();
    }

}

/**
 * @}
 */
