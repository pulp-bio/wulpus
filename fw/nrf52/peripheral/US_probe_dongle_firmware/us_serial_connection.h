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

/** @file us_serial_connection.h
 *
 * @brief    Dongle firmware serial connection (virual COM) includes
 * 
 */


#ifndef US_SERIAL_CONNECTION_H
#define US_SERIAL_CONNECTION_H

    
    /**@brief Function to process virual COM port queue
     *
     * @details This function processes the queue of the
     * virtual COM port. If there is a US frame ready to send, 
     * this is signaled by setting the flag send_us_frame_to_vcom
     * to true. If the flag send_us_frame_to_vcom is set to true,
     * the US frame is sent to the python script.
     */
    void us_virtual_com_port_queue_process(void);



     /**@brief Function to initialize virtual COM port
     *
     * @details This function processes the virtual COM port. It is used to 
     * communicate between the python script and the receiver dongle.
     */
    void us_serial_init(void);


#endif




