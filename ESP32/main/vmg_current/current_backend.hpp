#pragma once
#include "current_driver.hpp"
#include "esp_adc_cal_types_legacy.h"
#include "hal/adc_types.h"

class vmg_current_backend {
 public:
  virtual void update()          = 0;
  virtual ~vmg_current_backend() = default;
  void copyToSensor(SensorCurrent data, uint64_t current);
};
