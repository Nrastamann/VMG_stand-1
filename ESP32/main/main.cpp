#include <memory.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <cstdio>

#include "adc_reading.hpp"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "hal/adc_types.h"

constexpr static size_t ADC_BUFFER_SIZE{1024};
constexpr static size_t ADC_FRAME_SIZE{256};

void
app_main()
{
  vmg_adc_driver driver =
      vmg_adc_driver(AMOUNT_OF_ADC_SENSORS, ADC_BUFFER_SIZE, ADC_FRAME_SIZE);
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

  std::array<uint8_t, ADC_FRAME_SIZE> buf{};
  esp_err_t ret = 0;
  uint32_t read_length{};

  while (true) {
    ret = adc_continuous_read(*driver.get_handle(), buf.begin(),
                              static_cast<uint32_t>(ADC_FRAME_SIZE),
                              &read_length, 0);
    if (ret == ESP_OK) {
      for (size_t i = 0; i < read_length; i += SOC_ADC_DIGI_RESULT_BYTES) {
        auto* p       = reinterpret_cast<adc_digi_output_data_t*>(&buf[i]);
        uint32_t data = p->type1.data;
      }
    }
  }

  driver.calibration_deinit();
  driver.stop_and_deinit_driver();
}
