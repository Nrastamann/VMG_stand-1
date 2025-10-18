#pragma once
#include "vmg_voltage.hpp"

class vmg_voltage_driver{
    public:
    virtual void update() = 0;
    virtual ~vmg_voltage_driver(){}

    vmg_voltage_driver(vmg_voltage_sensor& frontend): voltage_front(frontend){}
    void copy_to_sensor(uint8_t instance);

    private:
    vmg_voltage_sensor& voltage_front;
};

