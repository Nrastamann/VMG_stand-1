#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include "esp_sleep.h"

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

//================================================
// ADC VARIABLES
//================================================

static void periodic_timer_callback(void *arg);

#define ADC_UNIT ADC_UNIT_1
#define _ADC_UNIT_STR(unit) #unit
#define ADC_UNIT_STR(unit) _ADC_UNIT_STR(unit)
#define ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_1
#define ADC_ATTEN ADC_ATTEN_DB_0
#define ADC_BIT_WIDTH SOC_ADC_DIGI_MAX_BITWIDTH

#define ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define ADC_GET_CHANNEL(p_data) ((p_data)->type1.channel)
#define ADC_GET_DATA(p_data) ((p_data)->type1.data)

#define READ_LEN 256

#define ADC_USED 3

#define ADC_CURRENT ADC_CHANNEL_5
#define ADC_VOLTAGE ADC_CHANNEL_6
#define ADC_DISTURBANCE ADC_CHANNEL_7

const static adc_channel_t channel[ADC_USED] = {ADC_CURRENT, ADC_VOLTAGE, ADC_DISTURBANCE};
static adc_continuous_handle_t adc_handler = NULL;

static const char *TAG = "DEBUG";

#define AMOUNT_OF_WINGS 2

static uint8_t result[READ_LEN] = {0};
struct PACKET_DATA
{
    uint32_t rpm; //done                    // Rotation per minute
    uint32_t ADC_Readings[ADC_USED]; //done  // current * 1000, voltage * 1000, and disturbance idk, I'll figure it out, when understand how voltage/current sensor works
    uint16_t temperature_1;          // I'll change this name, I promisse
    uint16_t temperature_2;          // I'll change this name, I promisse
    uint16_t temperature_3;          // I'll change this name, I promisse
    uint32_t weight;                 // I guess * 1000
    // maybeeee use smth like byte array to faster the process? also need to add some like
    // test code? to check if msg wasn't corrupted
};

static uint64_t time_rpm = 0; 
static uint16_t rotation_count = 0;
static struct PACKET_DATA packet_to_send = {0};

/*
// Callback that notify that ADC continuous driver has done enough number of conversions
//   static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
//   {
//       BaseType_t mustYield = pdFALSE;
//      //notify that ADC continuous driver has done enough number of conversions
//       vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

//      return (mustYield == pdTRUE);
//  }

// Init function, create some cfgs, initialise adc
*/
static void oneshot_adc_init(adc_channel_t *channel, uint8_t channel_num) // maybe remove parameters, bcz they're global
{
    adc_oneshot_unit_handle_t handle = NULL;

    adc_oneshot_unit_init_cfg_t dig_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&dig_cfg, &handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BIT_WIDTH,
    };

    for (uint8_t i = 0; i < ADC_USED; i++)
        ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, channel[i], &chan_config));

    ESP_LOGI(TAG, "ADC INITIALIZATION IS DONE");

    adc_handler = handle;
}

#define GPIO_INPUT_IO_0 CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1 CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_INPUT_IO_0)

/*
 * I suppose, that 1ULL - all possible pins, so be careful
 * Use bitwise (<<,>>, |, & - mostly) operations to get desired pin
 * */
#define ESP_INTR_FLAG_DEFAULT 0 // I think that flag is used to give us ability to handle all the GPIO separetely

// static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_rotation_isr_handler(void *arg)
{
    rotation_count++;
    // xTaskNotify();
    // packet_to_send.rpm++;

    printf("Got rotation");
}

static void periodic_timer_callback(void* arg)
{
    uint16_t count = rotation_count / AMOUNT_OF_WINGS;
    uint64_t time_to_count = esp_timer_get_time() - time_rpm;

    packet_to_send.rpm = count / time_to_count;
}

/*
static void rpm_calculation(void *arg) // arg - amount of rotations
{
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, NULL, portMAX_DELAY))
        {
            rotation_count++; // maybe change it, so we can increment value in handler?

            printf("Another rotation");
        }
    }
}
*/

/// @brief
/// @param arg
static void adc_reading_task(void *arg)
{

    char unit[] = ADC_UNIT_STR(ADC_UNIT);
    uint32_t ret_num = 0;
    esp_err_t ret;
    uint8_t index;
    adc_channel_t channel = *(adc_channel_t *)arg;
    uint16_t readings[10] = {0};

    switch (channel)
    {
    case ADC_CURRENT:
        index = 0;
        break;

    case ADC_VOLTAGE:
        index = 1;
        break;

    case ADC_DISTURBANCE:
        index = 2;
        break;

    default:
        ESP_LOGW(TAG, "WRONG CHANNEL SENT TO ADC, FIX IT");
        break;
    }

    for (;;)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handler, channel, &readings[0]));
        //        if (){} add smth with sending semaphores or idk

        packet_to_send.ADC_Readings[index] = readings;

        vTaskDelay(10);
    }
}

void app_main(void)
{

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        /* name is optional, but may help identify the timer when debugging */
        .name = "periodic"};

    time_rpm = esp_timer_get_time();

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000));

    gpio_config_t io_conf = {
        GPIO_INPUT_PIN_SEL, // gpio mask
        GPIO_MODE_INPUT,    /*!< GPIO mode: set input/output mode                     */
        1,                  /*!< GPIO pull-up                                         */
        0,                  /*!< GPIO pull-down                                       */
        GPIO_INTR_POSEDGE,  /*!< GPIO interrupt type                                  */
    };

    gpio_config(&io_conf);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_rotation_isr_handler, NULL);

    oneshot_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t));

    xTaskCreate(adc_reading_task, "ADC_Voltage", 2048, (void *)ADC_VOLTAGE, 10, NULL);         // check priorities, last null - handler
    xTaskCreate(adc_reading_task, "ADC_Current", 2048, (void *)ADC_CURRENT, 10, NULL);         // check priorities, last null - handler
    xTaskCreate(adc_reading_task, "ADC_Disturbance", 2048, (void *)ADC_DISTURBANCE, 10, NULL); // check priorities, last null - handler

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    // move it somewhere else
    memset(result, 0xcc, READ_LEN);

    while (1)
    {
        vTaskDelay(10);
    }
}