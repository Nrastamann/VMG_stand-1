#include "ElectronicVoltageSensor.hpp"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "hal/adc_types.h"
#include "vmg_utility/adc_driver.hpp"
#include "voltage_backend.hpp"
#include "voltage_driver.hpp"
bool
vmg_electronic_voltage_sensor::probe()
{
  return _adc_instance.getData() != UINT64_MAX;
}

void
vmg_electronic_voltage_sensor::update()
{
  readRaw();
}

void
vmg_electronic_voltage_sensor::readRaw()
{
  _raw_voltage = _adc_instance.getData();
}

void
vmg_electronic_voltage_sensor::calculate()
{
  _has_sample   = false;

  uint16_t temp = _raw_voltage *
                  (_first_resistanse_adc + _second_resistanse_adc) /
                  _second_resistanse_adc;
  _voltage =
      temp * (_first_resistanse + _second_resistanse) / _second_resistanse;

  _has_sample = true;
}
