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

#include "gsl/gsl"

#include <array>
#include <charconv>
#include <type_traits>

#define INFO_LOG(...) common::cli::write_line("INFO: ", __VA_ARGS__)
#define ERROR_LOG(...) common::cli::write_line("ERROR: ", __VA_ARGS__)
#ifdef DEBUGGING
#   define DEBUG_LOG(...) common::cli::write_line("DEBUG: ", __VA_ARGS__)
#else
#   define DEBUG_LOG(...)
#endif
namespace common::cli
{
namespace details
{
enum class Base : int
{
   decimal = 10,
   hexadecimal = 16
};

using Format = std::pair<Base, unsigned int>;

void write(const gsl::span<const uint8_t>& data);

void write(const char* argument);

template<typename T>
std::enable_if_t<std::is_integral_v<T>, void>
write(const T& argument)
{
   write(std::make_pair(argument, std::make_pair(details::Base::decimal, 0u)));
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>, void>
write(const std::pair<T, Format>& argument)
{
   const auto& format{argument.second};
   const auto base{static_cast<int>(format.first)};
   const auto leading_zeros{format.second};

   std::array<char, 64> buffer{'\0'};
   std::to_chars(buffer.data(), buffer.data() + buffer.size(), argument.first,
                 base); // FIXME Check result?

   const auto length{static_cast<int>(strlen(buffer.data()))};
   for(auto i{0}; i < static_cast<int>(leading_zeros) - length; i++)
   {
      write("0");
   }

   const auto pointer{reinterpret_cast<const uint8_t*>(buffer.data())};
   write({pointer, length});
}

} // namespace details

using Handler = bool (*)(const gsl::span<const char*>& arguments);

void initialize(const gsl::span<const Handler>& handlers);

bool schedule_command_reception();

void hexdump(const gsl::span<const uint8_t>& data);

template<typename... T>
void
write(const T&... arguments)
{
   (details::write(arguments), ...);
}

template<typename... T>
void
write_line(const T&... arguments)
{
   (details::write(arguments), ...);
   write("\r\n");
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>, std::pair<T, details::Format>>
dec(T value, unsigned int leading_zeros = 0)
{
   return std::make_pair(value, std::make_pair(details::Base::decimal, leading_zeros));
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>, std::pair<T, details::Format>>
hex(T value, unsigned int leading_zeros = 0)
{
   return std::make_pair(value, std::make_pair(details::Base::hexadecimal, leading_zeros));
}

} // namespace common::cli
