#pragma once
extern "C"
{
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sys/socket.h"
#include "netdb.h"
#include "errno.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "syscalls.c"
#include "fcntl.h"
}

#define INVALID_SOCK (-1)

#define YIELD_TO_ALL_MS 50

//=================================================================
// TASK HANDLERS(For notifies)
//=================================================================

static TaskHandle_t VOLTAGE_TASK_HANDLE = NULL;
static TaskHandle_t CURRENT_TASK_HANDLE = NULL;
static TaskHandle_t DISTURBNCE_TASK_HANDLE = NULL;

static TaskHandle_t TEMPERATURE1_TASK_HANDLE = NULL;
static TaskHandle_t TEMPERATURE2_TASK_HANDLE = NULL;
static TaskHandle_t TEMPERATURE3_TASK_HANDLE = NULL;

/*
static TaskHandle_t VOLTAGE_TASK_HANDLE = NULL;
static TaskHandle_t VOLTAGE_TASK_HANDLE = NULL;
static TaskHandle_t VOLTAGE_TASK_HANDLE = NULL; I DON'T REMEMBER FOR WHAT IT IS
*/

static TaskHandle_t WEIGHT_TASK_HANDLE = NULL;
static TaskHandle_t RPM_TASK_HANDLE = NULL;

//==================================================================
//
//==================================================================

static void log_socket_error(const char *tag, const int sock, const int err, const char *message);

static int try_receive(const char *tag, const int sock, char * data, size_t max_len);

static int socket_send(const char *tag, const int sock, const char * data, const size_t len);

static void tcp_client_task(void *pvParameters);