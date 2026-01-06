#include "ads1115.hpp"

#include <cstdint>

#include "vmg_i2c/i2c_driver.hpp"

constexpr uint8_t ONE_BYTE_SHIFT{8};
constexpr uint8_t SECOND_BYTE_AND{0xFF};

void
ads1115::writeRegisterImpl(uint8_t reg, uint16_t value,
                           i2c_subscriber<BUFFER_SIZE, BUFFER_SIZE>& bus)
{
  bus._w_buffer[0] = reg;
  bus._w_buffer[1] = value >> ONE_BYTE_SHIFT;
  bus._w_buffer[2] = value & SECOND_BYTE_AND;
  bus.send_data();
}

uint16_t
ads1115::readRegisterImpl(uint8_t reg,
                          i2c_subscriber<BUFFER_SIZE, BUFFER_SIZE>& bus)
{
  bus._w_buffer[0] = reg;
  bus.send_and_read_data();  // need to send only 1 byte, and then get 2

  return ((bus._r_buffer[0] << ONE_BYTE_SHIFT) | bus._r_buffer[1]);
}

void
ads1115::readADCImpl(i2c_subscriber<BUFFER_SIZE, BUFFER_SIZE>& bus)
{
}

bool
ads1115::probeImpl(i2c_subscriber<BUFFER_SIZE, BUFFER_SIZE>& bus)
{
}
