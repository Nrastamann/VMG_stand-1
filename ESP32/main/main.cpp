#include <memory.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>

#include "adc_reading.hpp"
#include "hal/adc_types.h"
#include "vmg_utility/adc_driver.hpp"
void
app_main()
{
  vmg_adc_driver driver = {AMOUNT_OF_ADC_SENSORS, ADC_BUFFER_SIZE,
                           ADC_FRAME_SIZE};
  adc_subscriber adc_ref;
  adc_ref.setRef(&driver);

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
  uint64_t avg_voltage;
  while (true) {
    avg_voltage = adc_ref.getData(ADC_CHANNEL_1);
    std::cout << avg_voltage << '\n';
  }

  driver.calibration_deinit();
  driver.stop_and_deinit_driver();
}
