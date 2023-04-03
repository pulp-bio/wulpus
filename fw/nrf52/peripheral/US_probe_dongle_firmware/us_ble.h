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

/** @file us_ble.h
 *
 * @brief    Dongle firmware Bluetooth Low Energy (BLE) includes
 * 
*/

#ifndef US_BLE_H
#define US_BLE_H


    uint32_t send_ble_packet(uint8_t* start_address, uint16_t length);

     /**@brief Function to initialize Bluetooth Low Energy (BLE) 
     */
    void us_ble_init(void);

    /**@brief Function to disconnect BLE
     */
    uint32_t us_sd_ble_gap_disconnect(uint8_t hci_status_code);


#endif







