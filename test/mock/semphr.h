/**
 * @file semphr.h
 * @brief FreeRTOS semaphore stub for host testing
 */
#ifndef SEMPHR_H
#define SEMPHR_H

#include "FreeRTOS.h"

static inline SemaphoreHandle_t xSemaphoreCreateMutex(void) { return NULL; }
static inline SemaphoreHandle_t xSemaphoreCreateBinary(void) { return NULL; }
static inline SemaphoreHandle_t xSemaphoreCreateCounting(UBaseType_t max, UBaseType_t init) {
    (void)max; (void)init; return NULL;
}
static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t s, TickType_t t) {
    (void)s; (void)t; return pdTRUE;
}
static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t s) {
    (void)s; return pdTRUE;
}
static inline BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t s,
                                               BaseType_t *pxHigherPriorityTaskWoken) {
    (void)s; (void)pxHigherPriorityTaskWoken; return pdTRUE;
}
static inline void vSemaphoreDelete(SemaphoreHandle_t s) { (void)s; }

#endif /* SEMPHR_H */
