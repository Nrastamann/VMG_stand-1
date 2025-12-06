#include "vmg_current.hpp"

#include <memory>

// calling update to all sensor drivers
// also picking healthy one to be primary
// also need to add check_health() or smth to run in update
void
vmg_current_driver::update()
{
  for (uint8_t i = 0; i < _amount_drivers; ++i) {
    (*_drivers[i])->update();
  }

  for (uint8_t i = 0; i < _amount_drivers; ++i) {
    if (_sensors[i].healthy) {
      _primary = i;
      break;
    }
  }
}

// more like runtime calibration during work, like temperature issues and other,
// need to check how to do properly
/*
void
vmg_current_driver::updateCalibration()
{
  // какие-то фильтры, по типу калмана, не знаю
  //  Посмотреть как во время работы сделать чтобы данные не плыли, ну и
  //  при больших токах и температуре
}
*/
