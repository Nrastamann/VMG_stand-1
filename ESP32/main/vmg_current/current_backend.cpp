#include "current_backend.hpp"

void
vmg_current_backend::copyToSensor(uint64_t current)
{
  // data._last_update_ms = current_time;
  // data.healthy?
  //
  if (_data._current != current) {
    // data._last_update_ms = current_time;
    _data._current = current;
  }
}
void
vmg_current_backend::updateHealth()
{
  // check if values are correct and
  // if there was update recently
}
void
vmg_current_backend::backendUpdate()
{
  update();
  updateHealth();
}
