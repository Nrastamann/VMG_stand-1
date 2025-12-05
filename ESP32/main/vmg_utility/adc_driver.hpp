#pragma once

#include <array>
#include <memory>

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "hal/adc_types.h"
#include "soc/soc_caps.h"

constexpr bool lut_usage{false};
constexpr bool curve_fitting{true};
constexpr bool NO_DEFAULT_VREF{false};

constexpr size_t AMOUNT_OF_ADC_SENSORS{2};
constexpr size_t SAMPLE_FREQUENCY{20000};
constexpr size_t ADC_RANGE{4096};

constexpr uint8_t MULTISAMPLING_RATE{10};
inline extern constexpr size_t ADC_FRAME_SIZE{SOC_ADC_DIGI_RESULT_BYTES *
                                              MULTISAMPLING_RATE};
inline extern constexpr size_t ADC_BUFFER_SIZE{ADC_FRAME_SIZE * 4};

class adc_subscriber;

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
  float get_lut(size_t reading);
#else
  void calibration_init();
  void calibration_deinit();
#endif
  void stop_and_deinit_driver();
  void init_driver();

  void config_digi_pattern(size_t index, adc_channel_t channel, adc_unit_t unit,
                           adc_bitwidth_t bitwidth, adc_atten_t attenuation);
  void setup_digi_pattern();
  void config_adc(adc_digi_convert_mode_t convert = ADC_CONV_SINGLE_UNIT_1,
                  uint32_t frequency              = SAMPLE_FREQUENCY);

  void
  config_digi_pattern(size_t index, adc_channel_t channel)
  {
    this->_digi_patt[index].channel = channel;
  }
  void
  set_attenuation(adc_atten_t attenuation = ADC_ATTEN_DB_12)
  {
    this->_attenuation = attenuation;
  }
  void
  set_bitwidth(adc_bitwidth_t bitwidth = ADC_BITWIDTH_12)
  {
    this->_bitwidth = bitwidth;
  }
  void
  set_unit(adc_unit_t unit = ADC_UNIT_1)
  {
    this->_unit = unit;
  }
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

  int get_data(adc_subscriber& subscriber);

  size_t
  get_channel_index(adc_channel_t channel_num)
  {
    size_t index = 0;
    for (auto const i : this->_digi_patt) {
      if (i.channel == channel_num) {
        return index;
      }
      index++;
    }
  }

 private:
  size_t _adc_amount               = AMOUNT_OF_ADC_SENSORS;
  adc_continuous_handle_t* _handle = nullptr;
  uint32_t _frame_size;
  uint32_t _max_buff_size;
  std::array<adc_digi_pattern_config_t, AMOUNT_OF_ADC_SENSORS> _digi_patt;
  adc_atten_t _attenuation = ADC_ATTEN_DB_0;
  adc_unit_t _unit         = ADC_UNIT_1;
  adc_bitwidth_t _bitwidth = ADC_BITWIDTH_12;

#if lut_usage
  const std::array<float, ADC_RANGE> _LUT{};
#else
  adc_cali_handle_t* _cali_handle = nullptr;
#endif
};

class adc_subscriber {
 public:
  void
  setRef(vmg_adc_driver* driver)
  {
    this->adc = driver;
  }
  vmg_adc_driver*
  getRef()
  {
    return *this->adc;
  }

  uint64_t getData();
  std::array<uint8_t, ADC_FRAME_SIZE>& get_buffer();

 private:
  std::shared_ptr<vmg_adc_driver*> adc = new vmg_adc_driver*;
  std::array<uint8_t, ADC_FRAME_SIZE> buffer;
};
