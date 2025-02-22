#pragma once

#include "adc_reading.h"
#include <cstddef>
#include <utility>
class Transmission_protocols
{
public:
    virtual void send_data(void *pvParameters) = 0;
};

// class Bluetooth : public Transmission_protocols
// {
//     void send_data() final;
// };

// class UDP : public Transmission_protocols
// {
//     void send_data() final;
// };

// class UART : public Transmission_protocols
// {
//     void send_data() final;
// };

unsigned char crc8(unsigned char *buffer, unsigned char size);

/// @brief
class PACKET_DATA
{
    uint32_t _rpm;                    // done                    // Rotation per minute
    uint32_t _ADC_Readings[ADC_USED]; // done  // current * 1000, voltage * 1000, and disturbance idk, I'll figure it out, when understand how voltage/current sensor works
    uint16_t _temperature[3];         // temperature
    uint32_t _weight;                 // done         // I guess * 1000
    unsigned char _crc;               // crc
public:
    PACKET_DATA() = default;

    uint32_t rpm();
    void rpm_set(const uint32_t rpm);

    uint32_t adc(const uint32_t index);
    void adc_set(const uint32_t adc, const uint32_t index);

    uint16_t temperature(const uint32_t index);
    void temperature_set(const uint16_t temperature, const uint32_t index);

    uint32_t weight();
    void weight_set(const uint32_t weight);

    unsigned char crc();
    void crc_set(const unsigned char crc);

    // maybeeee use smth like byte array to faster the process? also need to add some like
    // test code? to check if msg wasn't corrupted
};

extern struct PACKET_DATA packet_to_send;