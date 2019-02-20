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
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
}

#include "bluetooth.hpp"
#include "cli.hpp"
#include "core.hpp"
#include "dw1000.hpp"

using namespace common;

int
main()
{
   cli::initialize({});
   core::initialize();
   bluetooth::initialize("HelloWorld", {});
   dw1000::initialize(0x1234);

   INFO_LOG("Hello world");

   while(true)
   {
      app_sched_execute();
   }
}

