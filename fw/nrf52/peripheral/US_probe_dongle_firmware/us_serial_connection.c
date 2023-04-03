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

/** @file us_serial_connection.c
 *
 * @brief    Dongle firmware serial connection source code
 *
 * This file contains the source code for the serial connection
 * between the dongle and the python script.
 * 
 */

#include "boards.h"
#include "ble_nus_c.h"
#include "nrf_drv_clock.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include "us_ble.h"

#include "us_defines.h"
#include "us_serial_connection.h"

#define LED_CDC_ACM_CONN (BSP_BOARD_LED_2)
#define LED_CDC_ACM_RX   (BSP_BOARD_LED_3)

// Blue LED of the RGB LED 2
#define SERIAL_RX_LED_ID (3)

// USB DEFINES START
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1


// Maximum size of the MSP config in bytes
// According to the Config description
#define READ_SIZE               68


static char m_rx_buffer[READ_SIZE];
static char m_cdc_data_array[BLE_NUS_MAX_DATA_LEN];
static char start_string[9] = "START\n";

/** @brief CDC_ACM class instance */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);





extern bool send_us_frame_to_vcom;
extern volatile bool flag_use_buf_1;



// Buffers to store US data
extern ArrayList_type p_rx_data_1[NUMBER_OF_XFERS];
extern ArrayList_type p_rx_data_2[NUMBER_OF_XFERS];

extern bool m_usb_connected;


/**@brief Function to process virual COM port queue
 *
 * @details This function processes the queue of the
 * virtual COM port. If there is a US frame ready to send, 
 * this is signaled by setting the flag send_us_frame_to_vcom
 * to true. If the flag send_us_frame_to_vcom is set to true,
 * the US frame is sent to the python script.
 */
void us_virtual_com_port_queue_process(void)
{
    ret_code_t ret_val;
    ret_code_t ret;

    if(send_us_frame_to_vcom)
    {

        static bool started = false;
        while(!started)
        {
            app_usbd_event_queue_process();

            ret = app_usbd_cdc_acm_write(&m_app_cdc_acm, &start_string, sizeof(start_string));
            if (ret == NRF_SUCCESS)
            {
                started = true;
            }

        }
        static int  frame_counter = 0;
        // Switch between buffers each time
        if(flag_use_buf_1)
        {
            flag_use_buf_1 = false;
            while(frame_counter<NUMBER_OF_XFERS)
            {
                app_usbd_event_queue_process();
                if(frame_counter<NUMBER_OF_XFERS)
                {      
                    ret = app_usbd_cdc_acm_write(&m_app_cdc_acm, &p_rx_data_1[frame_counter], BYTES_PR_XFER);
                    if (ret == NRF_SUCCESS)
                    {
                        ++frame_counter;
                    }
                }
            }
        }
        else
        {
            flag_use_buf_1 = true;
            while(frame_counter<NUMBER_OF_XFERS)
            {
                app_usbd_event_queue_process();
                if(frame_counter<NUMBER_OF_XFERS)
                {      
                    ret = app_usbd_cdc_acm_write(&m_app_cdc_acm, &p_rx_data_2[frame_counter], BYTES_PR_XFER);
                    if (ret == NRF_SUCCESS)
                    {
                        ++frame_counter;
                    }
                }
            }        
        }
        frame_counter=0;
        send_us_frame_to_vcom = false;
        started = false;
    }
}




/** @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t */
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            /*Set up the first transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_rx_buffer,
                                                   READ_SIZE);
            //UNUSED_VARIABLE(ret);
            //ret = app_timer_stop(m_blink_cdc);
            //APP_ERROR_CHECK(ret);
            break;
        }

        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            //if (m_usb_connected)
            //{
            //    ret_code_t ret = app_timer_start(m_blink_cdc,
            //                                     APP_TIMER_TICKS(LED_BLINK_INTERVAL),
            //                                     (void *) LED_CDC_ACM_CONN);
            //    APP_ERROR_CHECK(ret);
            //}
            break;

        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            break;

        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            ret_code_t ret;
            //NRF_LOG_INFO("Bytes waiting: %d", app_usbd_cdc_acm_bytes_stored(p_cdc_acm));

            /*Get amount of data transfered*/
            size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
            //NRF_LOG_INFO("RX: size: %lu char: %c", size, m_rx_buffer[0]);

            //Set up the next first transfer
            ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                        m_rx_buffer,
                                        READ_SIZE);

            uint16_t length = READ_SIZE;

            // Invert LED if data transmission is sucessfull
            if(send_ble_packet((uint8_t *) m_rx_buffer, READ_SIZE) == NRF_SUCCESS)
            {
                // Red LED of RGB
                bsp_board_led_invert(SERIAL_RX_LED_ID);
            }

            break;

            /*
            index++;

            do
            {
                if ((m_cdc_data_array[index - 1] == '\n') ||
                    (m_cdc_data_array[index - 1] == '\r') ||
                    (index >= (m_ble_nus_max_data_len)))
                {
                    if (index > 1)
                    {
                        //bsp_board_led_invert(LED_CDC_ACM_RX);

                        do
                        {
                            uint16_t length = (uint16_t)index;
                            if (length + sizeof(ENDLINE_STRING) < BLE_NUS_MAX_DATA_LEN)
                            {
                                memcpy(m_cdc_data_array + length, ENDLINE_STRING, sizeof(ENDLINE_STRING));
                                length += sizeof(ENDLINE_STRING);
                            }

                            //ret = ble_nus_c_string_send(&m_ble_nus_c, (uint8_t *) m_cdc_data_array, length);
                            //ret = ble_nus_data_send(&m_nus,
                             //                       (uint8_t *) m_cdc_data_array,
                            //                        &length,
                            //                        m_conn_handle);

                            if (ret == NRF_ERROR_NOT_FOUND)
                            {
                                break;
                            }

                            if (ret == NRF_ERROR_RESOURCES)
                            {
                                break;
                            }

                            if ((ret != NRF_ERROR_INVALID_STATE) && (ret != NRF_ERROR_BUSY))
                            {
                                APP_ERROR_CHECK(ret);
                            }
                        }
                        while (ret == NRF_ERROR_BUSY);
                    }
                    //if(strcmp(m_cdc_data_array, "Hellofr\r\n") == 0)
                    ////if(m_cdc_data_array[0] == 'X')
                    //{
                    //    // Strings are equal
                    //    for (int i = 0; i < LEDS_NUMBER; i++)
                    //    {
                    //        bsp_board_led_invert(i);
                    //        //nrf_delay_ms(500);
                    //    }
                    //

            //memcpy(m_cdc_data_array, 0, 10);

                    index = 0;
                }

                
                

                //Get amount of data transferred
                size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);

                // Fetch data until internal buffer is empty 
                ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                            &m_cdc_data_array[index],
                                            1);
                if (ret == NRF_SUCCESS)
                {
                    index++;
                }
            }
            while (ret == NRF_SUCCESS);

            // Send data to peripheral
            ret_code_t ret_val;
            do
            {
                ret_val = ble_nus_c_string_send(&m_ble_nus_c, m_cdc_data_array, 7);
                if ((ret_val != NRF_SUCCESS) && (ret_val != NRF_ERROR_BUSY))
                {
                    APP_ERROR_CHECK(ret_val);
                }
            } while (ret_val == NRF_ERROR_BUSY);
            index = 0;

            for (int i = 0; i < LEDS_NUMBER; i++)
            {
                bsp_board_led_invert(i);
                //nrf_delay_ms(500);
            }
            */

            break;
        }
        default:
            break;
    }
}


static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:
            break;

        case APP_USBD_EVT_DRV_RESUME:
            break;

        case APP_USBD_EVT_STARTED:
            break;

        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;

        case APP_USBD_EVT_POWER_DETECTED:
            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;

        case APP_USBD_EVT_POWER_REMOVED:
        {
            //ret_code_t err_code = app_timer_stop(m_blink_cdc);
            //APP_ERROR_CHECK(err_code);
            m_usb_connected = false;
            app_usbd_stop();
        }
            break;

        case APP_USBD_EVT_POWER_READY:
        {
            //ret_code_t err_code = app_timer_start(m_blink_cdc,
            //                                      APP_TIMER_TICKS(LED_BLINK_INTERVAL),
            //                                      (void *) LED_CDC_ACM_CONN);
            //APP_ERROR_CHECK(err_code);
            m_usb_connected = true;
            app_usbd_start();
        }
            break;

        default:
            break;
    }
}


 /**@brief Function to initialize virtual COM port
 *
 * @details This function processes the virtual COM port. It is used to 
 * communicate between the python script and the receiver dongle.
 */
void us_serial_init(void)
{
    ret_code_t ret;

    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };

    
    app_usbd_serial_num_generate();

    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);


    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);

    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);
}

