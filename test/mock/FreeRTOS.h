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
typedef void    *TaskHandle_t;
typedef void    *SemaphoreHandle_t;
typedef void    *QueueHandle_t;
typedef void    *TimerHandle_t;

#define pdTRUE          1
#define pdFALSE         0
#define pdPASS          1
#define pdFAIL          0
#define portMAX_DELAY   0xFFFFFFFFUL

#define pdMS_TO_TICKS(xTimeInMs)  ((TickType_t)(xTimeInMs))

#define configSTACK_DEPTH_TYPE uint16_t

static inline void *pvPortMalloc(size_t xSize) { return malloc(xSize); }
static inline void  vPortFree(void *pv) { free(pv); }

#endif /* FREERTOS_H */
