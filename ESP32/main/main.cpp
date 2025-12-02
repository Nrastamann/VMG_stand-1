#include <memory.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <cstdio>

#include "adc_reading.hpp"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "hal/adc_types.h"

constexpr bool lut_usage{true};
constexpr size_t AMOUNT_OF_ADC_SENSORS{2};
constexpr size_t SAMPLE_FREQUENCY{20000};
constexpr size_t ADC_RANGE{4096};

class vmg_adc_driver {
 public:
  vmg_adc_driver(size_t const amount, uint32_t const max_buff_size,
                 uint32_t const frame_size) :
      _adc_amount(amount),
      _frame_size(frame_size),
      _max_buff_size(max_buff_size)
  {
  }
  vmg_adc_driver() = delete;
#if lut_usage
  float
  get_lut(size_t reading)
  {
    return this->_LUT[reading];
  }
#else
  static bool
  a()
  {  // надо добавить дефайны на то, какие схемы поддержаны и соответственно
     // написать
  }
#endif
  void
  stop_driver()
  {
    ESP_ERROR_CHECK(adc_continuous_stop(*this->_handle));
  }
  void
  start_driver()
  {
    ESP_ERROR_CHECK(adc_continuous_start(*this->_handle));
  }
  adc_continuous_handle_t*
  get_handle()
  {
    return this->_handle;
  }

  void
  stop_and_deinit_driver()
  {
    ESP_ERROR_CHECK(adc_continuous_stop(*this->_handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(*this->_handle));
  }
  void
  init_driver()
  {
    adc_continuous_handle_cfg_t handle_config = {
        .max_store_buf_size = this->_max_buff_size,
        .conv_frame_size    = this->_frame_size,
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&handle_config, this->_handle));
  }
  void
  config_digi_pattern(size_t index, adc_channel_t channel,
                      adc_bitwidth_t bitwidth = ADC_BITWIDTH_12,
                      adc_unit_t unit         = ADC_UNIT_1,
                      adc_atten_t attenuation = ADC_ATTEN_DB_12)
  {
    this->_digi_patt[index] = adc_digi_pattern_config_t{
        .atten     = static_cast<uint8_t>(attenuation),
        .channel   = static_cast<uint8_t>(channel),
        .unit      = static_cast<uint8_t>(unit),
        .bit_width = static_cast<uint8_t>(bitwidth),
    };
  }
  void
  config_adc(adc_digi_convert_mode_t convert = ADC_CONV_SINGLE_UNIT_1,
             uint32_t frequency              = SAMPLE_FREQUENCY)
  {
    adc_continuous_config_t adc_config = {
        .pattern_num    = static_cast<uint32_t>(this->_adc_amount),
        .adc_pattern    = this->_digi_patt.begin(),
        .sample_freq_hz = frequency,
        .conv_mode      = convert,
        .format = adc_digi_output_format_t::ADC_DIGI_OUTPUT_FORMAT_TYPE1,
    };

    ESP_ERROR_CHECK(adc_continuous_config(*this->_handle, &adc_config));
  }

 private:
  size_t const _adc_amount         = AMOUNT_OF_ADC_SENSORS;
  adc_continuous_handle_t* _handle = nullptr;
  uint32_t _frame_size;
  uint32_t _max_buff_size;
  std::array<adc_digi_pattern_config_t, AMOUNT_OF_ADC_SENSORS> _digi_patt;
#if lut_usage
  const std::array<float, ADC_RANGE> _LUT{};
#else
  adc_cali_handle_t* _cali_handle = nullptr;
#endif
};
constexpr static size_t ADC_BUFFER_SIZE{1024};
constexpr static size_t ADC_FRAME_SIZE{256};

void
app_main()
{
  vmg_adc_driver driver =
      vmg_adc_driver(AMOUNT_OF_ADC_SENSORS, ADC_BUFFER_SIZE, ADC_FRAME_SIZE);
  driver.init_driver();

  driver.config_digi_pattern(0, ADC_CHANNEL_1);
  driver.config_digi_pattern(1, ADC_CHANNEL_3);

  driver.config_adc();

  driver.start_driver();

  std::array<uint8_t, ADC_FRAME_SIZE> buf{};
  esp_err_t ret = 0;
  uint32_t read_length{};

  while (true) {
    ret = adc_continuous_read(*driver.get_handle(), buf.begin(),
                              static_cast<uint32_t>(ADC_FRAME_SIZE),
                              &read_length, 0);
    if (ret == ESP_OK) {
      for (size_t i = 0; i < read_length; i += SOC_ADC_DIGI_RESULT_BYTES) {
        auto* p       = reinterpret_cast<adc_digi_output_data_t*>(&buf[i]);
        uint32_t data = p->type1.data;
      }
    }
  }

  driver.stop_and_deinit_driver();
}
