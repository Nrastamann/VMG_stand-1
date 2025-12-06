#pragma once

#include <array>

#include "driver/adc_types_legacy.h"
#include "hal/adc_types.h"
#include "vmg_current_driver.hpp"
#include "vmg_utility/adc_driver.hpp"

uint8_t constexpr MULTISAMPLE_AMOUNT{10};
uint32_t constexpr REFERENCE_VOLTAGE{2380};

constexpr uint32_t DEFAULT_VREF{1100};
// in mV, used if eFuse values is not available

constexpr uint32_t DEFAULT_SENSOR_VOLTAGE{5000};
constexpr uint8_t DEFAULT_VOLTAGE_TO_CURRENT{20};

class vmg_current_acs758 : public vmg_current_backend {
 public:
  vmg_current_acs758(adc1_channel_t channel, uint8_t max_voltage,
                     uint16_t zero_voltage, uint8_t voltage_to_current) :
      _adc_instance(channel),
      _max_voltage(max_voltage),
      _zero_voltage(zero_voltage),
      _voltage_to_current(voltage_to_current)
  {
  }

  void update() final;
  void init() final;
  bool healthy();
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

  bool _has_sample = false;
  std::array<uint32_t, MULTISAMPLE_AMOUNT> _raw_readings;
  uint64_t _raw_current;
  float _ref_voltage = REFERENCE_VOLTAGE;

  uint64_t _current;
  adc_subscriber _adc_instance;
  uint8_t _max_voltage;
  uint16_t _zero_voltage      = REFERENCE_VOLTAGE;
  uint8_t _voltage_to_current = DEFAULT_VOLTAGE_TO_CURRENT;
};
