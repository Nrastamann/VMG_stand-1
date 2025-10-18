#include "vmg_ir.hpp"
#include <cstdint>

void vmg_ir::add_driver(vmg_ir_driver* driver){
    //i suppose you can't really diff them, so need to add pick of this type of sensor
    //to settings, maybe some default value driver on start and later on you 
    //download configuration, maybe store somewhere on EEPROM, idk
}

void vmg_ir::calibrate(){
    //i guess, in the beginning it need to start motors and check if 
    //all sensors are calibrated and healthy, also can use this data for calibration?
}

[[maybe_unused]] void vmg_ir::update_calibration(uint8_t instance){
    //i don't think we need this, but for consistency i added it there,
    //maybe unused
}

void vmg_ir::update(){
    for(uint8_t i = 0; i < amount_drivers; ++i){
        drivers[i]->update();
    }
    
    for(uint8_t i = 0; i < amount_sensors; ++i){
        if(sensors[i].healthy){
            primary = i;//maybe need to store driver number in sensor to identify?
            break;
        }
    }
}


