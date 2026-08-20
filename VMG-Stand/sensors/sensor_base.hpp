#pragma once

#include <cassert>
#include <cstdint>

#include "../HAL/HAL.hpp"
namespace vmg_system::sensors {
  static constexpr char const*
  busToStr(BusType type)
  {
    switch (type) {
      case BusType::NONE:
        return "None";
      case BusType::I2C:
        return "I2C";
      case BusType::SPI:
        return "SPI";
      case BusType::UART:
        return "UART";
      case BusType::TIMER:
        return "Timer";
      case BusType::INTERRUPT:
        return "Interrupt";
      default:
        std::unreachable();
    }
  }

  template <BusType SingleBus>
  class SensorBase {
   public:
    explicit SensorBase(size_t sensor_id) : _sensor_id(sensor_id) {}

    void
    setBusType(BusType new_type)
    {
      _bus_type = new_type;
    }
    BusType
    getBusType()
    {
      return _bus_type;
    }
    size_t
    getID()
    {
      return _sensor_id;
    }

    void
    update(this auto& self)
    {
      self.updateImpl();
    }

    uint64_t
    getValue(this auto& self, uint8_t sensor_reading = 0)
    {
      return self.getValueImpl(sensor_reading);
    }

   protected:
    vmg_system::hal::HAL* _hal{nullptr};
    size_t _sensor_id{};
    BusType _bus_type{};
  };
}  // namespace vmg_system::sensors
