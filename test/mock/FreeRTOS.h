/**
 * @file FreeRTOS.h
 * @brief FreeRTOS stub for host testing
 */
#ifndef FREERTOS_H
#define FREERTOS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint32_t TickType_t;
typedef int32_t  BaseType_t;
typedef uint32_t UBaseType_t;
typedef void*    TaskHandle_t;
typedef void*    SemaphoreHandle_t;
typedef void*    QueueHandle_t;

#define pdTRUE          1
#define pdFALSE         0
#define pdPASS          1
#define pdFAIL          0
#define portMAX_DELAY   0xFFFFFFFFUL

#define configSTACK_DEPTH_TYPE uint16_t

#endif /* FREERTOS_H */
