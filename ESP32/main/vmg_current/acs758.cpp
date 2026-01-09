#include "acs758.hpp"

#include <bits/stdc++.h>

#include <numeric>
#include <stdfloat>

#include "vmg_external_adc/ads1115.hpp"
#include "vmg_external_adc/external_adc_driver.hpp"
#include "vmg_utility/adc_driver.hpp"

template <typename T, typename Bus, typename Derived>
bool
vmg_current_acs758<T, Bus, Derived>::probe()
{
  return _adc_instance.getData() != UINT64_MAX;
}

template <typename T, typename Bus, typename Derived>
void
vmg_current_acs758<T, Bus, Derived>::update()
{
  _raw_current = _adc_instance.getData();
  calculate();
  copyToSensor(_current);
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
                   Bus>::init(std::shared_ptr<external_adc<Derived, Bus>> const&
                                  adc,
                              uint16_t mux)
{
  _external_adc = {adc};

  _external_adc.get().startADCReading(mux, /*continuous=*/true);
}

template <typename Bus, typename Derived>
void
vmg_current_acs758<EXTERNAL_ADC_SUBSCRIBER_TAG, external_adc<Derived, Bus>,
                   Bus>::update()
{
  _has_sample                                   = false;

  _raw[(_current_raw + 1) % MULTISAMPLING_RATE] = static_cast<uint32_t>(
      static_cast<float32_t>(_external_adc.get().computeVolts(
          _external_adc.get().getLastConversion())) *
      1000.0f32);

  _multisample_acquired = _multisample_acquired == MULTISAMPLING_RATE
                              ? MULTISAMPLING_RATE
                              : _multisample_acquired + 1;
  calculate();
  copyToSensor(_current);
}

template <typename Bus, typename Derived>
void
vmg_current_acs758<EXTERNAL_ADC_SUBSCRIBER_TAG, external_adc<Derived, Bus>,
                   Bus>::calculate()
{
  _current = std::accumulate(_raw.begin(), _raw.end(), 0) / MULTISAMPLING_RATE;
  _has_sample = true;
}
