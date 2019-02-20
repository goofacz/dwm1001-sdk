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
#include <cstring>

extern "C" {
#include "bsp.h"
#include "nrf_delay.h"
#include "nrf_sdh_ble.h"
#include "nrfx_gpiote.h"
#include "nrfx_spim.h"
}

#include "bluetooth.hpp"
#include "cli.hpp"
#include "core.hpp"
#include "dw1000.hpp"

namespace common::dw1000
{
static constexpr auto rx_antenna_delay{16456};
static constexpr auto tx_antenna_delay{16300};

static const nrfx_spim_t instance = NRFX_SPIM_INSTANCE(1);
static volatile bool spi_io_done{false};
static const size_t spi_buffer_size{128};
static std::array<uint8_t, spi_buffer_size> spi_tx_buffer;
static std::array<uint8_t, spi_buffer_size> spi_rx_buffer;

extern "C" int
readfromspi(uint16_t header_length, const uint8_t* header, uint32_t body_length, uint8_t* body)
{
   const auto message_length{header_length + body_length};
   if(message_length > spi_buffer_size)
   {
      ERROR_LOG("SPI transaction too long");
   }

   spi_tx_buffer.fill(0);
   spi_rx_buffer.fill(0);

   std::copy(header, header + header_length, spi_tx_buffer.begin());

   nrfx_spim_xfer_desc_t transfer;
   transfer.p_tx_buffer = spi_tx_buffer.data();
   transfer.tx_length = message_length;
   transfer.p_rx_buffer = spi_rx_buffer.data();
   transfer.rx_length = message_length;

   spi_io_done = false;
   if(nrfx_spim_xfer(&instance, &transfer, 0) != NRFX_SUCCESS)
   {
      ERROR_LOG("SPI operation failed");
   }

   while(!spi_io_done)
   {
   }

   std::copy(spi_rx_buffer.data() + header_length, spi_rx_buffer.data() + message_length, body);

   return 0;
}

extern "C" int
writetospi(uint16_t header_length, const uint8_t* header, uint32_t body_length, const uint8_t* body)
{
   const auto message_length{header_length + body_length};
   if(message_length > spi_buffer_size)
   {
      ERROR_LOG("SPI transaction too long");
   }

   spi_tx_buffer.fill(0);
   spi_rx_buffer.fill(0);

   std::copy(header, header + header_length, spi_tx_buffer.begin());
   std::copy(body, body + body_length, spi_tx_buffer.begin() + header_length);

   nrfx_spim_xfer_desc_t transfer;
   transfer.p_tx_buffer = spi_tx_buffer.data();
   transfer.tx_length = message_length;
   transfer.p_rx_buffer = spi_rx_buffer.data();
   transfer.rx_length = message_length;

   spi_io_done = false;
   if(nrfx_spim_xfer(&instance, &transfer, 0) != NRFX_SUCCESS)
   {
      ERROR_LOG("SPI operation failed");
   }

   while(!spi_io_done)
   {
   }

   return 0;
}

extern "C" void
deca_sleep(unsigned int time_ms)
{
   nrf_delay_ms(time_ms);
}

extern "C" decaIrqStatus_t
decamutexon()
{
   return 0;
}

extern "C" void decamutexoff(decaIrqStatus_t)
{
}

extern "C" void
spi_callback(const nrfx_spim_evt_t* /*p_event*/, void* /*p_context*/)
{
   spi_io_done = true;
}

extern "C" void irq_callback(nrfx_gpiote_pin_t /*pin*/, nrf_gpiote_polarity_t /*action*/)
{
   dwt_isr();
}

static void
dummy_irq_callback(const dwt_cb_data_t*)
{
   ERROR_LOG("DW1000 IRQ callback was not set!");
}

static void
reset_chip()
{
   // FIXME Use nrfx module
   nrf_gpio_cfg_output(DW1000_RST);
   nrf_gpio_pin_clear(DW1000_RST);
   nrf_delay_ms(2);
   nrf_gpio_cfg_input(DW1000_RST, NRF_GPIO_PIN_NOPULL);
   nrf_delay_ms(2);
}

static bool
reset_sequence()
{
   if(nrfx_gpiote_init() != NRFX_SUCCESS)
   {
      ERROR_LOG("GPIOTE initialization failed");
      return false;
   }

   nrfx_gpiote_in_config_t irq_configuration; // FIXME Use NRFX_GPIOTE_RAW_CONFIG_IN_SENSE_LOTOHI
   irq_configuration.is_watcher = false;
   irq_configuration.hi_accuracy = true;
   irq_configuration.pull = NRF_GPIO_PIN_NOPULL;
   irq_configuration.sense = NRF_GPIOTE_POLARITY_LOTOHI;
   irq_configuration.skip_gpio_setup = true;
   const auto initialize_gpio_irq_result{
     nrfx_gpiote_in_init(DW1000_IRQ, &irq_configuration, irq_callback)};
   if(initialize_gpio_irq_result != NRFX_SUCCESS)
   {
      ERROR_LOG("DW1000 IRQ setup failed");
      return false;
   }

   nrfx_gpiote_in_event_enable(DW1000_IRQ, true);

   reset_chip();

   nrfx_spim_config_t spi_configuration = NRFX_SPIM_DEFAULT_CONFIG;
   spi_configuration.sck_pin = SPIM1_SCK_PIN;
   spi_configuration.miso_pin = SPIM1_MISO_PIN;
   spi_configuration.mosi_pin = SPIM1_MOSI_PIN;
   spi_configuration.ss_pin = SPIM1_SS_PIN;
   spi_configuration.frequency = NRF_SPIM_FREQ_2M;
   const auto initialize_spi_result{
     nrfx_spim_init(&instance, &spi_configuration, &spi_callback, nullptr)};
   if(initialize_spi_result != NRFX_SUCCESS)
   {
      ERROR_LOG("SPI 2MHz setup failed");
      return false;
   }

   nrf_delay_ms(2);

   const auto initialize_dwt_result{dwt_initialise(DWT_LOADUCODE)};
   if(initialize_dwt_result != DWT_SUCCESS)
   {
      ERROR_LOG("DW1000 initialization failed");
      return false;
   }

   nrfx_spim_uninit(&instance);

   spi_configuration.frequency = NRF_SPIM_FREQ_8M;
   const auto reinitialize_spi_result{
     nrfx_spim_init(&instance, &spi_configuration, &spi_callback, nullptr)};
   if(reinitialize_spi_result != NRFX_SUCCESS)
   {
      ERROR_LOG("SPI 8MHz setup failed");
      return false;
   }

   nrf_delay_ms(2);

   dwt_config_t dw1000_configuration;
   std::memset(&dw1000_configuration, 0, sizeof(dw1000_configuration));
   dw1000_configuration.chan = 5;
   dw1000_configuration.prf = DWT_PRF_64M;
   dw1000_configuration.txPreambLength = DWT_PLEN_128;
   dw1000_configuration.rxPAC = DWT_PAC8;
   dw1000_configuration.txCode = 10;
   dw1000_configuration.rxCode = 10;
   dw1000_configuration.nsSFD = 0;
   dw1000_configuration.dataRate = DWT_BR_6M8;
   dw1000_configuration.phrMode = DWT_PHRMODE_STD;
   dw1000_configuration.sfdTO = (129 + 8 - 8); // Preamble length + 1 + SFD length - PAC size)
   dwt_configure(&dw1000_configuration);

   dwt_setleds(DWT_LEDS_ENABLE);

   dwt_setcallbacks(dummy_irq_callback, dummy_irq_callback, dummy_irq_callback, dummy_irq_callback);
   dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RFCG | DWT_INT_RFTO | DWT_INT_RXPTO | DWT_INT_RPHE |
                      DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_SFDT | DWT_INT_ARFE,
                    1);

   dwt_setrxantennadelay(rx_antenna_delay);
   dwt_settxantennadelay(tx_antenna_delay);

   // TODO Call dwt_setpreambledetecttimeout(0); ?
   return true;
}

static void
set_address(uint16_t pan_id, uint16_t short_address)
{
   dwt_setpanid(pan_id);
   dwt_setaddress16(short_address);
}

void
initialize(uint16_t pan_id)
{
   const auto reset_result = reset_sequence();
   if(!reset_result)
   {
      return;
   }

   const auto short_address{get_short_address()};
   if(!short_address)
   {
      return;
   }

   set_address(pan_id, *short_address);
   INFO_LOG("DW1000 PAN ID: 0x", cli::hex(pan_id, 4), " Short address: 0x",
            cli::hex(*short_address, 4));

   dwt_enableframefilter(DWT_FF_DATA_EN);
   dwt_setrxantennadelay(rx_antenna_delay);
   dwt_settxantennadelay(tx_antenna_delay);
}

void
set_irq_callbacks(dwt_cb_t tx_done_callback,
                  dwt_cb_t rx_done_callback,
                  dwt_cb_t rx_timeout_callback,
                  dwt_cb_t rx_error_callback)
{
   dwt_setcallbacks(tx_done_callback, rx_done_callback, rx_timeout_callback, rx_error_callback);
}

void
set_tx_buffer(const gsl::span<uint8_t>& buffer)
{
   dwt_writetxdata(buffer.size(), buffer.data(), 0);
   dwt_writetxfctrl(buffer.size(), 0, 0);
}

void
get_tx_buffer(gsl::span<uint8_t> buffer)
{
   dwt_readrxdata(buffer.data(), buffer.size(), 0);
}

std::optional<uint16_t>
get_short_address()
{
   const auto address{bluetooth::get_address()};
   if(!address)
   {
      ERROR_LOG("Failed to obtain BLE address!");
      return std::nullopt;
   }

   return static_cast<uint16_t>((address->addr[4] << 8) | address->addr[5]);
}

void
transmit_now()
{
   dwt_starttx(DWT_START_TX_IMMEDIATE);
}

bool
transmit_deffered(uint64_t timestamp)
{
   // dwt_setdelayedtrxtime() writes to DX_TIME_ID at offset 1
   timestamp = (timestamp >> 8);
   dwt_setdelayedtrxtime(timestamp);

   const auto result{dwt_starttx(DWT_START_TX_DELAYED)};
   if(result != DWT_SUCCESS)
   {
      ERROR_LOG("Delayed TX time was passed!");
      return false;
   }

   return true;
}

void
receive_now(uint16_t timeout)
{
   dwt_rxreset();
   dwt_setrxtimeout(timeout);
   dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

uint64_t
get_rx_timestamp()
{
   std::array<uint8_t, 5> buffer{0};
   uint64_t result{0};

   dwt_readrxtimestamp(buffer.data());
   std::for_each(buffer.rbegin(), buffer.rend(),
                 [&result](const auto nibble) { result = (result << 8) | nibble; });

   return result;
}

uint64_t
get_tx_timestamp()
{
   std::array<uint8_t, 5> buffer{0};
   uint64_t result{0};

   dwt_readtxtimestamp(buffer.data());
   std::for_each(buffer.rbegin(), buffer.rend(),
                 [&result](const auto nibble) { result = (result << 8) | nibble; });

   return result;
}

uint64_t
get_clock_time()
{
   std::array<uint8_t, 5> buffer{0};
   uint64_t result{0};

   dwt_readsystime(buffer.data());
   std::for_each(buffer.rbegin(), buffer.rend(),
                 [&result](const auto nibble) { result = (result << 8) | nibble; });

   return result;
}

} // namespace common::dw1000
