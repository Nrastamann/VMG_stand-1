#include "i2c_driver.hpp"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "hal/i2c_types.h"
static constexpr uint8_t DEFAULT_GLITCH_IGNORE_COUNT{7};
void
vmg_i2c_driver::config(
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

void
vmg_i2c_driver::device_config(i2c_device_config_t const& slave_config,
                              i2c_subscriber& subscriber)
{
  ESP_ERROR_CHECK(i2c_master_bus_add_device(_bus_handle, &slave_config,
                                            &subscriber.get_handle()));
}

template <size_t N>
void
i2c_subscriber<N>::read_data()
{
  ESP_ERROR_CHECK(i2c_master_transmit(_handle, _w_buffer, _wlen, -1));
}

template <size_t N>
void
i2c_subscriber<N>::send_data()
{
  ESP_ERROR_CHECK(i2c_master_receive(_handle, _r_buffer, _rlen, -1));
}

template <size_t N>
void
i2c_subscriber<N>::send_and_read_data()
{
  ESP_ERROR_CHECK(i2c_master_transmit_receive(_handle, _w_buffer, _wlen,
                                              _r_buffer, _rlen, -1));
}
