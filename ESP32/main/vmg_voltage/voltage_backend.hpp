#pragma once
#include "esp_adc_cal_types_legacy.h"
#include "hal/adc_types.h"
#include "voltage_driver.hpp"

class vmg_voltage_backend {
 public:
  virtual void update()          = 0;
  virtual ~vmg_voltage_backend() = default;
  void copyToSensor(SensorVoltage data, uint64_t voltage);
};
