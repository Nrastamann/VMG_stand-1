#pragma once
#include <array>
#include <cstdint>

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "hal/i2c_types.h"
#include "soc/gpio_num.h"

#define PULLUP_INTERNAL_ENABLED false
#define PULLUP_EXTERNAL_ENABLED false

class I2C_DRIVER_SUBSCRIBER_TAG {};

static constexpr uint8_t SENDLEN{UINT8_MAX};
static constexpr uint8_t RECVLEN{UINT8_MAX};

template <size_t Recv, size_t Send>
class i2c_subscriber;

class vmg_i2c_driver {
 public:
  vmg_i2c_driver() = delete;
  vmg_i2c_driver(i2c_port_num_t _i2c_port, gpio_num_t _sda_port,
                 gpio_num_t _scl_port, i2c_clock_source_t _clk_source,
                 uint8_t glitch_ignore_count);
#if PULLUP_INTERNAL_ENABLED || PULLUP_EXTERNAL_ENABLED
  void probe(uint16_t addr);  // search for given address
  static constexpr uint8_t PROBING_TIMEOUT_MS{100};
#endif

  void config(uint8_t glitch_ignore_count);

  // config

  void
  stop_bus()
  {
    ESP_ERROR_CHECK(i2c_del_master_bus(_bus_handle));
  }

  template <size_t Recv, size_t Send>
  void device_config(i2c_device_config_t const& slave_config,
                     i2c_subscriber<Recv, Send>& subscriber);
  // send device as param and config

 private:
  i2c_master_bus_handle_t _bus_handle;
};

template <size_t Recv = RECVLEN, size_t Send = SENDLEN>
class i2c_subscriber {
 public:
  void read_data(uint8_t amount = Recv);
  // read data to buffer

  void send_data(uint8_t amount = Send);
  // send data to slave

  void send_and_read_data(uint8_t send_amount    = Send,
                          uint8_t receive_amount = Recv);
  i2c_master_dev_handle_t&
  get_handle()
  {
    return _handle;
  }
  // send and read after that from slave

  size_t _wlen;
  size_t _rlen;

  std::array<uint8_t, Send> _w_buffer;
  std::array<uint8_t, Recv> _r_buffer;

 private:
  i2c_master_dev_handle_t _handle;
};
