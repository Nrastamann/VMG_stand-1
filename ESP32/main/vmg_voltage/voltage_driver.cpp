#include "voltage_driver.hpp"

void
vmg_voltage_driver::update()
{
  for (uint8_t i = 0; i < _amount_drivers; ++i) {
    (*_drivers[i])->update();
  }

  for (uint8_t i = 0; i < _amount_sensors; ++i) {
    if (*_drivers[i].healthy) {
      _primary = i;
      break;
    }
  }
}
