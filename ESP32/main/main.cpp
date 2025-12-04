#include <memory.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>

#include "adc_reading.hpp"
#include "driver/adc.h"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "hal/adc_types.h"
#include "vmg_utility/adc_driver.hpp"
void
app_main()
{
  vmg_adc_driver driver = {AMOUNT_OF_ADC_SENSORS, ADC_BUFFER_SIZE,
                           ADC_FRAME_SIZE};
  adc_subscriber adc_ref;
  adc_ref.set_ref(&driver);

  driver.init_driver();
  driver.calibration_init();

  driver.config_digi_pattern(0, ADC_CHANNEL_1);
  driver.config_digi_pattern(1, ADC_CHANNEL_3);

  driver.set_attenuation();
  driver.set_bitwidth();
  driver.set_unit();
  driver.setup_digi_pattern();

  driver.config_adc();

  driver.start_driver();

  while (true) {
    if (size_t res = adc_ref.get_data(); res != -1) {
      for (size_t i = 0; i < res; i += SOC_ADC_DIGI_RESULT_BYTES) {
        auto* p =
            reinterpret_cast<adc_digi_output_data_t*>(&adc_ref.get_buffer()[i]);
      }
    }
  }

  driver.calibration_deinit();
  driver.stop_and_deinit_driver();
}
