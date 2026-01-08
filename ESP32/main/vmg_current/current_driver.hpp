#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "esp_log.h"

class vmg_current_backend;

static constexpr uint8_t MULTISAMPLING_RATE{10};
static constexpr uint8_t CURRENT_MAX_AMOUNT{3};
static constexpr uint8_t CURRENT_MAX_AMOUNT_DRIVERS{3};

struct SensorCurrent {
  uint32_t _last_update_ms;
  uint32_t _last_change_ms;

  uint64_t _current;

  bool _healthy;
  bool _calibrated;
};

class vmg_current_driver {
 public:
  void update();
  // void updateCallibration();

  void
  calibrate()
  {
    calibrate(_primary);
  };

  void
  addBackend(vmg_current_backend* ptr)
  {
    if (_amount_sensors == CURRENT_MAX_AMOUNT) {
      ESP_LOGE("Current frontend", "Too much current drivers, need to clear");
      return;
    }
    _drivers[_amount_sensors++] = std::make_unique<vmg_current_backend*>(ptr);
  }

  void
  calibrate(uint8_t instance)
  {
    (*_drivers[instance])->calibrate();
  }

  uint64_t
  getPrimaryCurrent()
  {
    return _sensors[_primary]._current;
  }

 private:
  std::array<SensorCurrent, CURRENT_MAX_AMOUNT> _sensors;
  std::array<std::unique_ptr<vmg_current_backend*>, CURRENT_MAX_AMOUNT_DRIVERS>
      _drivers;

  uint8_t _primary;
  uint8_t _amount_sensors = 0;
  uint8_t _amount_drivers = 0;
};
