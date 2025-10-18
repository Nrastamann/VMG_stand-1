#pragma once

#include <vector>
constexpr uint8_t IR_MAX_AMOUNT{3};
constexpr uint8_t IR_MAX_AMOUNT_DRIVERS{3};

class vmg_ir_driver;

class vmg_ir{
    public:
    void add_driver(vmg_ir_driver*);
    void update();
    void update_calibration();

    void calibrate(){calibrate(primary);}
    void calibrate(uint8_t instance);

    private:
    class sensor{
        uint32_t last_update_ms;
        uint32_t last_change_ms;

        uint32_t turn_amount;
        //maybe some additional data to calculate rpm, but i think
        //it depends on specific sensor also it depends on vmg_configuration
        //which should be separated

        bool healthy;
        bool calibrated;
    };
    
    std::vector<sensor, IR_MAX_AMOUNT> sensors;
    std::vector<vmg_ir_driver*, IR_MAX_AMOUNT_DRIVERS> drivers;

    uint8_t primary;
    uint8_t amount_sensors;
    uint8_t amount_drivers;
};
