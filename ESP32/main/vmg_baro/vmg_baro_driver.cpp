#include "vmg_baro_driver.hpp"

void vmg_baro_driver::_copy_to_sensor(uint8_t instance, float pressure, float temperature){
    if (instance >= vmg_barometer.amount_sensors) return;
    //timestamp

    vmg_barometer.baro_physical[instance].pressure = pressure;
    vmg_barometer.baro_physical[instance].temperature = temperature;
    //add time update, millis or use freertos 
}
