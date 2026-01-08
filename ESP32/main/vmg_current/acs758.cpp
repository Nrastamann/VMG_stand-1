#include "acs758.hpp"

#include <stdfloat>

#include "current_backend.hpp"
#include "current_driver.hpp"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "hal/adc_types.h"
#include "vmg_i2c/i2c_driver.hpp"
#include "vmg_utility/adc_driver.hpp"

template <typename T, typename Bus, typename Derived>
bool
vmg_current_acs758<T, Bus, Derived>::probe()
{
  return _adc_instance.getData() != UINT64_MAX;
}

template <typename T, typename Bus, typename Derived>
void
vmg_current_acs758<T, Bus, Derived>::readRaw()
{
  _raw_current = _adc_instance.getData();
}

template <typename T, typename Bus, typename Derived>
void
vmg_current_acs758<T, Bus, Derived>::update()
{
  readRaw();
}

template <typename T, typename Bus, typename Derived>
void
vmg_current_acs758<T, Bus, Derived>::calculate()
{
  _has_sample   = false;
  uint64_t temp = _raw_current * DEFAULT_SENSOR_VOLTAGE / VREF;
  _current      = (temp - _zero_voltage) / _voltage_to_current;
  _has_sample   = true;
}

//=============================I2C variant
template <typename Bus, typename Derived>
bool
vmg_current_acs758<EXTERNAL_ADC_SUBSCRIBER_TAG, external_adc<Derived, Bus>,
                   Bus>::probe()
{
  return true;
}

template <typename Bus, typename Derived>
void
vmg_current_acs758<EXTERNAL_ADC_SUBSCRIBER_TAG, external_adc<Derived, Bus>,
                   Bus>::readRaw()
{
  _raw_current = _i2c_subscriber.read_data();
}

template <typename Bus, typename Derived>
void
vmg_current_acs758<EXTERNAL_ADC_SUBSCRIBER_TAG, external_adc<Derived, Bus>,
                   Bus>::update()
{
  readRaw();
}

template <typename Bus, typename Derived>
void
vmg_current_acs758<EXTERNAL_ADC_SUBSCRIBER_TAG, external_adc<Derived, Bus>,
                   Bus>::calculate()
{
  uint64_t current = 1;
  _has_sample      = true;
}
