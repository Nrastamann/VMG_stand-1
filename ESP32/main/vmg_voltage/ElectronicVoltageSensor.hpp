#pragma once

#include <array>

#include "hal/adc_types.h"
#include "vmg_utility/adc_driver.hpp"
#include "voltage_backend.hpp"
#include "voltage_driver.hpp"

static uint8_t constexpr MULTISAMPLE_AMOUNT{10};
static uint32_t constexpr DEFAULT_REFERENCE_VOLTAGE{2380};

constexpr uint32_t DEFAULT_VREF{1100};
// in mV, used if eFuse values is not available

constexpr uint32_t DEFAULT_SENSOR_VOLTAGE{5000};
constexpr uint16_t DEFAULT_MAX_CURRENT{25000};

class vmg_electronic_voltage_sensor : public vmg_voltage_backend {
 public:
  vmg_electronic_voltage_sensor(
      adc_channel_t channel, adc_dma_storage* storage,
      uint8_t max_voltage        = DEFAULT_MAX_CURRENT,
      uint16_t zero_voltage      = DEFAULT_REFERENCE_VOLTAGE,
      uint8_t voltage_to_current = DEFAULT_VOLTAGE_TO_CURRENT) :
      _adc_instance(channel, storage),
      _max_current(max_current),
      _zero_voltage(zero_voltage),
      _voltage_to_current(voltage_to_current)
  {
  }

  vmg_electronic_voltage_sensor() = delete;
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
  uint64_t _raw_voltage = 0;
  float _ref_voltage;

  uint64_t _voltage = 0;
  adc_subscriber _adc_instance;
  uint8_t _max_current;
  uint16_t _zero_voltage;
};
