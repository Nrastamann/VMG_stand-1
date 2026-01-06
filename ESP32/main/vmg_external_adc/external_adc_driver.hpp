#include <cstdint>
#include <cstdlib>

template <typename Derived, typename bus>
class external_adc {
 public:
  external_adc(uint8_t bitshift, uint8_t gain, uint16_t dataRate) :
      _bitShift(bitshift), _gain(gain), _dataRate(dataRate)
  {
  }

  void
  writeRegister(uint8_t reg, uint16_t value)
  {
    static_cast<Derived*>(this)->writeRegisterImpl(reg, value, _bus);
  };
  uint16_t
  readRegister(uint8_t reg)
  {
    return static_cast<Derived*>(this)->readRegisterImpl(reg, _bus);
  }
  void
  readADC()
  {
    static_cast<Derived*>(this)->readADCImpl(_bus);
  }
  bool
  probe()
  {
    return static_cast<Derived*>(this)->probeImpl(_bus);
  }
  bus _bus;
  uint8_t _bitShift;  ///< bit shift amount
  uint8_t _gain;      ///< ADC gain
  uint16_t _dataRate;
};
