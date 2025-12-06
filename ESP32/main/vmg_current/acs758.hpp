#pragma once

#include <array>

#include "driver/adc_types_legacy.h"
#include "hal/adc_types.h"
#include "vmg_current_driver.hpp"
#include "vmg_utility/adc_driver.hpp"
uint8_t constexpr MULTISAMPLE_AMOUNT = 10;
float constexpr REFERENCE_VOLTAGE    = 2.38;
class vmg_current_acs758 : public vmg_current_driver {
 public:
  vmg_current_acs758(vmg_current_sensor& driver, adc1_channel_t channel,
                     adc_atten_t adc_attenuation);

  void update() override;
  void
  setAdcSubscription(adc_subscriber&& adc_instance)
  {
    this->_adc_instance = std::move(adc_instance);
  }

 private:
  bool probe();

  void readRaw();
  void calculate();
  bool dataReady();

  bool _has_sample;
  std::array<uint32_t, MULTISAMPLE_AMOUNT> _raw_readings;
  uint32_t _raw_current;
  float _ref_voltage = REFERENCE_VOLTAGE;
  uint32_t _current;
  adc_subscriber _adc_instance;
};
