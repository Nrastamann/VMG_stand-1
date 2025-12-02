#pragma once

class vmg_current_driver;

constexpr uint8_t MULTISAMPLING_RATE{10};
constexpr uint8_t CURRENT_MAX_AMOUNT{3};
constexpr uint8_t CURRENT_MAX_AMOUNT_DRIVERS{3};

class vmg_current_sensor {
 public:
  void add_driver(vmg_current_driver*);
  void update();
  void update_callibration();

  void
  calibrate()
  {
    calibrate(primary);
  };
  void calibrate(uint8_t instance);

 private:
  class sensor {
    std::vector<uint16_t, MULTISAMPLING_RATE> raw_data;

    uint32_t last_update_ms;
    uint32_t last_change_ms;

    float current;

    bool healthy;
    bool calibrated;
  };

  std::array<sensor, CURRENT_MAX_AMOUNT> sensors;
  std::array<vmg_current_driver*, CURRENT_MAX_AMOUNT_DRIVERS> drivers;

  uint8_t primary;
  uint8_t amount_sensors;
  uint8_t amount_drivers;
};
