#pragma once
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "packet.hpp"

//=================================================================
// RPM COUNTING CONSTANTS
//=================================================================

#define AMOUNT_OF_WINGS 2
#define ONE_SECOND_MS   1000000

//=================================================================
// ISR PINS
//=================================================================

#define ESP_INTR_FLAG_DEFAULT \
  0  // I think that flag is used to give us ability to handle all the GPIO
     // separetely
#define GPIO_INPUT_IO_0    GPIO_NUM_4  // 4
#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_INPUT_IO_0)

/*
 * I suppose, that 1ULL - all possible pins, so be careful
 * Use bitwise (<<,>>, |, & - mostly) operations to get desired pin
 * */

//=================================================================
// RPM COUNTING FUNCTIONS
//=================================================================

void IRAM_ATTR gpio_rotation_isr_handler(void* arg);

void periodic_timer_callback(void* arg);

void rpm_safe_writing_task(void* arg);

//=================================================================
// RPM VARIABLES
//=================================================================

extern uint64_t time_rpm;
extern uint64_t rotation_count;
extern uint64_t final_rpm;