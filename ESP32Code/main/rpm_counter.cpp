#include "rpm_counter.h"

/// @brief ISR which increments amount of rotations
/// @param arg NULL
static void IRAM_ATTR gpio_rotation_isr_handler(void *arg)
{
    rotation_count++;

    printf("Got rotation");
}

/// @brief timer callback, every second recalculates rpm based on amount of rotations per second
/// @param arg NULL
static void periodic_timer_callback(void *arg)
{
    uint16_t count = rotation_count / AMOUNT_OF_WINGS;
    uint64_t time_to_count = esp_timer_get_time() - time_rpm;

    final_rpm = count / time_to_count * ONE_SECOND_MS * 60;

    time_rpm = esp_timer_get_time();
    rotation_count = 0;
}


/// @brief function to safe rewriting rpm
/// @param arg NULL
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
