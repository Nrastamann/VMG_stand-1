#include "ads1115.hpp"

#include <cstdint>
#include <esp_log>

#include "esp_log.h"
#include "external_adc_driver.hpp"
#include "vmg_i2c/i2c_driver.hpp"

static constexpr uint8_t ONE_BYTE_SHIFT{8};
static constexpr uint8_t SECOND_BYTE_AND{0xFF};
static constexpr uint16_t TWO_POWER_FIFTEEN{0x8000};

static std::unordered_map<uint8_t, uint16_t const> const muxMap = {
    {1, ADS1X15_REG_CONFIG_MUX_DIFF_0_1},
    {3, ADS1X15_REG_CONFIG_MUX_DIFF_0_3},
    {4, ADS1X15_REG_CONFIG_MUX_DIFF_1_3},
    {5, ADS1X15_REG_CONFIG_MUX_DIFF_2_3}
};

void
ads1115::writeRegisterImpl(uint8_t reg, uint16_t value, base_type* base)
{
  bus_type& bus    = base->getBus();
  bus._w_buffer[0] = reg;
  bus._w_buffer[1] = value >> ONE_BYTE_SHIFT;
  bus._w_buffer[2] = value & SECOND_BYTE_AND;
  bus.send_data();
}

uint16_t
ads1115::readRegisterImpl(uint8_t reg, base_type* base)
{
  bus_type& bus    = base->getBus();
  bus._w_buffer[0] = reg;
  bus.send_and_read_data(1, 2);
  return ((bus._r_buffer[0] << ONE_BYTE_SHIFT) | bus._r_buffer[1]);
}

int16_t
ads1115::readADCSingleImpl(uint8_t channel, base_type* base)
{
  if (channel > 3) {
    ESP_LOGE("ads1115", "Wrong channel number");
    return 0;
  }

  startADCReadingImpl(MUX_BY_CHANNEL[channel], false, base);

  while (!conversionCompleteImpl(base));
  return getLastConversionImpl(base);
}

int16_t
ads1115::readDifferentialImpl(uint8_t channel1, uint8_t channel2,
                              base_type* base)
{
  if (channel1 > 3 || channel2 > 3) {
    ESP_LOGE("ads1115", "Wrong channel number");
    return 0;
  }

  auto iter = muxMap.find(channel1 + channel2);
  if (iter == muxMap.end()) {
    return 0;
  }

  startADCReadingImpl((*iter).second, false, base);

  while (!conversionCompleteImpl(base));

  return getLastConversionImpl(base);
}

void
ads1115::startComparatorImpl(uint8_t channel, int16_t threshold,
                             base_type* base)
{
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_1CONV |    // Comparator enabled and asserts on 1
                                         // match
      ADS1X15_REG_CONFIG_CLAT_LATCH |    // Latching mode
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW |  // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD |    // Traditional comparator (default val)
      ADS1X15_REG_CONFIG_MODE_CONTIN |   // Continuous conversion mode
      ADS1X15_REG_CONFIG_MODE_CONTIN;    // Continuous conversion mode

  // Set PGA/voltage range
  config |= base->getGain();

  // Set data rate
  config |= base->getDataRate();

  config |= MUX_BY_CHANNEL[channel];

  // Set the high threshold register
  // Shift 12-bit results left 4 bits for the ADS1015
  writeRegisterImpl(ADS1X15_REG_POINTER_HITHRESH,
                    threshold << base->getBitShift(), base);

  // Write config register to the ADC
  writeRegisterImpl(ADS1X15_REG_POINTER_CONFIG, config, base);
}

int16_t
ads1115::getLastConversionImpl(base_type* base)
{
  uint8_t shift = base->getBitShift();
  uint16_t res  = readRegisterImpl(ADS1X15_REG_POINTER_CONVERT, base) >> shift;

  if (shift == 0) {
    return shift;
  }

  if (res > 0x07FF) {
    res |= 0xF000;
  }

  return (int16_t)res;
}
static adcGain_t constexpr adcGainTConvert(uint8_t const val)
{
  switch (val) {
    case 0:
      return adcGain_t::GAIN_TWOTHIRDS;
    case 1:
      return adcGain_t::GAIN_ONE;
    case 2:
      return adcGain_t::GAIN_TWO;
    case 3:
      return adcGain_t::GAIN_FOUR;
    case 4:
      return adcGain_t::GAIN_EIGHT;
    case 5:
      return adcGain_t::GAIN_SIXTEEN;
    default:
      ESP_LOGE("ads1115", "Couldn't convert int to gain");
      return adcGain_t::GAIN_TWOTHIRDS;
  }
}

float
ads1115::getFsRangeImpl(base_type* base)
{
  switch (adcGainTConvert(base->getGain())) {
    case adcGain_t::GAIN_TWOTHIRDS:
      return 6.144F;
    case adcGain_t::GAIN_ONE:
      return 4.096F;
    case adcGain_t::GAIN_TWO:
      return 2.048F;
    case adcGain_t::GAIN_FOUR:
      return 1.024F;
    case adcGain_t::GAIN_EIGHT:
      return 0.512F;
    case adcGain_t::GAIN_SIXTEEN:
      return 0.256F;
    default:
      return 0.0F;
  }
}

float
ads1115::computeVoltsImpl(int16_t amount, base_type* base)
{
  return static_cast<float>(amount) *
         (getFsRangeImpl(base) /
          static_cast<float>(TWO_POWER_FIFTEEN >> base->getBitShift()));
}
void
ads1115::startADCReadingImpl(uint16_t mux, bool continuous, base_type* base)
{
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_1CONV |    // Set CQUE to any value other than
                                         // None so we can use it in RDY mode
      ADS1X15_REG_CONFIG_CLAT_NONLAT |   // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW |  // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD;     // Traditional comparator (default val)

  if (continuous) {
    config |= ADS1X15_REG_CONFIG_MODE_CONTIN;
  }
  else {
    config |= ADS1X15_REG_CONFIG_MODE_SINGLE;
  }

  // Set PGA/voltage range
  config |= base->getGain();

  // Set data rate
  config |= base->getDataRate();

  // Set channels
  config |= mux;

  // Set 'start single-conversion' bit
  config |= ADS1X15_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegisterImpl(ADS1X15_REG_POINTER_CONFIG, config, base);

  // Set ALERT/RDY to RDY mode.
  writeRegisterImpl(ADS1X15_REG_POINTER_HITHRESH, TWO_POWER_FIFTEEN, base);
  writeRegisterImpl(ADS1X15_REG_POINTER_LOWTHRESH, 0x0000, base);
}

bool
ads1115::conversionCompleteImpl(base_type* base)
{
  return (readRegisterImpl(ADS1X15_REG_POINTER_CONFIG, base) &
          TWO_POWER_FIFTEEN) != 0;
}
