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

#include <algorithm>
#include <array>
#include <charconv>
#include <system_error>

extern "C" {
#include "app_scheduler.h"
#include "nrfx_uarte.h"
}

#include "cli.hpp"
#include "utilities.hpp"

namespace common::cli
{
static const nrfx_uarte_t uart_instance = NRFX_UARTE_INSTANCE(0);
static std::array<Handler, 10> command_handlers{nullptr};

namespace details
{
void
write(const gsl::span<const uint8_t>& data)
{
   const int buffer_size{32};
   auto begin{data.cbegin()};
   auto end{data.size() < buffer_size ? data.cend() : begin + buffer_size};

   while(begin != end)
   {
      std::array<uint8_t, buffer_size> buffer;
      std::copy(begin, end, buffer.begin());
      nrfx_uarte_tx(&uart_instance, buffer.data(),
                    std::distance(begin, end)); // FIXME Check result?

      begin = end;
      end = std::distance(end, data.cend()) < buffer_size ? data.cend() : end + buffer_size;
   }
}

void
write(const char* argument)
{
   const auto pointer{reinterpret_cast<const uint8_t*>(argument)};
   const auto length{static_cast<int>(strlen(argument))};
   write({pointer, length});
}

} // namespace details

static bool
is_printable_ascii(uint8_t value)
{
   return (value >= 0x20) && (value <= 0x7e);
}

static bool
read_arguments(std::array<char, 64>& storage, std::array<const char*, 5>& arguments)
{
   arguments.fill(nullptr);

   auto character{storage.begin()};
   auto argument{arguments.begin()};
   while(true)
   {
      uint8_t buffer{0};
      nrfx_uarte_rx(&uart_instance, &buffer, 1); // FIXME Check result?

      if(buffer == '\r')
      {
         write("\n\r");
      }
      else if(!is_printable_ascii(buffer))
      {
         continue;
      }
      else
      {
         nrfx_uarte_tx(&uart_instance, &buffer, 1); // FIXME Check result?
      }

      if(buffer == '\r')
      {
         *character = '\0';
         break;
      }
      else if(buffer == ' ')
      {
         if(character == storage.begin() || *std::prev(character) == '\0')
         {
            continue;
         }
         else
         {
            *character = '\0';
            ++character;
            ++argument;
         }
      }
      else
      {
         if(character == storage.begin() || *std::prev(character) == '\0')
         {
            if(argument == arguments.end())
            {
               write_line("\r\nToo many arguments");
               return false;
            }

            *argument = storage.data() + std::distance(storage.begin(), character);
         }

         *character = static_cast<char>(buffer);
         ++character;
      }

      if(character == storage.end())
      {
         write_line("\r\n");
         ERROR_LOG("Command too long");
         return false;
      }
   }

   return true;
}

static void
process_command()
{
   write("##> ");

   std::array<char, 64> storage;
   std::array<const char*, 5> arguments;

   if(!read_arguments(storage, arguments))
   {
      return;
   }

   const auto is_null{[](const char* element) { return element == nullptr; }};
   if(std::all_of(arguments.begin(), arguments.end(), is_null))
   {
      return;
   }

   for(const auto& handler : command_handlers)
   {
      if(!handler)
      {
         break;
      }

      const auto first_invalid_argument{std::find_if(arguments.begin(), arguments.end(), is_null)};
      const auto valid_arguments{std::distance(arguments.begin(), first_invalid_argument)};

      if(handler({arguments.data(), valid_arguments}))
      {
         return;
      }
   }

   ERROR_LOG("Unknown command");
}

static void
receive_command_task(void* /*event*/, uint16_t /*event_size*/)
{
   process_command();

   if(!schedule_command_reception())
   {
      while(true)
      {
         // nop
      }
   }
}

void
initialize(const gsl::span<const Handler>& handlers)
{
   nrfx_uarte_config_t uart_config = NRFX_UARTE_DEFAULT_CONFIG;
   uart_config.pseltxd = TX_PIN_NUM;
   uart_config.pselrxd = RX_PIN_NUM;
   nrfx_uarte_init(&uart_instance, &uart_config, nullptr); // FIXME Check result?

   if(static_cast<unsigned int>(handlers.size()) > command_handlers.size())
   {
      ERROR_LOG("CLI can handle at most ", command_handlers.size(), " commands!");
      return;
   }

   std::copy(handlers.begin(), handlers.end(), command_handlers.begin());
}

bool
schedule_command_reception()
{
   return utilities::schedule_task(receive_command_task);
}

void
hexdump(const gsl::span<const uint8_t>& data)
{
   const auto row_length{4};
   for(auto row{0}; (row * row_length) < data.size(); row++)
   {
      write(dec(row, 3), " | ");

      const auto offset{row * row_length};
      const auto first_byte{std::next(data.cbegin(), offset)};
      const auto bytes_number{std::distance(first_byte, data.cend()) < row_length
                                ? std::distance(first_byte, data.cend())
                                : row_length};

      std::for_each(first_byte, std::next(first_byte, bytes_number),
                    [](uint8_t byte) { write(hex(byte, 2), " "); });

      for(auto i{0}; i < (row_length - bytes_number); i++)
      {
         write("   ");
      }

      write("| \n\r");
   }
}

} // namespace common::cli
