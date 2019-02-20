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

extern "C" {
#include "app_scheduler.h"
#include "app_timer.h"
#include "boards.h"
#include "nrf_pwr_mgmt.h"
}

#include "cli.hpp"
#include "core.hpp"

namespace common::core
{
void
initialize()
{
   bsp_board_init(BSP_INIT_LEDS);

   APP_SCHED_INIT(0, 5);

   INFO_LOG("Build: ", GIT_COMMIT);

   if(app_timer_init() != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to init timer!");
   }

   if(nrf_pwr_mgmt_init() != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to init power mgmt!");
   }
}

} // namespace common::core
