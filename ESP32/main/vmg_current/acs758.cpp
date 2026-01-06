#include "acs758.hpp"

#include "current_backend.hpp"
#include "current_driver.hpp"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "hal/adc_types.h"
#include "vmg_i2c/i2c_driver.hpp"
#include "vmg_utility/adc_driver.hpp"

template <typename T, size_t Recv, size_t Send>
bool
vmg_current_acs758<T, Recv, Send>::probe()
{
  return _adc_instance.getData() != UINT64_MAX;
}

template <typename T, size_t Recv, size_t Send>
void
vmg_current_acs758<T, Recv, Send>::readRaw()
{
  _raw_current = _adc_instance.getData();
}

template <typename T, size_t Recv, size_t Send>
void
vmg_current_acs758<T, Recv, Send>::update()
{
  readRaw();
}

template <typename T, size_t Recv, size_t Send>
void
vmg_current_acs758<T, Recv, Send>::calculate()
{
  _has_sample   = false;
  uint64_t temp = _raw_current * DEFAULT_SENSOR_VOLTAGE / VREF;
  _current      = (temp - _zero_voltage) / _voltage_to_current;
  _has_sample   = true;
}

//=============================I2C variant
template <size_t Recv, size_t Send>
bool
vmg_current_acs758<I2C_DRIVER_SUBSCRIBER_TAG, Recv, Send>::probe()
{
  _i2c_subscriber.probe();
  return true;
}

template <size_t Recv, size_t Send>
void
vmg_current_acs758<I2C_DRIVER_SUBSCRIBER_TAG, Recv, Send>::readRaw()
{
  _raw_current = _i2c_subscriber.read_data();
}

template <size_t Recv, size_t Send>
void
vmg_current_acs758<I2C_DRIVER_SUBSCRIBER_TAG, Recv, Send>::update()
{
  readRaw();
}

template <size_t Recv, size_t Send>
void
vmg_current_acs758<I2C_DRIVER_SUBSCRIBER_TAG, Recv, Send>::calculate()
{
  _has_sample   = false;
  uint64_t temp = _raw_current * DEFAULT_SENSOR_VOLTAGE / VREF;
  _current      = (temp - _zero_voltage) / _voltage_to_current;
  _has_sample   = true;
}
