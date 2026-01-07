#include "i2c_driver.hpp"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "hal/i2c_types.h"
static constexpr uint8_t DEFAULT_GLITCH_IGNORE_COUNT{7};
vmg_i2c_driver::vmg_i2c_driver(
    i2c_port_num_t _i2c_port, gpio_num_t _sda_port, gpio_num_t _scl_port,
    i2c_clock_source_t _clk_source,
    uint8_t glitch_ignore_count = DEFAULT_GLITCH_IGNORE_COUNT)
{
  i2c_master_bus_config_t const i2c_mst_config{
      .i2c_port          = _i2c_port,
      .sda_io_num        = _sda_port,
      .scl_io_num        = _scl_port,
      .clk_source        = _clk_source,
      .glitch_ignore_cnt = glitch_ignore_count,
      .flags.enable_internal_pullup =
          PULLUP_INTERNAL_ENABLED || PULLUP_EXTERNAL_ENABLED};  //???

  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &_bus_handle));
}

#if PULLUP_INTERNAL_ENABLED || PULLUP_EXTERNAL_ENABLED
void
vmg_i2c_driver::probe(uint16_t addr)
{
  ESP_ERROR_CHECK(i2c_master_probe(_bus_handle, addr, PROBING_TIMEOUT_MS));
}
#endif

template <size_t Recv, size_t Send>
void
vmg_i2c_driver::device_config(i2c_device_config_t const& slave_config,
                              i2c_subscriber<Recv, Send>& subscriber)
{
  ESP_ERROR_CHECK(i2c_master_bus_add_device(_bus_handle, &slave_config,
                                            &subscriber.get_handle()));
}

template <size_t Recv, size_t Send>
void
i2c_subscriber<Recv, Send>::read_data(uint8_t amount)
{
  static_assert(amount <= Recv,
                "I2C: amount shouldn't be greater than template argument");
  ESP_ERROR_CHECK(i2c_master_transmit(_handle, _r_buffer, amount, -1));
}

template <size_t Recv, size_t Send>
void
i2c_subscriber<Recv, Send>::send_data(uint8_t amount)
{
  static_assert(amount <= Send,
                "I2C: amount shouldn't be greater than template argument");

  ESP_ERROR_CHECK(i2c_master_receive(_handle, _w_buffer, amount, -1));
}

template <size_t Recv, size_t Send>
void
i2c_subscriber<Recv, Send>::send_and_read_data(uint8_t send_amount,
                                               uint8_t receive_amount)
{
  static_assert(send_amount <= Send,
                "I2C: amount shouldn't be greater than template argument");

  static_assert(receive_amount <= Recv,
                "I2C: amount shouldn't be greater than template argument");

  ESP_ERROR_CHECK(i2c_master_transmit_receive(_handle, _w_buffer, send_amount,
                                              _r_buffer, receive_amount, -1));
}
