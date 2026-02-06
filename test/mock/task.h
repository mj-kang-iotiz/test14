/**
 * @file task.h
 * @brief FreeRTOS task.h stub for host testing
 */
#ifndef TASK_H
#define TASK_H

#include "FreeRTOS.h"

/* Stub: returns configurable mock tick value */
extern uint32_t mock_tick_count;
static inline TickType_t xTaskGetTickCount(void) { return mock_tick_count; }

static inline BaseType_t xTaskCreate(void *pxTaskCode, const char *pcName,
                                     configSTACK_DEPTH_TYPE usStackDepth,
                                     void *pvParameters, UBaseType_t uxPriority,
                                     TaskHandle_t *pxCreatedTask) {
    (void)pxTaskCode; (void)pcName; (void)usStackDepth;
    (void)pvParameters; (void)uxPriority; (void)pxCreatedTask;
    return pdPASS;
}

static inline void vTaskDelete(TaskHandle_t xTask) { (void)xTask; }

#endif /* TASK_H */
