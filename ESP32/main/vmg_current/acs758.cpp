#include "acs758.hpp"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "hal/adc_types.h"
#include "vmg_current.hpp"
#include "vmg_current_driver.hpp"
#include "vmg_utility/adc_driver.hpp"
bool
vmg_current_acs758::probe()
{
  return _adc_instance.getData() != UINT64_MAX;
}

void
vmg_current_acs758::update()
{
  readRaw();
}

void
vmg_current_acs758::readRaw()
{
  _raw_current = _adc_instance.getData();
}

void
vmg_current_acs758::calculate()
{
  uint64_t temp = _raw_current * DEFAULT_SENSOR_VOLTAGE / VREF;
  _current      = (temp - _zero_voltage) / _voltage_to_current;
  _has_sample   = true;
}

bool
vmg_current_acs758::dataReady()
{
  return _has_sample;
}
