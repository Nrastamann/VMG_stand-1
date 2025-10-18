#pragma once
#include "esp_adc_cal_types_legacy.h"
#include "hal/adc_types.h"
#include "vmg_current.hpp"
#include "esp_adc_cal.h"

/*
enum class adc_type_t{
    ADC1,
    ADC2
};
*/

class vmg_current_driver{
    public:
    virtual void update() = 0;

    virtual ~vmg_current_driver(){}

    vmg_current_driver(vmg_current_sensor& frontend, adc1_channel_t channel, adc_atten_t adc_attenuation): 
 //       adc_number(adc_number), reason - potential usage, not now
        adc_attenuation(adc_attenuation),
        adc_channel(channel),
        current_front(frontend)
    {}
    
    void _copy_to_sensor(uint8_t instance);
    
    private:

//    const adc_type_t adc_number;
    adc_atten_t adc_attenuation;
    adc1_channel_t adc_channel;
    esp_adc_cal_characteristics_t calibration_data;        
    vmg_current_sensor& current_front;
};
