#include "adc_driver.hpp"

#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"

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
vmg_adc_driver::config_digi_pattern(size_t index, adc_channel_t channel,
                                    adc_unit_t unit, adc_bitwidth_t bitwidth,
                                    adc_atten_t attenuation)
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
vmg_adc_driver::config_adc(adc_digi_convert_mode_t convert, uint32_t frequency)
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
