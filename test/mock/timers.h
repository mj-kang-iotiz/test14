/**
 * @file timers.h
 * @brief FreeRTOS timers stub for host testing
 */
#ifndef TIMERS_H
#define TIMERS_H

#include "FreeRTOS.h"

typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

static inline TimerHandle_t xTimerCreate(const char *pcTimerName,
                                         TickType_t xTimerPeriodInTicks,
                                         UBaseType_t uxAutoReload,
                                         void *pvTimerID,
                                         TimerCallbackFunction_t pxCallbackFunction) {
    (void)pcTimerName; (void)xTimerPeriodInTicks; (void)uxAutoReload;
    (void)pvTimerID; (void)pxCallbackFunction;
    return NULL;
}

static inline BaseType_t xTimerStart(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    (void)xTimer; (void)xTicksToWait; return pdPASS;
}

static inline BaseType_t xTimerStop(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    (void)xTimer; (void)xTicksToWait; return pdPASS;
}

static inline BaseType_t xTimerDelete(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    (void)xTimer; (void)xTicksToWait; return pdPASS;
}

static inline BaseType_t xTimerReset(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    (void)xTimer; (void)xTicksToWait; return pdPASS;
}

static inline BaseType_t xTimerIsTimerActive(TimerHandle_t xTimer) {
    (void)xTimer; return pdFALSE;
}

static inline void vTimerSetTimerID(TimerHandle_t xTimer, void *pvNewID) {
    (void)xTimer; (void)pvNewID;
}

static inline void *pvTimerGetTimerID(TimerHandle_t xTimer) {
    (void)xTimer; return NULL;
}

#endif /* TIMERS_H */
