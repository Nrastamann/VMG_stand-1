#include <memory.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>

#include "adc_reading.hpp"
#include "hal/adc_types.h"
#include "vmg_current/acs758.hpp"
#include "vmg_utility/adc_driver.hpp"
void
app_main()
{
  adc_dma_storage storage;
  vmg_adc_driver driver  = {AMOUNT_OF_ADC_SENSORS, ADC_BUFFER_SIZE,
                            ADC_FRAME_SIZE};

  adc_subscriber adc_ref = adc_subscriber(ADC_CHANNEL_1, &storage);

  driver.init_driver();
  driver.calibration_init();
  vmg_current_acs758 sensor_current =
      vmg_current_acs758(ADC_CHANNEL_1, &storage);
  driver.config_digi_pattern(0, ADC_CHANNEL_1);
  driver.config_digi_pattern(1, ADC_CHANNEL_3);

  driver.set_attenuation();
  driver.set_bitwidth();
  driver.set_unit();
  driver.setup_digi_pattern();

  driver.config_adc();

  driver.start_driver();
  uint64_t avg_voltage = 0;
  while (true) {
    avg_voltage = adc_ref.getData();
    std::cout << avg_voltage << '\n';
  }

  driver.calibration_deinit();
  driver.stop_and_deinit_driver();
}
