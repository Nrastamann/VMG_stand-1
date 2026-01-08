#include <array>
#include <cstdint>
#include <unordered_map>

#include "vmg_external_adc/external_adc_driver.hpp"
#include "vmg_i2c/i2c_driver.hpp"
/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
constexpr uint16_t ADS1X15_ADDRESS{0x48};  ///< 1001 000 (ADDR = GND)
/*=========================================================================*/

/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
constexpr uint16_t ADS1X15_REG_POINTER_MASK{0x03};       ///< Point mask
constexpr uint16_t ADS1X15_REG_POINTER_CONVERT{0x00};    ///< Conversion
constexpr uint16_t ADS1X15_REG_POINTER_CONFIG{0x01};     ///< Configuration
constexpr uint16_t ADS1X15_REG_POINTER_LOWTHRESH{0x02};  ///< Low threshold
constexpr uint16_t ADS1X15_REG_POINTER_HITHRESH{0x03};   ///< High threshold
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
constexpr uint16_t ADS1X15_REG_CONFIG_OS_MASK{0x8000};
///< OS Mask
constexpr uint16_t ADS1X15_REG_CONFIG_OS_SINGLE{0x8000};
///< Write: Set to start a single-conversion
constexpr uint16_t ADS1X15_REG_CONFIG_OS_BUSY{0x0000};
///< Read: Bit = 0 when conversion is in progress
constexpr uint16_t ADS1X15_REG_CONFIG_OS_NOTBUSY{0x8000};
///< Read: Bit = 1 when device is not performing a conversion

constexpr uint16_t ADS1X15_REG_CONFIG_MUX_MASK{0x7000};  ///< Mux Mask
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_DIFF_0_1{0x0000};
///< Differential P = AIN0, N = AIN1 (default)
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_DIFF_0_3{0x1000};
///< Differential P = AIN0, N = AIN3
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_DIFF_1_3{0x2000};
///< Differential P = AIN1, N = AIN3
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_DIFF_2_3{0x3000};
///< Differential P = AIN2, N = AIN3
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_SINGLE_0{0x4000};
///< Single-ended AIN0
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_SINGLE_1{0x5000};
///< Single-ended AIN1
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_SINGLE_2{0x6000};
///< Single-ended AIN2
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_SINGLE_3{0x7000};
///< Single-ended AIN3

constexpr std::array<uint16_t, 4> MUX_BY_CHANNEL = {
    ADS1X15_REG_CONFIG_MUX_SINGLE_0,  ///< Single-ended AIN0
    ADS1X15_REG_CONFIG_MUX_SINGLE_1,  ///< Single-ended AIN1
    ADS1X15_REG_CONFIG_MUX_SINGLE_2,  ///< Single-ended AIN2
    ADS1X15_REG_CONFIG_MUX_SINGLE_3   ///< Single-ended AIN3
};  ///< MUX config by channel

constexpr uint16_t ADS1X15_REG_CONFIG_PGA_MASK{0x0E00};  ///< PGA Mask
constexpr uint16_t ADS1X15_REG_CONFIG_PGA_6_144V{
    0x0000};  ///< +/-6.144V range = Gain 2/3
constexpr uint16_t ADS1X15_REG_CONFIG_PGA_4_096V{
    0x0200};  ///< +/-4.096V range = Gain 1
constexpr uint16_t ADS1X15_REG_CONFIG_PGA_2_048V{0x0400};
///< +/-2.048V range = Gain 2 (default)
constexpr uint16_t ADS1X15_REG_CONFIG_PGA_1_024V{
    0x0600};  ///< +/-1.024V range = Gain 4
constexpr uint16_t ADS1X15_REG_CONFIG_PGA_0_512V{
    0x0800};  ///< +/-0.512V range = Gain 8
constexpr uint16_t ADS1X15_REG_CONFIG_PGA_0_256V{
    0x0A00};  ///< +/-0.256V range = Gain 16

constexpr uint16_t ADS1X15_REG_CONFIG_MODE_MASK{0x0100};  ///< Mode Mask
constexpr uint16_t ADS1X15_REG_CONFIG_MODE_CONTIN{
    0x0000};  ///< Continuous conversion mode
constexpr uint16_t ADS1X15_REG_CONFIG_MODE_SINGLE{0x0100};
///< Power-down single-shot mode (default)

constexpr uint16_t ADS1X15_REG_CONFIG_RATE_MASK{0x00E0};  ///< Data Rate Mask

constexpr uint16_t ADS1X15_REG_CONFIG_CMODE_MASK{0x0010};  ///< CMode Mask
constexpr uint16_t ADS1X15_REG_CONFIG_CMODE_TRAD{0x0000};
///< Traditional comparator with hysteresis (default)
constexpr uint16_t ADS1X15_REG_CONFIG_CMODE_WINDOW{
    0x0010};  ///< Window comparator

constexpr uint16_t ADS1X15_REG_CONFIG_CPOL_MASK{0x0008};  ///< CPol Mask
constexpr uint16_t ADS1X15_REG_CONFIG_CPOL_ACTVLOW{0x0000};
///< ALERT/RDY pin is low when active (default)
constexpr uint16_t ADS1X15_REG_CONFIG_CPOL_ACTVHI{0x0008};
///< ALERT/RDY pin is high when active

constexpr uint16_t ADS1X15_REG_CONFIG_CLAT_MASK{0x0004};
///< Determines if ALERT/RDY pin latches once asserted
constexpr uint16_t ADS1X15_REG_CONFIG_CLAT_NONLAT{0x0000};
///< Non-latching comparator (default)
constexpr uint16_t ADS1X15_REG_CONFIG_CLAT_LATCH{
    0x0004};  ///< Latching comparator

constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_MASK{0x0003};  ///< CQue Mask
constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_1CONV{0x0000};
///< Assert ALERT/RDY after one conversions
constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_2CONV{0x0001};
///< Assert ALERT/RDY after two conversions
constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_4CONV{0x0002};
///< Assert ALERT/RDY after four conversions
constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_NONE{
    0x0003};  ///< Disable the comparator and put
              ///< ALERT/RDY in high state (default)
/*=========================================================================*/

/** Gain settings */
enum class adcGain_t {
  GAIN_TWOTHIRDS = ADS1X15_REG_CONFIG_PGA_6_144V,
  GAIN_ONE       = ADS1X15_REG_CONFIG_PGA_4_096V,
  GAIN_TWO       = ADS1X15_REG_CONFIG_PGA_2_048V,
  GAIN_FOUR      = ADS1X15_REG_CONFIG_PGA_1_024V,
  GAIN_EIGHT     = ADS1X15_REG_CONFIG_PGA_0_512V,
  GAIN_SIXTEEN   = ADS1X15_REG_CONFIG_PGA_0_256V
};

/** Data rates */
constexpr uint16_t RATE_ADS1015_128SPS{0x0000};  ///< 128 samples per second
constexpr uint16_t RATE_ADS1015_250SPS{0x0020};  ///< 250 samples per second
constexpr uint16_t RATE_ADS1015_490SPS{0x0040};  ///< 490 samples per second
constexpr uint16_t RATE_ADS1015_920SPS{0x0060};  ///< 920 samples per second
constexpr uint16_t RATE_ADS1015_1600SPS{
    0x0080};  ///< 1600 samples per second (default)
constexpr uint16_t RATE_ADS1015_2400SPS{0x00A0};  ///< 2400 samples per second
constexpr uint16_t RATE_ADS1015_3300SPS{0x00C0};  ///< 3300 samples per second
                                                  ///

constexpr uint16_t RATE_ADS1115_8SPS{0x0000};   ///< 8 samples per second
constexpr uint16_t RATE_ADS1115_16SPS{0x0020};  ///< 16 samples per second
constexpr uint16_t RATE_ADS1115_32SPS{0x0040};  ///< 32 samples per second
constexpr uint16_t RATE_ADS1115_64SPS{0x0060};  ///< 64 samples per second
constexpr uint16_t RATE_ADS1115_128SPS{
    0x0080};  ///< 128 samples per second (default)
constexpr uint16_t RATE_ADS1115_250SPS{0x00A0};  ///< 250 samples per second
constexpr uint16_t RATE_ADS1115_475SPS{0x00C0};  ///< 475 samples per second
constexpr uint16_t RATE_ADS1115_860SPS{0x00E0};  ///< 860 samples per second

constexpr uint8_t BUFFER_SIZE{3};
struct ads1115
    : protected external_adc<ads1115,
                             i2c_subscriber<BUFFER_SIZE, BUFFER_SIZE>> {
 private:
  using bus_type  = i2c_subscriber<BUFFER_SIZE, BUFFER_SIZE>;
  using base_type = external_adc<ads1115, bus_type>;

 public:
  ads1115() = default;

  void static writeRegisterImpl(uint8_t reg, uint16_t value, base_type* base);
  uint16_t static readRegisterImpl(uint8_t reg, base_type* base);
  static int16_t readADCSingleImpl(uint8_t channel, base_type* base);

  static int16_t readDifferentialImpl(uint8_t channel1, uint8_t channel2,
                                      base_type* base);
  static void startComparatorImpl(uint8_t channel, int16_t threshold,
                                  base_type* base);

  static int16_t getLastConversionImpl(base_type* base);
  static float getFsRangeImpl(base_type* base);

  static float computeVoltsImpl(int16_t amount, base_type* base);
  static void startADCReadingImpl(uint16_t mux, bool continuous,
                                  base_type* base);

  static bool conversionCompleteImpl(base_type* base);
};
