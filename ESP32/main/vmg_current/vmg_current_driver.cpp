#include "vmg_current_driver.hpp"

void
vmg_current_backend::copyToSensor(Sensor data, uint64_t current)
{
  // data._last_update_ms = current_time;
  // data.healthy?
  //
  if (data._current != current) {
    // data._last_update_ms = current_time;
    data._current = current;
  }
}
