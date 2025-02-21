#include "adc_readings.h"

/// @brief 
/// @param channel 
/// @param channel_num 
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

    ESP_LOGI(ADC_TAG, "ADC INITIALIZATION IS DONE");

    adc_handler = handle;
}


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
        ESP_LOGW(ADC_TAG, "WRONG CHANNEL SENT TO ADC, FIX IT");
        break;
    }

    for (;;)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handler, channel, &readings[0]));

        ulTaskNotifyTake(COUNTING_NOTIFY, pdMS_TO_TICKS(ONE_SECOND_MS));

        packet_to_send.ADC_Readings[index] = readings[0];

        vTaskDelay(10);
    }
}
