/**
 * Copyright (c) 2017 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
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
 * 5. Any software provided in binary form under this license must not be
 * reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

#include "dwm1001_board.h"

// Non-nRF configuration
#define GSL_UNENFORCED_ON_CONTRACT_VIOLATION

#define NRFX_UARTE_ENABLED 1
#define NRFX_UARTE_DEFAULT_CONFIG_HWFC 0
#define NRFX_UARTE_DEFAULT_CONFIG_PARITY 0
// 61865984 - 230400 baud; 30801920 - 115200 baud
#define NRFX_UARTE_DEFAULT_CONFIG_BAUDRATE 61865984
#define NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define NRFX_UARTE0_ENABLED 1

#define NRFX_SPIM_ENABLED 1
#define NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define NRFX_SPIM_NRF52_ANOMALY_109_WORKAROUND_ENABLED 1
#define NRFX_SPIM_MISO_PULL_CFG 0
#define NRFX_SPIM1_ENABLED 1

#define NRF_BLE_CONN_PARAMS_ENABLED 1
#define NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION 499
#define NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION 65535
#define NRF_BLE_GATT_ENABLED 1

#define NRF_BLE_QWR_ENABLED 1
#define NRF_BLE_QWR_MAX_ATTR 0

#define NRFX_CLOCK_ENABLED 1
#define NRFX_CLOCK_CONFIG_LF_SRC 1 // XTAL
#define NRFX_CLOCK_CONFIG_IRQ_PRIORITY 6

#define NRFX_GPIOTE_ENABLED 1
#define NRFX_GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 1
#define NRFX_GPIOTE_CONFIG_IRQ_PRIORITY 7

#define NRFX_PRS_ENABLED 1
#define NRFX_PRS_BOX_0_ENABLED 0
#define NRFX_PRS_BOX_1_ENABLED 0
#define NRFX_PRS_BOX_2_ENABLED 0
#define NRFX_PRS_BOX_3_ENABLED 0
#define NRFX_PRS_BOX_4_ENABLED 1

#define NRF_CLOCK_ENABLED 1 // XTAL
#define CLOCK_CONFIG_LF_SRC 1
#define CLOCK_CONFIG_IRQ_PRIORITY 6

#define APP_SCHEDULER_ENABLED 1
#define APP_SCHEDULER_WITH_PAUSE 0
#define APP_SCHEDULER_WITH_PROFILER 0

#define APP_TIMER_ENABLED 1
#define APP_TIMER_CONFIG_RTC_FREQUENCY 0 // 32768 Hz
#define APP_TIMER_CONFIG_IRQ_PRIORITY 6
#define APP_TIMER_CONFIG_OP_QUEUE_SIZE 10
#define APP_TIMER_CONFIG_USE_SCHEDULER 0
#define APP_TIMER_KEEPS_RTC_ACTIVE 0
#define APP_TIMER_SAFE_WINDOW_MS 300000
#define APP_TIMER_WITH_PROFILER 0
#define APP_TIMER_CONFIG_SWI_NUMBER 0

#define NRF_BALLOC_ENABLED 1

#define NRF_PWR_MGMT_ENABLED 1
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED 0
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S 3
#define NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED 1
#define NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY 0
#define NRF_PWR_MGMT_CONFIG_USE_SCHEDULER 1
#define NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT 3

#define NRF_SECTION_ITER_ENABLED 1

#define NRF_STRERROR_ENABLED 0

#define NRF_SDH_BLE_ENABLED 1
#define NRF_SDH_BLE_GAP_DATA_LENGTH 27
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 1
#define NRF_SDH_BLE_CENTRAL_LINK_COUNT 0
#define NRF_SDH_BLE_TOTAL_LINK_COUNT 1
#define NRF_SDH_BLE_GAP_EVENT_LENGTH 6
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE 23
#define NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE 1408
#define NRF_SDH_BLE_VS_UUID_COUNT 10
#define NRF_SDH_BLE_SERVICE_CHANGED 0
#define NRF_SDH_BLE_OBSERVER_PRIO_LEVELS 4

#define BLE_ADV_SOC_OBSERVER_PRIO 1
#define BLE_ADV_BLE_OBSERVER_PRIO 1
#define BLE_CONN_PARAMS_BLE_OBSERVER_PRIO 1
#define BLE_CONN_STATE_BLE_OBSERVER_PRIO 0
#define BLE_DB_DISC_BLE_OBSERVER_PRIO 1
#define BLE_DIS_C_BLE_OBSERVER_PRIO 2
#define BSP_BTN_BLE_OBSERVER_PRIO 1
#define NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO 2
#define NRF_BLE_GATT_BLE_OBSERVER_PRIO 1
#define NRF_BLE_QWR_BLE_OBSERVER_PRIO 2
#define NRF_BLE_SCAN_OBSERVER_PRIO 1
#define PM_BLE_OBSERVER_PRIO 1

#define NRF_SDH_ENABLED 1

#define NRF_SDH_DISPATCH_MODEL NRF_SDH_DISPATCH_MODEL_INTERRUPT

#define NRF_SDH_CLOCK_LF_SRC 1
#define NRF_SDH_CLOCK_LF_RC_CTIV 0
#define NRF_SDH_CLOCK_LF_RC_TEMP_CTIV 0
#define NRF_SDH_CLOCK_LF_ACCURACY NRF_CLOCK_LF_ACCURACY_20_PPM

#define NRF_SDH_REQ_OBSERVER_PRIO_LEVELS 2
#define NRF_SDH_STATE_OBSERVER_PRIO_LEVELS 2
#define NRF_SDH_STACK_OBSERVER_PRIO_LEVELS 2

#define CLOCK_CONFIG_STATE_OBSERVER_PRIO 0
#define POWER_CONFIG_STATE_OBSERVER_PRIO 0
#define RNG_CONFIG_STATE_OBSERVER_PRIO 0

#define NRF_SDH_ANT_STACK_OBSERVER_PRIO 0
#define NRF_SDH_BLE_STACK_OBSERVER_PRIO 0
#define NRF_SDH_SOC_STACK_OBSERVER_PRIO 0

#define NRF_SDH_SOC_ENABLED 1
#define NRF_SDH_SOC_OBSERVER_PRIO_LEVELS 2

#define CLOCK_CONFIG_SOC_OBSERVER_PRIO 0

#define BLE_ADVERTISING_ENABLED 1

#define NRF_QUEUE_ENABLED 1

#define NRF_LOG_ENABLED 0

#endif // SDK_CONFIG_H
