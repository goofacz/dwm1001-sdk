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
}

#include "cli.hpp"
#include "utilities.hpp"

void
operator delete(void*, unsigned int)
{
   ERROR_LOG("Illegal delete() call!");
}

namespace common::utilities
{
bool
schedule_task(SchedulerTask task)
{
   if(const auto result = app_sched_event_put(nullptr, 0, task); result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to start task: 0x", cli::hex(reinterpret_cast<unsigned int>(task)),
                " error_code: ", cli::dec(result));
      return false;
   }

   return true;
}

} // namespace common::utilities
