#pragma once
#include <array>

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "hal/i2c_types.h"
#include "soc/gpio_num.h"

#define PULLUP_INTERNAL_ENABLED false
#define PULLUP_EXTERNAL_ENABLED false

template <size_t N>
class i2c_subscriber;

class vmg_i2c_driver {
 public:
  vmg_i2c_driver() = delete;

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

  template <size_t N>
  void device_config(i2c_device_config_t const& slave_config,
                     i2c_subscriber<N>& subscriber);
  // send device as param and config

 private:
  i2c_port_num_t _i2c_port;
  gpio_num_t _sda_port;
  gpio_num_t _scl_port;
  i2c_clock_source_t _clk_source;
  uint16_t _dev_addr;
  uint32_t _scl_speed;

  i2c_master_bus_handle_t _bus_handle;
};

template <size_t N>
class i2c_subscriber {
 public:
  void read_data();
  // read data to buffer

  void send_data();
  // send data to slave

  void send_and_read_data();
  i2c_master_dev_handle_t&
  get_handle()
  {
    return _handle;
  }
  // send and read after that from slave

 private:
  size_t _wlen;
  size_t _rlen;

  std::array<uint8_t, N> _w_buffer;
  std::array<uint8_t, N> _r_buffer;

  i2c_master_dev_handle_t _handle;
};
