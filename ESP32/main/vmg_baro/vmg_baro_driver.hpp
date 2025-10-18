#pragma once
#include <vmg_baro.hpp>

class vmg_baro_driver{
    public:
    virtual void update() = 0;//interface for sensors drivers 
    virtual ~vmg_baro_driver(){}

    vmg_baro_driver(vmg_baro& frontend): baro_front(frontend){}
    
    void _copy_to_sensor(uint8_t instance);

    private:
    
    vmg_baro& baro_front;
    
};
