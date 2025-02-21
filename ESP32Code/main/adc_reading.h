#pragma once
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


//================================================
// ADC VARIABLES
//================================================

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

static const char *ADC_TAG = "ADC_READINGS";

//================================================
// ADC FUNCTIONS
//================================================

static void adc_reading_task(void *arg);

static void oneshot_adc_init(adc_channel_t *channel, uint8_t channel_num);