//
// Copyright (C) 2018 Tomasz Jankowski <t.jankowski AT pwr.edu.pl>
//                                     <tomasz.jankowski.mail AT gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#pragma once

#include <optional>

extern "C" {
#include "deca_device_api.h"
#include "deca_regs.h"
}

#include "gsl/gsl"

namespace common::dw1000
{
namespace time
{
constexpr uint16_t tx_antenna_delay{16300};
constexpr uint16_t rx_antenna_delay{16456};
constexpr auto uus_to_dwt_conversion_factor{65536};

} // namespace time

constexpr auto speed_of_light = 299702547.0l;

void initialize(uint16_t pan_id);

void set_irq_callbacks(dwt_cb_t tx_done_callback,
                       dwt_cb_t rx_done_callback,
                       dwt_cb_t rx_timeout_callback,
                       dwt_cb_t rx_error_callback);

void set_tx_buffer(const gsl::span<uint8_t>& buffer);

void get_tx_buffer(gsl::span<uint8_t> buffer);

std::optional<uint16_t> get_short_address();

void transmit_now();

bool transmit_deffered(uint64_t timestamp);

void receive_now(uint16_t timeout);

uint64_t get_rx_timestamp();

uint64_t get_tx_timestamp();

uint64_t get_clock_time();

} // namespace common::dw1000
