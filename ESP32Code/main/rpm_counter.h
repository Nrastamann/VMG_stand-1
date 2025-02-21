#pragma once

#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

//=================================================================
// RPM COUNTING CONSTANTS
//=================================================================

#define AMOUNT_OF_WINGS 2
#define ONE_SECOND_MS 1000000

//=================================================================
// ISR PINS
//=================================================================

#define ESP_INTR_FLAG_DEFAULT 0    // I think that flag is used to give us ability to handle all the GPIO separetely
#define GPIO_INPUT_IO_0 GPIO_NUM_4 // 4
#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_INPUT_IO_0)

/*
 * I suppose, that 1ULL - all possible pins, so be careful
 * Use bitwise (<<,>>, |, & - mostly) operations to get desired pin
 * */

//=================================================================
// RPM COUNTING FUNCTIONS
//=================================================================

static void IRAM_ATTR gpio_rotation_isr_handler(void *arg);

static void periodic_timer_callback(void *arg);

static void rpm_safe_writing_task(void *arg);

//=================================================================
// RPM VARIABLES
//=================================================================

static uint64_t time_rpm = 0;
static uint16_t rotation_count = 0;
static uint64_t final_rpm = 0;