#pragma once

#include <array>
#include <memory>
#include <optional>

#include "current_backend.hpp"
#include "current_driver.hpp"
#include "hal/adc_types.h"
#include "vmg_external_adc/ads1115.hpp"
#include "vmg_external_adc/external_adc_driver.hpp"
#include "vmg_i2c/i2c_driver.hpp"
#include "vmg_utility/adc_driver.hpp"

uint32_t constexpr DEFAULT_REFERENCE_VOLTAGE{2380};

constexpr uint32_t DEFAULT_VREF{1100};
// in mV, used if eFuse values is not available

constexpr uint32_t DEFAULT_SENSOR_VOLTAGE{5000};
constexpr uint8_t DEFAULT_VOLTAGE_TO_CURRENT{20};
constexpr uint8_t DEFAULT_MAX_CURRENT{100};

template <typename T = ADC_DRIVER_SUBSCRIBER_TAG, typename Bus = adc_subscriber,
          typename Derived = std::nullopt_t>
class vmg_current_acs758 : public vmg_current_backend {
 public:
  vmg_current_acs758(adc_channel_t channel, adc_dma_storage* storage,
                     uint16_t zero_voltage      = DEFAULT_REFERENCE_VOLTAGE,
                     uint8_t voltage_to_current = DEFAULT_VOLTAGE_TO_CURRENT) :
      _adc_instance(channel, storage),
      _zero_voltage(zero_voltage),
      _voltage_to_current(voltage_to_current)
  {
  }

  vmg_current_acs758() = delete;
  void update() final;
  bool healthy();
  void
  setAdcSubscription(Bus&& adc_instance)
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

  uint64_t _current     = 0;
  Bus _adc_instance;
  uint16_t _zero_voltage;
  uint8_t _voltage_to_current = DEFAULT_VOLTAGE_TO_CURRENT;
};

template <typename Bus, typename Derived>
class vmg_current_acs758<EXTERNAL_ADC_SUBSCRIBER_TAG,
                         external_adc<Derived, Bus>, Bus>
    : public vmg_current_backend {
 public:
  vmg_current_acs758() = default;
  void update() final;
  bool healthy();

  void
  setExternalAdcInstance(external_adc<Derived, Bus>* adc)
  {
    _external_adc = std::make_unique<external_adc<Derived, Bus>*>(adc);
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

  bool _has_sample                              = false;
  std::array<uint32_t, MULTISAMPLING_RATE> _raw = 0;

  uint8_t _multisample_acquired                 = 0;
  uint64_t _current_raw                         = 0;
  uint64_t _current                             = 0;
  std::unique_ptr<external_adc<Derived, Bus>*> _external_adc;
};
