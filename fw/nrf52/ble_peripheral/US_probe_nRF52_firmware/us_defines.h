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

/** @file us_defines.h
 *
 * @brief    nRF52 Acquisition PCB general includes
 * 
*/

#ifndef US_DEFINES_H
#define US_DEFINES_H

    // Number of bytes per transfer to send to SPI slave
    #define BYTES_PR_XFER_TX   201
    // Number of bytes per transfer to receive from SPI slave
    #define BYTES_PR_XFER_RX   201

    // Number of SPI transfers to complete for one US frame
    #define NUMBER_OF_XFERS 4
    //#define DELAY_BETWEEN_TRANSFERS 1



    typedef struct ArrayList
    {
        uint8_t buffer[BYTES_PR_XFER_RX];
    } ArrayList_type;

#endif