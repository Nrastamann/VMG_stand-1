#include "acs758.hpp"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "hal/adc_types.h"
#include "vmg_current_driver.hpp"
#include "vmg_current.hpp"

constexpr uint32_t DEFAULT_VREF {1100};//in mV, used if eFuse values is not available

vmg_current_acs758::vmg_current_acs758(
        vmg_current_sensor& driver
        , adc1_channel_t channel
        , adc_atten_t adc_attenuation): 
    
    vmg_current_driver(driver, channel, adc_attenuation) {
        
        adc1_config_channel_atten(channel, adc_attenuation);
        adc1_config_width(ADC_WIDTH_BIT_12);

        esp_adc_cal_characteristics_t adcn_chars;

        //get callibration values
        esp_adc_cal_characterize(ADC_UNIT_1, adc_attenuation, ADC_WIDTH_BIT_12, 0, &adcn_chars);

        //need to like somehow pick the best way to pick between

    }

bool vmg_current_acs758::_probe(){
    //read some values and if they're valid = probe done right and driver is healthy   
}

void vmg_current_acs758::update(){
    //need to read data and write it to frontend    
}

void vmg_current_acs758::_read_raw(){
    
}

void vmg_current_acs758::_calculate(){

}

bool vmg_current_acs758::_data_ready(){
    //check if last reading was good, maybe need to sample data only once in a period, 
    //so need to
}
