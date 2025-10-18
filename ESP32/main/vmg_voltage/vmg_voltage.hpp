#pragma once

#include <array>
constexpr uint8_t MULTISAMPLING_RATE{10};
constexpr uint8_t VOLTAGE_MAX_AMOUNT{3};
constexpr uint8_t VOLTAGE_MAX_AMOUNT_DRIVERS{3};

class vmg_voltage_driver;

class vmg_voltage_sensor{
    public:
    void add_driver(vmg_voltage_driver*);
    void update();
    void update_calibration();

    void calibrate(){calibrate(primary);}
    void calibrate(uint8_t instance);

    private:
    class sensor{
        std::vector<uint16_t, MULTISAMPLING_RATE> raw_data;
        
        uint32_t last_update_ms;
        uint32_T last_change_ms;

        float voltage;

        bool healthy;
        bool calibrated;
    };

    std::array<sensor, VOLTAGE_MAX_AMOUNT> sensors;
    std::array<vmg_voltage_driver*, VOLTAGE_MAX_AMOUNT_DRIVERS> drivers;

    uint8_t primary;
    uint8_t amount_sensors;
    uint8_t amound_drivers;
};
