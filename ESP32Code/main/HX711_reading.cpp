#include "HX711_reading.h"
#include "freertos/FreeRTOS.h"

/// @brief 
/// @param arg 
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