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
    static_cast<Derived*>(this)->writeRegisterImpl(reg, value, this);
  };

  uint16_t
  readRegister(uint8_t reg)
  {
    return static_cast<Derived*>(this)->readRegisterImpl(reg, this);
  }

  int16_t
  readADCSingle(uint8_t channel)
  {
    return static_cast<Derived*>(this)->readADCImplSingle(channel, this);
  }

  uint8_t
  getGain()
  {
    return _gain;
  }

  void
  setGain(uint8_t gain)
  {
    _gain = gain;
  }
  void
  setDataRate(uint16_t dataRate)
  {
    _dataRate = dataRate;
  }

  uint16_t
  getDataRate()
  {
    return _dataRate;
  }

  bus&
  getBus()
  {
    return _bus;
  }

  void
  setBus(bus const& configured_bus)
  {
    _bus = configured_bus;
  }

  int16_t
  readADCDifferential(uint8_t channel1, uint8_t channel2)
  {
    return static_cast<Derived*>(this)->readDifferentialImpl(channel1, channel2,
                                                             this);
  }

  void
  startComparator(uint8_t channel, int16_t threshold)
  {
    static_cast<Derived*>(this)->startComparatorImpl(channel, threshold, this);
  }
  int16_t
  getLastConversion()
  {
    return static_cast<Derived*>(this)->getLastConversionImpl(this);
  }
  float
  getFsRange()
  {
    return static_cast<Derived*>(this)->getFsRangeImpl(this);
  }

  uint8_t
  getBitShift()
  {
    return _bitShift;
  }

  float
  computeVolts()
  {
    return static_cast<Derived*>(this)->computeVoltsImpl(this);
  }

  void
  startADCReading(uint16_t mux, bool continuous)
  {
    return static_cast<Derived*>(this)->startADCReadingImpl(mux, continuous,
                                                            this);
  }

  bool
  conversionComplete()
  {
    return static_cast<Derived*>(this)->conversionCompleteImpl(this);
  }

 protected:
  bus _bus;
  uint8_t _bitShift;  ///< bit shift amount
  uint8_t _gain;      ///< ADC gain
  uint16_t _dataRate;
};
