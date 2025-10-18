#include "vmg_current.hpp"

//calling update to all sensor drivers
//also picking healthy one to be primary
//also need to add check_health() or smth to run in update
void vmg_current_sensor::update(){
    for(uint8_t i = 0; i < amount_drivers; ++i){
        drivers[i]->update();
    }

    for(uint8_t i = 0; i < amount_drivers; ++i){
        if(sensors[i].healthy){
            primary = i;
            break;
        }
    }
}

void vmg_current_sensor::calibrate(uint8_t instance){
   //посмотреть, как калибруют датчики тока
   //Подумать что сюда можно передать
}

//more like runtime calibration during work, like temperature issues and other,
//need to check how to do properly
void vmg_current_sensor::update_calibration(){
    //Посмотреть как во время работы сделать чтобы данные не плыли, ну и 
    //при больших токах и температуре
}

void vmg_current_sensor::add_driver(vmg_current_driver* driver){
    //Подумать как присоединить драйвер к датчику, т.к. здесь не i2c,
    //вероятно имеет смысл попробовать вызвать healthy/calibrate/еще что-то
}
