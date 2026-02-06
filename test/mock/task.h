/**
 * @file task.h
 * @brief FreeRTOS task.h stub for host testing
 */
#ifndef TASK_H
#define TASK_H

#include "FreeRTOS.h"

#define tskIDLE_PRIORITY ((UBaseType_t)0U)

typedef void (*TaskFunction_t)(void *);

typedef enum { eRunning = 0, eReady, eBlocked, eSuspended, eDeleted, eInvalid } eTaskState;

/* Stub: returns configurable mock tick value */
extern uint32_t mock_tick_count;
static inline TickType_t xTaskGetTickCount(void) {
    return mock_tick_count;
}

static inline BaseType_t xTaskCreate(void *pxTaskCode, const char *pcName,
                                     configSTACK_DEPTH_TYPE usStackDepth, void *pvParameters,
                                     UBaseType_t uxPriority, TaskHandle_t *pxCreatedTask) {
    (void)pxTaskCode;
    (void)pcName;
    (void)usStackDepth;
    (void)pvParameters;
    (void)uxPriority;
    (void)pxCreatedTask;
    return pdPASS;
}

static inline void vTaskDelete(TaskHandle_t xTask) {
    (void)xTask;
}
static inline void vTaskDelay(TickType_t xTicksToDelay) {
    (void)xTicksToDelay;
}
static inline void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement) {
    (void)pxPreviousWakeTime;
    (void)xTimeIncrement;
}
static inline void vTaskSuspend(TaskHandle_t xTask) {
    (void)xTask;
}
static inline void vTaskResume(TaskHandle_t xTask) {
    (void)xTask;
}
static inline eTaskState eTaskGetState(TaskHandle_t xTask) {
    (void)xTask;
    return eDeleted;
}

#endif /* TASK_H */
