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

/** @file us_spi.h
 *
 * @brief    nRF52 Acquisition PCB firmware SPI includes
 * 
*/

#ifndef US_SPI_H
#define US_SPI_H

    /**@brief Initialize SPI with the timer, counter and the PPI
     *
     *@details This functions initializes the SPI transfers. It
     * also initializes the timer, counter and the PPI which are 
     * necessary to control the SPI transactions independently
     * from the CPU.
     */
    void us_spi_init(void);


#endif




