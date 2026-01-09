#pragma once
#include "current_driver.hpp"
#include "esp_adc_cal_types_legacy.h"
#include "hal/adc_types.h"

// need to call probe before starting, re-design this shit
class vmg_current_backend {
 public:
  vmg_current_backend(SensorCurrent& data) : _data(data) {}
  void backendUpdate();  // call update and health
 protected:
  virtual void update()          = 0;  // update data inside backend
  virtual ~vmg_current_backend() = default;
  void copyToSensor(uint64_t current);  // copy data to sensor
  void updateHealth();                  // update health flag
  SensorCurrent& _data;
};
