#pragma once

#include <array>
#include <cstdlib>

#include "vmg_utility/adc_driver.hpp"
#include "vmg_voltage/voltage_driver.hpp"

static constexpr uint8_t VOLTAGE_MAX_AMOUNT{3};
static constexpr uint8_t VOLTAGE_MAX_AMOUNT_DRIVERS{3};

class vmg_voltage_backend;

struct SensorVoltage {
  uint32_t _last_update_ms;
  uint32_t _last_change_ms;

  uint64_t _voltage;

  bool _healthy;
  bool _calibrated;
};

class vmg_voltage_driver {
 public:
  void update();
  // void updateCallibration();

  void
  calibrate()
  {
    calibrate(_primary);
  };

  void
  addBackend(vmg_voltage_backend* ptr)
  {
    _drivers.push_back(std::make_unique<vmg_voltage_backend*>(ptr));
  }

  void
  calibrate(uint8_t instance)
  {
    (*_drivers[instance])->calibrate();
  }

  uint64_t
  getPrimaryCurrent()
  {
    return _sensors[_primary]._voltage;
  }

 private:
  std::array<SensorVoltage, VOLTAGE_MAX_AMOUNT> _sensors;
  std::array<std::unique_ptr<vmg_voltage_backend*>, VOLTAGE_MAX_AMOUNT_DRIVERS>
      _drivers;

  uint8_t _primary;
  uint8_t _amount_sensors;
  uint8_t _amount_drivers;
};
