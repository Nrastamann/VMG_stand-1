
#include "freertos/FreeRTOS.h"
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
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include "esp_sleep.h"
extern "C"{
#include "HX711.h"
}


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
#define ADC_BIT_WIDTH ADC_BITWIDTH_12

#define ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define ADC_GET_CHANNEL(p_data) ((p_data)->type1.channel)
#define ADC_GET_DATA(p_data) ((p_data)->type1.data)

#define READ_LEN 256

#define ADC_USED 3

#define ADC_CURRENT ADC_CHANNEL_5
#define ADC_VOLTAGE ADC_CHANNEL_6
#define ADC_DISTURBANCE ADC_CHANNEL_7

static adc_channel_t channel[ADC_USED] = {ADC_CURRENT, ADC_VOLTAGE, ADC_DISTURBANCE};

static adc_oneshot_unit_handle_t adc_handler = NULL;

//=================================================================
// TASK HANDLERS(For notifies)
//=================================================================

static TaskHandle_t VOLTAGE_TASK_HANDLE = NULL;
static TaskHandle_t CURRENT_TASK_HANDLE = NULL;
static TaskHandle_t DISTURBNCE_TASK_HANDLE = NULL;

static TaskHandle_t TEMPERATURE1_TASK_HANDLE = NULL;
static TaskHandle_t TEMPERATURE2_TASK_HANDLE = NULL;
static TaskHandle_t TEMPERATURE3_TASK_HANDLENULL;

static TaskHandle_t VOLTAGE_TASK_HANDLE = NULL;
static TaskHandle_t VOLTAGE_TASK_HANDLE = NULL;
static TaskHandle_t VOLTAGE_TASK_HANDLE = NULL;

static TaskHandle_t WEIGHT_TASK_HANDLE = NULL;
static TaskHandle_t RPM_TASK_HANDLE = NULL;

//=================================================================
// LOGING TAGS
//=================================================================

static const char *TAG = "DEBUG";

//=================================================================
// RPM COUNTING CONSTANTS
//=================================================================

#define AMOUNT_OF_WINGS 2
#define ONE_SECOND_MS 1000000

//=================================================================
// HX711 CONSTANTS
//=================================================================

#define GPIO_SCALES_DATA GPIO_NUM_16
#define GPIO_SCLK GPIO_NUM_17
#define AVG_SAMPLES 10

#define COUNTING_NOTIFY pdFALSE
#define BINARY_NOTIFY pdTRUE

struct PACKET_DATA
{
    uint32_t rpm;                    // done                    // Rotation per minute
    uint32_t ADC_Readings[ADC_USED]; // done  // current * 1000, voltage * 1000, and disturbance idk, I'll figure it out, when understand how voltage/current sensor works
    uint16_t temperature_1;          // I'll change this name, I promisse
    uint16_t temperature_2;          // I'll change this name, I promisse
    uint16_t temperature_3;          // I'll change this name, I promisse
    uint32_t weight;                 // done         // I guess * 1000
    // maybeeee use smth like byte array to faster the process? also need to add some like
    // test code? to check if msg wasn't corrupted
};

//=================================================================
// RPM VARIABLES
//=================================================================

static uint64_t time_rpm = 0;
static uint16_t rotation_count = 0;
static uint64_t final_rpm = 0;

static struct PACKET_DATA packet_to_send = {0};

static void weight_reading_task(void *arg)
{
    HX711_init(GPIO_SCALES_DATA, GPIO_SCLK, eGAIN_128);
    HX711_tare();

    uint64_t weight = 0;

    for (;;)
    {
        weight = HX711_get_units(AVG_SAMPLES);

        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(ONE_SECOND_MS));

        packet_to_send.weight = weight;
        vTaskDelay(100);
    }
}

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

//=================================================================
// ISR PINS
//=================================================================

#define ESP_INTR_FLAG_DEFAULT 0 // I think that flag is used to give us ability to handle all the GPIO separetely
#define GPIO_INPUT_IO_0 GPIO_NUM_4//4
#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_INPUT_IO_0)

/*
 * I suppose, that 1ULL - all possible pins, so be careful
 * Use bitwise (<<,>>, |, & - mostly) operations to get desired pin
 * */

static void IRAM_ATTR gpio_rotation_isr_handler(void *arg)
{
    rotation_count++;

    printf("Got rotation");
}

static void periodic_timer_callback(void *arg)
{
    uint16_t count = rotation_count / AMOUNT_OF_WINGS;
    uint64_t time_to_count = esp_timer_get_time() - time_rpm;

    packet_to_send.rpm = count / time_to_count * ONE_SECOND_MS * 60;

    time_rpm = esp_timer_get_time();
    rotation_count = 0;
}
static void rpm_safe_writing_task(void *arg)
{
    for (;;)
    {
        if (final_rpm)
        {
            ulTaskNotifyTake(COUNTING_NOTIFY, pdMS_TO_TICKS(ONE_SECOND_MS * 5));

            packet_to_send.rpm = final_rpm;

            final_rpm = 0;
        }
        else
        {
            vTaskDelay(100);
        }
    }
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
    int readings[10] = {0};

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

        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(ONE_SECOND_MS));

        packet_to_send.ADC_Readings[index] = readings[0];

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

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, ONE_SECOND_MS));

    gpio_config_t io_conf = {
        GPIO_INPUT_PIN_SEL, // gpio mask
        GPIO_MODE_INPUT,    /*!< GPIO mode: set input/output mode                     */
        GPIO_PULLUP_DISABLE,                  /*!< GPIweight_reading_task-up                                         */
        GPIO_PULLDOWN_DISABLE,                  /*!< GPIO pull-down                                       */
        GPIO_INTR_POSEDGE,  /*!< GPIO interrupt type                                  */
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