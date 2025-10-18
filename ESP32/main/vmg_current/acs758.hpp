#pragma once

#include "driver/adc_types_legacy.h"
#include "hal/adc_types.h"
#include "vmg_current_driver.hpp"
#include <array>
uint8_t constexpr MULTISAMPLE_AMOUNT = 10;
float constexpr REFERENCE_VOLTAGE = 2.38;
class vmg_current_acs758 : public vmg_current_driver{
public:
    vmg_current_acs758(vmg_current_sensor& driver, adc1_channel_t channel, adc_atten_t adc_attenuation);

    void update() override;
    
private:
    bool _probe();
    
    void _read_raw();
    void _calculate();
    bool _data_ready();
    
    bool _has_sample;
    std::array <uint32_t,MULTISAMPLE_AMOUNT> _raw_readings;
    uint32_t _raw_current;
    float _ref_voltage = REFERENCE_VOLTAGE;
    uint32_t _current;
};
