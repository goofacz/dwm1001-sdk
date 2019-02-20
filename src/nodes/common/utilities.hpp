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

#include <charconv>
#include <cstring>
#include <optional>
#include <type_traits>

namespace common::utilities
{
namespace details
{
template<auto...>
struct is_unique;

template<>
struct is_unique<>
{
   static constexpr bool value{true};
};

template<auto element, auto... elements>
struct is_unique<element, elements...>
{
   static constexpr bool value{((element != elements) && ...) && is_unique<elements...>::value};
};

} // namespace details

template<auto... Ts>
inline constexpr bool is_unique{details::is_unique<Ts...>::value};

template<typename T>
std::enable_if_t<std::is_integral_v<T>, std::optional<T>>
from_string(const char* argument, int base = 10)
{
   T result{0};
   if(auto [pointer, error_code] =
        std::from_chars(argument, argument + strlen(argument), result, base);
      error_code == std::errc())
   {
      return result;
   }
   else
   {
      return std::nullopt;
   }
}

using SchedulerTask = void (*)(void*, uint16_t);

bool schedule_task(SchedulerTask task);

} // namespace common::utilities
