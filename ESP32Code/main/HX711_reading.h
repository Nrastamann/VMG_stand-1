#pragma once

#include "HX711.h"
#include "rpm_counter.h"
//=================================================================
// HX711 CONSTANTS
//=================================================================

#define GPIO_SCALES_DATA GPIO_NUM_16
#define GPIO_SCLK GPIO_NUM_17
#define AVG_SAMPLES 10

static void weight_reading_task(void *arg);