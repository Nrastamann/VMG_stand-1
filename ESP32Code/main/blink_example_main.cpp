extern "C"
{
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#include "sdkconfig.h"
#include "esp_timer.h"
#include "esp_sleep.h"

}

//================================================
// MINE HEADERS
//================================================

#include "adc_reading.h"
#include "rpm_counter.h"
#include "wifi_connection.h"
#include "tcp_connection.h"
#include "HX711_reading.h"


/**
 * Brief:
 * This code is running on VMG-Stand.
 *
 * Done:
 * Part with IR Sensor, should config pins later, also need to test it
 *
 * Wip:
 * Need to add Voltage sensor support (current priority)
 * Need to add Current sensor support
 * Need to add PWM for motor
 * Need to add wifi transmission
 * Need to add some other sensors(termometers and smth else i think)
 * MAKE IT ALL READABLE
 *
 * ALL THIS UNTILL NEXT WEEK(17.02), WORK BTCH
 */

// data contains in next format: 12 bits for data and 4 for channel

//=================================================================
// LOGING TAGS
//=================================================================

static const char *MAIN_TAG = "MAIN";

#define COUNTING_NOTIFY pdFALSE
#define BINARY_NOTIFY pdTRUE
class Transmission_protocols
{
public:
    virtual void send_data() = 0;
};

class Bluetooth : public Transmission_protocols
{
    void send_data() final
    {
    }
};

class UDP : public Transmission_protocols
{
    void send_data() final
    {
    }
};

class UART : public Transmission_protocols
{
    void send_data() final
    {
    }
};

byte crc8(byte *buffer, byte size) {
    byte crc = 0;
    for (byte i = 0; i < size; i++) {
      byte data = buffer[i];
      for (int j = 8; j > 0; j--) {
        crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
        data >>= 1;
      }
    }
    return crc;
  }

/// @brief 
struct PACKET_DATA
{
    uint32_t rpm;                    // done                    // Rotation per minute
    uint32_t ADC_Readings[ADC_USED]; // done  // current * 1000, voltage * 1000, and disturbance idk, I'll figure it out, when understand how voltage/current sensor works
    uint16_t temperature_1;          // I'll change this name, I promisse
    uint16_t temperature_2;          // I'll change this name, I promisse
    uint16_t temperature_3;          // I'll change this name, I promisse
    uint32_t weight;                 // done         // I guess * 1000
    byte crc;                        //crc
    // maybeeee use smth like byte array to faster the process? also need to add some like
    // test code? to check if msg wasn't corrupted
};

static struct PACKET_DATA packet_to_send = {0};

void app_main(void)
{
    //=========================================================
    //WIFI CONNECTION
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    //=========================================================


    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        /* name is optional, but may help identify the timer when debugging */
        .name = "periodic"};

    time_rpm = esp_timer_get_time();

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, ONE_SECOND_MS));

    gpio_config_t io_conf = {
        GPIO_INPUT_PIN_SEL,    // gpio mask
        GPIO_MODE_INPUT,       /*!< GPIO mode: set input/output mode                     */
        GPIO_PULLUP_DISABLE,   /*!< GPIweight_reading_task-up                                         */
        GPIO_PULLDOWN_DISABLE, /*!< GPIO pull-down                                       */
        GPIO_INTR_POSEDGE,     /*!< GPIO interrupt type                                  */
    };

    gpio_config(&io_conf);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_rotation_isr_handler, NULL);

    oneshot_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t));

    xTaskCreate(weight_reading_task, "Weight_reading", 2048, NULL, 10, &WEIGHT_TASK_HANDLE);

    xTaskCreate(adc_reading_task, "ADC_Voltage", 2048, (void *)ADC_VOLTAGE, 10, &VOLTAGE_TASK_HANDLE);            // check priorities, last null - handler
    xTaskCreate(adc_reading_task, "ADC_Current", 2048, (void *)ADC_CURRENT, 10, &CURRENT_TASK_HANDLE);            // check priorities, last null - handler
    xTaskCreate(adc_reading_task, "ADC_Disturbance", 2048, (void *)ADC_DISTURBANCE, 10, &DISTURBNCE_TASK_HANDLE); // check priorities, last null - handler

    xTaskCreate(rpm_safe_writing_task, "Writing_RPM", 2048, NULL, 10, &RPM_TASK_HANDLE);

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    while (1)
    {
        vTaskDelay(10);
    }
}