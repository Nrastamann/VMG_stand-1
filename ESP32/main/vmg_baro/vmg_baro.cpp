#include <vmg_baro.hpp>

void
vmg_baro::update()
{
  for (auto& driver : baro_drivers) {
    driver->update();
  }

  for (uint8_t i = 0; i < amount_sensors; ++i) {
    if (baro_physical[i].healthy) {
      primary = i;
      break;
    }
  }
}

void
vmg_baro::add_baro_driver(vmg_baro_driver* driver)
{
  if (!driver) {
    return;
  }

  if (amount_drivers >= BARO_DRIVERS_AMOUNT) {
    // Panic, too many drivers
  }
  baro_drivers[amount_drivers++] = driver;
  return;
}
