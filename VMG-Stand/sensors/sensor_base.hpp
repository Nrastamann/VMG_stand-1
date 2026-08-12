#pragma once

#include <cassert>
#include <cstdint>
namespace vmg_system::sensors {
  enum class BusType : uint8_t {
    NONE,
    I2C,
    SPI,
    UART,
    TIMER,
    INTERRUPT,
  };

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

    int
    collectSPI(this auto& self)
    {
      static_assert(SingleBus == BusType::SPI || SingleBus == BusType::NONE,
                    "Calling for spi bus, when configured to use other bus");
      return self.collectSPIImpl();
    }  // is there any sense to that?
    void
    collectI2C(this auto& self)
    {
      static_assert(SingleBus == BusType::I2C || SingleBus == BusType::NONE,
                    "Calling for i2c bus, when configured to use other bus");
      return self.collectI2CImpl();
    }
    void
    collectUART(this auto& self)
    {
      static_assert(SingleBus == BusType::UART || SingleBus == BusType::NONE,
                    "Calling for uart bus, when configured to use other bus");
      return self.collectUARTImpl();
    }
    void
    collectTimer(this auto& self)
    {
      static_assert(SingleBus == BusType::TIMER || SingleBus == BusType::NONE,
                    "Calling for timer bus, when configured to use other bus");
      return self.collectTimerImpl();
    }
    void
    collectInt(this auto& self)
    {
      static_assert(
          SingleBus == BusType::INTERRUPT || SingleBus == BusType::NONE,
          "Calling for interrupt bus, when configured to use other bus");
      return self.collectIntImpl();
    }

    size_t _sensor_id{};
    BusType _bus_type{};
  };
}  // namespace vmg_system::sensors
