#include "rpm_counter.hpp"

uint64_t time_rpm       = 0;
uint64_t rotation_count = 0;
uint64_t final_rpm      = 0;
uint64_t check_rpm = 0;

/// @brief ISR which increments amount of rotations
/// @param arg NULL
void IRAM_ATTR
gpio_rotation_isr_handler(void* arg)
{
  ++rotation_count;
  ++check_rpm;
}

/// @brief timer callback, every second recalculates rpm based on amount of
/// rotations per second
/// @param arg NULL
void
periodic_timer_callback(void* arg)
{
  uint64_t const count         = rotation_count / AMOUNT_OF_WINGS;
  ESP_LOGI("TAG IR","GOT %lld", rotation_count);
  ESP_LOGI("TAG IR WITHOUT CALCULATIONS","GOT %lld", check_rpm);
  
  uint64_t const time_to_count = esp_timer_get_time() - time_rpm;

  final_rpm                    = count * ONE_SECOND_MS / time_to_count;

  time_rpm                     = esp_timer_get_time();

  rotation_count               = 0;
}

/// @brief function to safe rewriting rpm
/// @param arg NULL
void
rpm_safe_writing_task(void* arg)
{
  for (;;) {
    if (final_rpm) {
      ulTaskNotifyTake(0, pdMS_TO_TICKS(ONE_SECOND_MS));

      packet_to_send.rpm_set(final_rpm);

      final_rpm = 0;
    }
    else {
      vTaskDelay(100); //need to manage priority w/ delays, first one idea - table with all tasks, and decide then
    }
  }
}
