#include "voltage_backend.hpp"

void
vmg_voltage_backend::copyToSensor(SensorVoltage data, uint64_t voltage)
{
  // data._last_update_ms = current_time;
  // data.healthy?
  //
  if (data._voltage != voltage) {
    // data._last_update_ms = current_time;
    data._voltage = voltage;
  }
}
