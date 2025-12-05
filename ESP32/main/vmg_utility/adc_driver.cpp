#include "adc_driver.hpp"

#include <cstdint>

#include "driver/adc.h"
#include "driver/adc_types_legacy.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "hal/adc_types.h"
#include "soc/soc_caps.h"
#if lut_usage
float
vmg_adc_driver::get_lut(size_t reading)
{
  return this->_LUT[reading];
}
#else
void
vmg_adc_driver::calibration_init()
{
#if curve_fitting
  for (auto i : this->_digi_patt) {
    adc_cali_curve_fitting_config_t cali_config =
        {.unit_id = i.channel, .chan = i.channel, .atten = i.atten, .bitwidth = i.bitwidth} ESP_ERROR_CHECK(
            adc_cali_create_scheme_curve_fitting(&cali_config,
                                                 this->_cali_handle));
  }
};
#else
  adc_cali_line_fitting_config_t cali_config = {.unit_id  = this->_unit,
                                                .atten    = this->_attenuation,
                                                .bitwidth = this->_bitwidth,
#if NO_DEFAULT_VREF
                                                .default_vref = 1100
#endif
  };
  ESP_ERROR_CHECK(
      adc_cali_create_scheme_line_fitting(&cali_config, this->_cali_handle));
#endif
}

void
vmg_adc_driver::calibration_deinit()
{
  if (this->_cali_handle == nullptr) {
    return;
  }
#if curve_fitting
  adc_cali_delete_scheme_curve_fitting(this->_cali_handle);
#elif !lut_usage
  adc_cali_delete_scheme_line_fitting(this->_cali_handle);
#endif
}
#endif

void
vmg_adc_driver::stop_and_deinit_driver()
{
  ESP_ERROR_CHECK(adc_continuous_stop(*this->_handle));
  ESP_ERROR_CHECK(adc_continuous_deinit(*this->_handle));
}
void
vmg_adc_driver::init_driver()
{
  adc_continuous_handle_cfg_t handle_config = {
      .max_store_buf_size = this->_max_buff_size,
      .conv_frame_size    = this->_frame_size,
  };

  ESP_ERROR_CHECK(adc_continuous_new_handle(&handle_config, this->_handle));
}
void
vmg_adc_driver::config_digi_pattern(size_t const index,
                                    adc_channel_t const channel,
                                    adc_unit_t const unit,
                                    adc_bitwidth_t const bitwidth,
                                    adc_atten_t const attenuation)
{
  this->_digi_patt[index] = adc_digi_pattern_config_t{
      .atten     = static_cast<uint8_t>(attenuation),
      .channel   = static_cast<uint8_t>(channel),
      .unit      = static_cast<uint8_t>(unit),
      .bit_width = static_cast<uint8_t>(bitwidth),
  };
}
void
vmg_adc_driver::setup_digi_pattern()
{
  for (auto i : this->_digi_patt) {
    i.atten     = static_cast<uint8_t>(this->_attenuation);
    i.unit      = static_cast<uint8_t>(this->_unit);
    i.bit_width = static_cast<uint8_t>(this->_bitwidth);
  }
}

void
vmg_adc_driver::config_adc(adc_digi_convert_mode_t const convert,
                           uint32_t const frequency)
{
  adc_continuous_config_t adc_config = {
      .pattern_num    = static_cast<uint32_t>(this->_adc_amount),
      .adc_pattern    = this->_digi_patt.begin(),
      .sample_freq_hz = frequency,
      .conv_mode      = convert,
      .format         = adc_digi_output_format_t::ADC_DIGI_OUTPUT_FORMAT_TYPE1,
  };

  ESP_ERROR_CHECK(adc_continuous_config(*this->_handle, &adc_config));
}

int
vmg_adc_driver::get_data(adc_subscriber& subscriber)
{
  uint32_t read_length;

  return adc_continuous_read(*this->_handle, subscriber.get_buffer().begin(),
                             ADC_FRAME_SIZE, &read_length, 0) == ESP_OK
             ? read_length
             : -1;
}

uint64_t
adc_subscriber::getData()
{
  size_t len   = this->adc.get_data(this);
  size_t count = 1;
  uint64_t sum = 0;
  for (size_t i = 0; i < len; i += SOC_ADC_DIGI_RESULT_BYTES) {
    auto* p = reinterpret_cast<adc_digi_output_data_t*>(&this->buffer[i]);
    if (p->type1.channel >= ADC1_CHANNEL_MAX) {
      if (p->type1.data > 100) {  // > ~0.1V
        sum   += p->type1.data;
        count += 1;
      }
    }
  }
  if (count != 1) {
    sum /= --count;
  }
#if lut
  return this->adc->get_lut(sum);
#endif
  return sum;
}

std::array<uint8_t, ADC_FRAME_SIZE>&
adc_subscriber::get_buffer()
{
  return this->buffer;
}
