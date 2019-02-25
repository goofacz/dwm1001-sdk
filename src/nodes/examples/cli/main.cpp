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
#include "utilities.hpp"
#include "dw1000.hpp"

using namespace common;

/*
 * README
 *
 * UART CLI uses 230400 bps!
 */

static bool
handle_help_command(const gsl::span<const char*>& arguments)
{
   if(strcmp(arguments.at(0), "help") != 0)
   {
      return false;
   }

   cli::write_line("Available commands:");
   cli::write_line(" help - Displays this message");
   cli::write_line( "hex [dec_value] - Convert decimal value to hex");
   cli::write_line( "dec [hex_value] - Convert hex value to decimal");
   return true;
}

static bool
handle_hex_command(const gsl::span<const char*>& arguments)
{
   if(strcmp(arguments.at(0), "hex") != 0)
   {
      return false;
   }

   if(arguments.size() != 2)
   {
      ERROR_LOG("Wrong no. arguments");
      return true;
   }

   auto value = utilities::from_string<uint32_t>(arguments.at(1), 10);
   if(!value)
   {
      ERROR_LOG("Invalid \"value\"");
      return true;
   }
   
   cli::write_line("Result: 0x", cli::hex(*value));

   return true;
}

static bool
handle_dec_command(const gsl::span<const char*>& arguments)
{
   if(strcmp(arguments.at(0), "dec") != 0)
   {
      return false;
   }

   if(arguments.size() != 2)
   {
      ERROR_LOG("Wrong no. arguments");
      return true;
   }

   auto value = utilities::from_string<uint32_t>(arguments.at(1), 16);
   if(!value)
   {
      ERROR_LOG("Invalid \"value\"");
      return true;
   }
   
   cli::write_line("Result: ", *value);
   
   return true;
}

int
main()
{
   cli::initialize({{handle_help_command, handle_hex_command, handle_dec_command}});
   core::initialize();
   bluetooth::initialize("CliHelloWorld", {});
   dw1000::initialize(0x1234);

   INFO_LOG("CLI: Hello world");
   
   cli::schedule_command_reception();

   while(true)
   {
      app_sched_execute();
   }
}

