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
//ADC VARIABLES
//================================================

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

static adc_channel_t channel[ADC_USED] = {ADC_CHANNEL_6, ADC_CHANNEL_7, ADC_CHANNEL_5};
static adc_continuous_handle_t adc_handler = NULL;
static TaskHandle_t s_task_handle;
static const char *TAG = "DEBUG";

static uint8_t result[READ_LEN] = {0};
struct PACKET_DATA
{
    uint32_t rpm; //Rotation per minute
    uint32_t current; //current * 1000? idk, I'll figure it out, when understand how voltage/current sensor works
    uint32_t voltage; //voltage * 1000 i guess
    uint16_t temperature_1; //I'll change this name, I promisse
    uint16_t temperature_2; //I'll change this name, I promisse
    uint16_t temperature_3; //I'll change this name, I promisse
    uint32_t weight; // I guess * 1000
    uint32_t disturbance; //I don't know 
    //maybeeee use smth like byte array to faster the process? also need to add some like
    //test code? to check if msg wasn't corrupted
};

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
static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = READ_LEN,
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = ADC_CONV_MODE,
        .format = ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channel_num;
    for (int i = 0; i < channel_num; i++)
    {
        adc_pattern[i].atten = ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7; // idk why the fugg 7, rly
        adc_pattern[i].unit = ADC_UNIT;
        adc_pattern[i].bit_width = ADC_BIT_WIDTH;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is :%" PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is :%" PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is :%" PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    adc_handler = handle;
}

#define GPIO_INPUT_IO_0 CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1 CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_INPUT_IO_0)

static uint32_t rotation_count = 0;
static uint32_t adc_current = 0;
/*
 * I suppose, that 1ULL - all possible pins, so be careful
 * Use bitwise (<<,>>, |, & - mostly) operations to get desired pin
 * */
#define ESP_INTR_FLAG_DEFAULT 0 // I think that flag is used to give us ability to handle all the GPIO separetely

//static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_rotation_isr_handler(void *arg)
{
    rotation_count++;
    //need to 
    printf("Got rotation");
}

/*
static void gpio_task(void *arg) // arg - amount of rotations
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
static void adc_reading_task(void *arg)
{

    char unit[] = ADC_UNIT_STR(ADC_UNIT);
    uint32_t ret_num = 0;
    esp_err_t ret;

    for (;;)
    {
        ret = adc_continuous_read(adc_handler, result, READ_LEN, &ret_num, 0);
        if (ret == ESP_OK)
        {
            ESP_LOGI("TASK", "ret is %x, ret_num is %" PRIu32 " bytes", ret, ret_num);
            for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
            {
                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                uint32_t chan_num = ADC_GET_CHANNEL(p);
                uint32_t data = ADC_GET_DATA(p);
                /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                if (chan_num < SOC_ADC_CHANNEL_NUM(ADC_UNIT))
                {
                    ESP_LOGI(TAG, "Unit: %s, Channel: %" PRIu32 ", Value: %" PRIx32, unit, chan_num, data);
                }
                else
                {
                    ESP_LOGW(TAG, "Invalid data [%s_%" PRIu32 "_%" PRIx32 "]", unit, chan_num, data);
                }
            }
            /**
             * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
             * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
             * usually you don't need this delay (as this task will block for a while).
             */
            vTaskDelay(1);
        }
        else if (ret == ESP_ERR_TIMEOUT)
        {
            // We try to read `READ_LEN` until API returns timeout, which means there's no available data
            break;
        }
    }
    ESP_ERROR_CHECK(adc_continuous_stop(adc_handler));
    ESP_ERROR_CHECK(adc_continuous_deinit(adc_handler));

    ESP_LOGW(TAG, "TASK WAS DELETED, CHECK EVERYTHING AND TRY FIXING");

    vTaskDelete(NULL);
}

void app_main(void)
{
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

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    //move it somewhere else
    memset(result, 0xcc, READ_LEN);

    continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t));

    while (1)
    {
        vTaskDelay(1);
    }

}