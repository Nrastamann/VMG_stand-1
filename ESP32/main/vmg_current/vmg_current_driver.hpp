#pragma once
#include "esp_adc_cal_types_legacy.h"
#include "hal/adc_types.h"
#include "vmg_current.hpp"

class vmg_current_backend {
 public:
  virtual void update()          = 0;

  virtual ~vmg_current_backend() = default;

  static void copyToSensor(Sensor data, uint64_t current);
};
