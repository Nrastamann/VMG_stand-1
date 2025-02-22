#include "packets_and_sending.h"

uint32_t PACKET_DATA::rpm() { return _rpm; }
void PACKET_DATA::rpm_set(const uint32_t rpm){_rpm = rpm;} 

uint32_t PACKET_DATA::adc(const uint32_t index) { return _ADC_Readings[index]; };
void PACKET_DATA::adc_set(const uint32_t adc, const uint32_t index){_ADC_Readings[index] = adc;} 

uint16_t PACKET_DATA::temperature(const uint32_t index) { return _temperature[index]; }
void PACKET_DATA::temperature_set(const uint16_t temperature, const uint32_t index){_temperature[index] = temperature;} 

uint32_t PACKET_DATA::weight() { return _weight; }
void PACKET_DATA::weight_set(const uint32_t weight){_weight = weight;} 

unsigned char PACKET_DATA::crc()  { return _crc; }
void PACKET_DATA::crc_set(const unsigned char crc){_crc = crc;} 

// class Bluetooth : public Transmission_protocols
// {
//     void send_data() final
//     {
//     }
// };

// class UDP : public Transmission_protocols
// {
//     void send_data() final
//     {
//     }
// };

// class UART : public Transmission_protocols
// {
//     void send_data() final
//     {
//     }
// };

unsigned char crc8(char *buffer, char size)
{
    unsigned char crc = 0;
    for (unsigned char i = 0; i < size; i++)
    {
        unsigned char data = buffer[i];
        for (int j = 8; j > 0; j--)
        {
            crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
            data >>= 1;
        }
    }
    return crc;
}