#pragma once

#include <array>

#include "current_backend.hpp"
#include "current_driver.hpp"
#include "hal/adc_types.h"
#include "vmg_utility/adc_driver.hpp"

uint32_t constexpr DEFAULT_REFERENCE_VOLTAGE{2380};

constexpr uint32_t DEFAULT_VREF{1100};
// in mV, used if eFuse values is not available

constexpr uint32_t DEFAULT_SENSOR_VOLTAGE{5000};
constexpr uint8_t DEFAULT_VOLTAGE_TO_CURRENT{20};
constexpr uint8_t DEFAULT_MAX_CURRENT{100};

class vmg_current_acs758 : public vmg_current_backend {
 public:
  vmg_current_acs758(adc_channel_t channel, adc_dma_storage* storage,
                     uint8_t max_current        = DEFAULT_MAX_CURRENT,
                     uint16_t zero_voltage      = DEFAULT_REFERENCE_VOLTAGE,
                     uint8_t voltage_to_current = DEFAULT_VOLTAGE_TO_CURRENT) :
      _adc_instance(channel, storage),
      _max_current(max_current),
      _zero_voltage(zero_voltage),
      _voltage_to_current(voltage_to_current)
  {
  }

  vmg_current_acs758() = delete;
  void update() final;
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
  [[nodiscard]] bool
  dataReady() const
  {
    return _has_sample;
  };

  bool _has_sample      = false;
  uint64_t _raw_current = 0;
  float _ref_voltage;

  uint64_t _current = 0;
  adc_subscriber _adc_instance;
  uint8_t _max_current;
  uint16_t _zero_voltage;
  uint8_t _voltage_to_current = DEFAULT_VOLTAGE_TO_CURRENT;
};
