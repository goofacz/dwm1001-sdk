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

#include "gbee/gbee.hpp"
#include "gsl/gsl"
#include "utilities.hpp"

#include <array>

namespace common
{
enum class MacHeaderField
{
   frame_control,
   sequence_number,
   pan_id,
   destination_short_address,
   source_short_address,
};

constexpr uint16_t default_frame_type{0x8841};
constexpr size_t default_extra_size{2}; // FCS at the end

using MacHeader = gbee::Group<gbee::Field<MacHeaderField::frame_control, uint16_t>,
                              gbee::Field<MacHeaderField::sequence_number, uint8_t>,
                              gbee::Field<MacHeaderField::pan_id, uint16_t>,
                              gbee::Field<MacHeaderField::destination_short_address, uint16_t>,
                              gbee::Field<MacHeaderField::source_short_address, uint16_t>>;

} // namespace common
