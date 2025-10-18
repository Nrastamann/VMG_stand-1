#include "vmg_voltage.hpp"

void vmg_voltage_sensor::add_driver(vmg_voltage_driver*){
    //also need to add somehow create opportunity to connect driver to sensor
    //need to check how to init config
}

void vmg_voltage_sensor::update(){
    for (uint8_t i = 0; i < amound_drivers; ++i){
        drivers[i]->update();
    }

    for (uint8_t i = 0; i < amount_sensors;++i){
        if (sensors[i].healthy){
            primary = i;
            break;
        }
    }
}

void vmg_voltage_sensor::update_calibration(){}

void vmg_voltage_sensor::calibrate(uint8_t instance){}
