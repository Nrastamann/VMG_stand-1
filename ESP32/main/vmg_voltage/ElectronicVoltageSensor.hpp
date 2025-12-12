#pragma once

#include <array>

#include "hal/adc_types.h"
#include "vmg_utility/adc_driver.hpp"
#include "voltage_backend.hpp"
#include "voltage_driver.hpp"

constexpr uint16_t DEFAULT_R1{30000};
constexpr uint16_t DEFAULT_R2{7500};

constexpr uint16_t DEFAULT_R1_SECOND{110};
constexpr uint16_t DEFAULT_R2_SECOND{31};

constexpr uint16_t REFERENCE_VOLTAGE{5000};

constexpr uint16_t DEFAULT_MAX_VOLTAGE{25000};

class vmg_electronic_voltage_sensor : public vmg_voltage_backend {
 public:
  vmg_electronic_voltage_sensor(adc_channel_t channel, adc_dma_storage* storage,
                                uint16_t r1        = DEFAULT_R1,
                                uint16_t r2        = DEFAULT_R2,
                                uint16_t second_r1 = DEFAULT_R1_SECOND,
                                uint16_t second_r2 = DEFAULT_R2_SECOND) :
      _adc_instance(channel, storage),
      _first_resistanse(r1),
      _second_resistanse(r2),
      _first_resistanse_adc(second_r1),
      _second_resistanse_adc(second_r2)
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
  uint16_t _raw_voltage = 0;

  uint16_t _voltage     = 0;
  adc_subscriber _adc_instance;

  uint16_t _first_resistanse;
  uint16_t _second_resistanse;
  uint16_t _first_resistanse_adc;
  uint16_t _second_resistanse_adc;
};
