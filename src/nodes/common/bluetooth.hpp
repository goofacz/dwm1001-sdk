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

#include "gsl/gsl"

#include <array>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>

extern "C" {
#include "ble_advertising.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
}

namespace common::bluetooth
{
class Service final
{
 public:
   Service() = default;
   Service(const Service&) = delete;
   Service(Service&&) = delete;
   ~Service() = default;

   Service& operator=(const Service&) = delete;
   Service& operator=(Service&&) = delete;

   void register_service(const ble_uuid128_t& uuid_base, uint16_t service_uuid);

   uint8_t get_uuid_type() const;
   uint16_t get_handle() const;

 private:
   uint8_t uuid_type{0};
   uint16_t handle{0};
};

class CharacteristicBase
{
 public:
   struct Parameters final
   {
      Parameters(const Service& initial_service, uint16_t initial_uuid);

      const Service& service;
      uint16_t uuid{0};
      bool read{false};
      bool write{false};
      bool notify{false};
   };

 public:
   CharacteristicBase(const CharacteristicBase&) = delete;
   CharacteristicBase(CharacteristicBase&&) = delete;
   virtual ~CharacteristicBase() = default;

   CharacteristicBase& operator=(const CharacteristicBase&) = delete;
   CharacteristicBase& operator=(CharacteristicBase&&) = delete;

   const ble_gatts_char_handles_t& get_handles() const;

 protected:
   CharacteristicBase() = default;

   void register_characteristic(const Parameters& parameters,
                                uint16_t initial_value_size,
                                uint16_t maximum_value_size,
                                uint8_t* value,
                                bool variable_length);
   void update_value(uint16_t value_size, uint8_t* value);
   void send_notification(uint16_t value_size, uint8_t* value);

 private:
   ble_gatts_char_handles_t handles{};
};

template<typename T>
class Characteristic : public CharacteristicBase
{
 public:
   explicit Characteristic(T&& initial_value) : value{std::forward<T>(initial_value)}
   {
      // nop
   }

   Characteristic(const Characteristic&) = delete;
   Characteristic(Characteristic&&) = delete;
   ~Characteristic() = default;

   Characteristic& operator=(const Characteristic&) = delete;
   Characteristic& operator=(Characteristic&&) = delete;

   void
   register_characteristic(const Parameters& parameters)
   {
      CharacteristicBase::register_characteristic(parameters, sizeof(value), sizeof(value),
                                                  reinterpret_cast<uint8_t*>(&value), false);
   }

   void
   send_notification()
   {
      CharacteristicBase::send_notification(sizeof(value), reinterpret_cast<uint8_t*>(&value));
   }

   const T&
   get_value() const
   {
      return value;
   }

   void
   set_value(const T& new_value)
   {
      value = new_value;
      update_value(sizeof(value), reinterpret_cast<uint8_t*>(&value));
   }

 private:
   T value{};
};

template<>
class Characteristic<uint8_t*> : public CharacteristicBase
{
 public:
   Characteristic() = default;
   Characteristic(const Characteristic&) = delete;
   Characteristic(Characteristic&&) = delete;
   ~Characteristic() = default;

   Characteristic& operator=(const Characteristic&) = delete;
   Characteristic& operator=(Characteristic&&) = delete;

   void
   register_characteristic(const Parameters& parameters, uint16_t initial_value_size)
   {
      CharacteristicBase::register_characteristic(parameters, initial_value_size,
                                                  BLE_GATTS_VAR_ATTR_LEN_MAX, value.data(), true);
   }

   void
   send_notification()
   {
      CharacteristicBase::send_notification(value_size, value.data());
   }

   const uint8_t*
   get_value() const
   {
      return value.data();
   }

   void
   set_value(gsl::span<uint8_t> new_value)
   {
      std::copy(new_value.begin(), new_value.end(), value.begin());
      value_size = new_value.size();
      update_value(value_size, value.data());
   }

 private:
   std::array<uint8_t, BLE_GATTS_VAR_ATTR_LEN_MAX> value{0};
   uint16_t value_size{0};
};

using ServiceInitializer = void (*)(ble_uuid_t& uuid);

void initialize(const std::string_view device_name,
                std::initializer_list<ServiceInitializer> service_initializers);

std::optional<ble_gap_addr_t> get_address();

} // namespace common::bluetooth
