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
#include <cstring>
#include <string_view>

extern "C" {
#include "app_timer.h"
#include "ble_conn_params.h"
#include "nrf_ble_qwr.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
}

#include "bluetooth.hpp"
#include "cli.hpp"
#include "core.hpp"
#include "sdk_config.h"
#include "utilities.hpp"

namespace common::bluetooth
{
static const auto configuration_tag{1}; // A tag identifying the SoftDevice Bluetooth configuration

static uint16_t connection_handle{BLE_CONN_HANDLE_INVALID};
NRF_BLE_QWR_DEF(queue_write_module);
BLE_ADVERTISING_DEF(advertising_module);

static void
bluetooth_gap_connected_handler(const ble_evt_t* event)
{
   DEBUG_LOG("Connected");
   connection_handle = event->evt.gap_evt.conn_handle;
   const auto result = nrf_ble_qwr_conn_handle_assign(&queue_write_module, connection_handle);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to handle GAP connect!");
   }
}

static void
bluetooth_gap_disconnected_handler(const ble_evt_t* /*event*/)
{
   DEBUG_LOG("Disconnected");
   connection_handle = BLE_CONN_HANDLE_INVALID;
   const auto result = nrf_ble_qwr_conn_handle_assign(&queue_write_module, connection_handle);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to handle GAP disconnect!");
   }
}

static void
bluetooth_scurity_parameters_request_handler(const ble_evt_t* /*event*/)
{
   const auto result = sd_ble_gap_sec_params_reply(
     connection_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, nullptr, nullptr);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to handle GAP secutiry parameters request!");
   }
}

static void
bluetooth_phy_update_request_handler(const ble_evt_t* event)
{
   DEBUG_LOG("PHY update request.");
   ble_gap_phys_t phys;
   memset(&phys, 0, sizeof(phys));
   phys.rx_phys = BLE_GAP_PHY_AUTO;
   phys.tx_phys = BLE_GAP_PHY_AUTO;
   const auto result = sd_ble_gap_phy_update(event->evt.gap_evt.conn_handle, &phys);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to handle GAP PHY update request!");
   }
}

static void
bluetooth_gatt_server_system_atributes_missing_handler(const ble_evt_t* /*event*/)
{
   const auto result = sd_ble_gatts_sys_attr_set(connection_handle, nullptr, 0, 0);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to handle GATT Client system attributes missing!");
   }
}

static void
bluetooth_gatt_client_timeout_handler(const ble_evt_t* event)
{
   DEBUG_LOG("GATT Client timeoutt");
   const auto result = sd_ble_gap_disconnect(event->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to handle GATT Client timeout!");
   }
}

static void
bluetooth_gatt_server_timeout_handler(const ble_evt_t* event)
{
   const auto result = sd_ble_gap_disconnect(event->evt.gatts_evt.conn_handle,
                                             BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to handle GATT Server timeout!");
   }
}

extern "C" void
bluetooth_event_handler(const ble_evt_t* event, void* /*context*/)
{
   switch(event->header.evt_id)
   {
      case BLE_GAP_EVT_CONNECTED:
         bluetooth_gap_connected_handler(event);
         break;

      case BLE_GAP_EVT_DISCONNECTED:
         bluetooth_gap_disconnected_handler(event);
         break;

      case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
         bluetooth_scurity_parameters_request_handler(event);
         break;

      case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
         bluetooth_phy_update_request_handler(event);
         break;

      case BLE_GATTS_EVT_SYS_ATTR_MISSING:
         bluetooth_gatt_server_system_atributes_missing_handler(event);
         break;

      case BLE_GATTC_EVT_TIMEOUT:
         bluetooth_gatt_client_timeout_handler(event);
         break;

      case BLE_GATTS_EVT_TIMEOUT:
         bluetooth_gatt_server_timeout_handler(event);
         break;

      default:
         // No implementation needed.
         break;
   }
}

extern "C" void
queue_write_module_error_handler(uint32_t nrf_error)
{
   // TODO
}

extern "C" void
connection_parameters_event_handler(ble_conn_params_evt_t* event)
{
   if(event->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
   {
      const auto result =
        sd_ble_gap_disconnect(connection_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
      if(result != NRF_SUCCESS)
      {
         ERROR_LOG("Failed disconnect GAP!");
      }
   }
}

extern "C" void connection_parameters_error_handler(uint32_t /*nrf_error*/)
{
   ERROR_LOG("Failed disconnect GAP!");
}

static void
initialize_bluetooth_stack()
{
   const auto request_result = nrf_sdh_enable_request();
   if(request_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to enable SoftDevice Hanlder!");
   }

   uint32_t ram_start{0};
   const auto configuration_set_result = nrf_sdh_ble_default_cfg_set(configuration_tag, &ram_start);
   if(configuration_set_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to set SoftDevice config!");
   }

   const auto enable_result = nrf_sdh_ble_enable(&ram_start);
   if(enable_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to enable SoftDevice!");
   }

   const auto observer_priority{3}; // BLE observer priority
   NRF_SDH_BLE_OBSERVER(observer, observer_priority, bluetooth_event_handler, nullptr);
}

static void
initialize_gap(const std::string_view& device_name)
{
   ble_gap_conn_sec_mode_t security_mode;
   BLE_GAP_CONN_SEC_MODE_SET_OPEN(&security_mode);
   const auto set_name_result = sd_ble_gap_device_name_set(
     &security_mode, reinterpret_cast<const uint8_t*>(device_name.data()), device_name.size());
   if(set_name_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to set GAP device name!");
   }

   const auto min_interval{
     MSEC_TO_UNITS(100, UNIT_1_25_MS)}; // Minimum acceptable connection interval (0.5 seconds)
   const auto max_interval{
     MSEC_TO_UNITS(200, UNIT_1_25_MS)}; // Maximum acceptable connection interval (1 second)
   const auto slave_latency{0}; // Slave latency
   const auto supervisory_timeout{
     MSEC_TO_UNITS(4000, UNIT_10_MS)}; // Connection supervisory time-out (4 seconds)
   ble_gap_conn_params_t parameters;

   memset(&parameters, 0, sizeof(parameters));
   parameters.min_conn_interval = min_interval;
   parameters.max_conn_interval = max_interval;
   parameters.slave_latency = slave_latency;
   parameters.conn_sup_timeout = supervisory_timeout;

   const auto set_ppcp_result = sd_ble_gap_ppcp_set(&parameters);
   if(set_ppcp_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to set GAP PPCP!");
   }
}

static void
initialize_queue_write_module()
{
   nrf_ble_qwr_init_t parameters;
   memset(&parameters, 0, sizeof(parameters));
   parameters.error_handler = queue_write_module_error_handler;

   const auto result = nrf_ble_qwr_init(&queue_write_module, &parameters);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to initialize Queue Write Module!");
   }
}

static void
initialize_advertising_module(const ble_advdata_uuid_list_t& uuids)
{
   const auto interval{64}; // 40 ms; units of 0.625 ms
   const auto duration{BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED};
   ble_advertising_init_t parameters;
   memset(&parameters, 0, sizeof(parameters));

   parameters.advdata.name_type = BLE_ADVDATA_FULL_NAME;
   parameters.advdata.include_appearance = true;
   parameters.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
   parameters.advdata.uuids_complete.uuid_cnt = uuids.uuid_cnt;
   parameters.advdata.uuids_complete.p_uuids = uuids.p_uuids;

   parameters.config.ble_adv_fast_enabled = true;
   parameters.config.ble_adv_fast_interval = interval;
   parameters.config.ble_adv_fast_timeout = duration;

   parameters.evt_handler = nullptr;

   const auto result = ble_advertising_init(&advertising_module, &parameters);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to initialize Advertising!");
   }

   ble_advertising_conn_cfg_tag_set(&advertising_module, configuration_tag);
}

static void
initialize_connection_parameters_module()
{
   const auto first_update_dealy{APP_TIMER_TICKS(20000)};
   const auto next_update_dealy{APP_TIMER_TICKS(5000)};
   const auto max_update_dealy{3};
   ble_conn_params_init_t parameters;
   memset(&parameters, 0, sizeof(parameters));

   parameters.p_conn_params = nullptr;
   parameters.first_conn_params_update_delay = first_update_dealy;
   parameters.next_conn_params_update_delay = next_update_dealy;
   parameters.max_conn_params_update_count = max_update_dealy;
   parameters.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
   parameters.disconnect_on_fail = true;
   parameters.evt_handler = connection_parameters_event_handler;
   parameters.error_handler = connection_parameters_error_handler;

   const auto result = ble_conn_params_init(&parameters);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to initialize Connection Parameters!");
   }
}

void
CharacteristicBase::register_characteristic(const Parameters& parameters,
                                            uint16_t initial_value_size,
                                            uint16_t maximum_value_size,
                                            uint8_t* value,
                                            bool variable_length)
{
   ble_gatts_attr_md_t cccd_metadata;
   memset(&cccd_metadata, 0, sizeof(cccd_metadata));
   BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_metadata.read_perm);
   BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_metadata.write_perm);
   cccd_metadata.vloc = BLE_GATTS_VLOC_STACK;

   ble_gatts_char_md_t characteristic_metadata;
   memset(&characteristic_metadata, 0, sizeof(characteristic_metadata));
   characteristic_metadata.char_props.read = parameters.read;
   characteristic_metadata.char_props.write = parameters.write;
   characteristic_metadata.char_props.notify = parameters.notify;
   characteristic_metadata.p_char_user_desc = nullptr;
   characteristic_metadata.p_char_pf = nullptr;
   characteristic_metadata.p_user_desc_md = nullptr;
   characteristic_metadata.p_cccd_md = &cccd_metadata;
   characteristic_metadata.p_sccd_md = nullptr;

   ble_gatts_attr_md_t attribute_metadata;
   memset(&attribute_metadata, 0, sizeof(attribute_metadata));
   BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attribute_metadata.read_perm);
   BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attribute_metadata.write_perm);
   attribute_metadata.vloc = BLE_GATTS_VLOC_USER;
   attribute_metadata.rd_auth = 0;
   attribute_metadata.wr_auth = 0;
   attribute_metadata.vlen = variable_length ? 1 : 0;

   ble_uuid_t uuid;
   memset(&uuid, 0, sizeof(uuid));
   uuid.type = parameters.service.get_uuid_type();
   uuid.uuid = parameters.uuid;

   ble_gatts_attr_t attribute_characteristic_value;
   memset(&attribute_characteristic_value, 0, sizeof(attribute_characteristic_value));
   attribute_characteristic_value.p_uuid = &uuid;
   attribute_characteristic_value.p_attr_md = &attribute_metadata;
   attribute_characteristic_value.init_len = initial_value_size;
   attribute_characteristic_value.p_value = value;
   attribute_characteristic_value.init_offs = 0;
   attribute_characteristic_value.max_len = maximum_value_size;

   const auto result =
     sd_ble_gatts_characteristic_add(parameters.service.get_handle(), &characteristic_metadata,
                                     &attribute_characteristic_value, &handles);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to add characteristic to GATT!");
   }
}

void
CharacteristicBase::update_value(uint16_t value_size, uint8_t* value)
{
   ble_gatts_value_t gatt_value;
   memset(&gatt_value, 0, sizeof(gatt_value));
   gatt_value.len = value_size;
   gatt_value.offset = 0;
   gatt_value.p_value = value;

   const auto set_result =
     sd_ble_gatts_value_set(connection_handle, handles.value_handle, &gatt_value);
   if(set_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to update GATT Chracteristic value!");
   }
}

void
CharacteristicBase::send_notification(uint16_t value_size, uint8_t* value)
{
   if(connection_handle == BLE_CONN_HANDLE_INVALID)
   {
      return;
   }

   ble_gatts_hvx_params_t hvx_parameters;
   memset(&hvx_parameters, 0, sizeof(hvx_parameters));
   hvx_parameters.handle = handles.value_handle;
   hvx_parameters.type = BLE_GATT_HVX_NOTIFICATION;
   hvx_parameters.offset = 0;
   hvx_parameters.p_len = &value_size;
   hvx_parameters.p_data = value;

   const auto notify_result = sd_ble_gatts_hvx(connection_handle, &hvx_parameters);
   if(notify_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to notify about GATT Chracteristic value!");
   }
}

const ble_gatts_char_handles_t&
CharacteristicBase::get_handles() const
{
   return handles;
}

void
Service::register_service(const ble_uuid128_t& uuid_base, uint16_t service_uuid)
{
   const auto add_vendor_service_result = sd_ble_uuid_vs_add(&uuid_base, &uuid_type);
   if(add_vendor_service_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to Vendor Specific UUID!");
   }

   ble_uuid_t uuid;
   memset(&uuid, 0, sizeof(uuid));
   uuid.type = uuid_type;
   uuid.uuid = service_uuid;

   const auto add_gatt_service_result =
     sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &uuid, &handle);
   if(add_gatt_service_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to add service to GATTT!");
   }
}

uint8_t
Service::get_uuid_type() const
{
   return uuid_type;
}

uint16_t
Service::get_handle() const
{
   return handle;
}

CharacteristicBase::Parameters::Parameters(const Service& initial_service, uint16_t initial_uuid)
  : service{initial_service}, uuid{initial_uuid}
{
   // nop
}

void
initialize(const std::string_view device_name,
           std::initializer_list<ServiceInitializer> service_initializers)
{
   initialize_bluetooth_stack();
   initialize_gap(device_name);
   initialize_queue_write_module();
   initialize_connection_parameters_module();

   ble_uuid_t uuids[service_initializers.size()];
   size_t i{0};
   for(auto& service_initializer : service_initializers)
   {
      service_initializer(uuids[i]);
      i++;
   }

   ble_advdata_uuid_list_t uuid_list;
   uuid_list.uuid_cnt = service_initializers.size();
   uuid_list.p_uuids = uuids;
   initialize_advertising_module(uuid_list);

   const auto result = ble_advertising_start(&advertising_module, BLE_ADV_MODE_FAST);
   if(result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to start advertising!");
   }

   const auto address = get_address();
   if(!address)
   {
      ERROR_LOG("Failed to obtain BLE address!");
   }

   INFO_LOG("BLE address: ", cli::hex(address->addr[5], 2), "-", cli::hex(address->addr[4], 2), "-",
            cli::hex(address->addr[3], 2), "-", cli::hex(address->addr[2], 2), "-",
            cli::hex(address->addr[1], 2), "-", cli::hex(address->addr[0], 2));
}

std::optional<ble_gap_addr_t>
get_address()
{
   ble_gap_addr_t bluetooth_address;
   memset(&bluetooth_address, 0, sizeof(bluetooth_address));
   const auto get_address_result = sd_ble_gap_addr_get(&bluetooth_address);
   if(get_address_result != NRF_SUCCESS)
   {
      ERROR_LOG("Failed to obtain BLE address!");
      return std::nullopt;
   }

   return bluetooth_address;
}

} // namespace common::bluetooth
