#include <iostream>
#include <vector>
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "hal/adc_types.h"

void app_main(){
    adc_continuous_handle_t handle = nullptr;
    adc_continuous_handle_cfg_t handle_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = 256, 
      };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&handle_config, &handle));

    adc_digi_convert_mode_t convert = ADC_CONV_SINGLE_UNIT_1;
    std::vector<adc_digi_pattern_config_t> vec;

        configs_cont.push_back();
    adc_continuous_config_t adc_config = {
        .pattern_num = 2,
        .sample_freq_hz = 44100,
        .conv_mode = convert,
        .format = adc_digi_output_format_t::ADC_DIGI_OUTPUT_FORMAT_TYPE1,
        .adc_pattern
    };
    
    ESP_ERROR_CHECK(adc_continuous_config(handle, &adc_config));
}
