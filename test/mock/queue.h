/**
 * @file queue.h
 * @brief FreeRTOS queue stub for host testing
 */
#ifndef QUEUE_H
#define QUEUE_H

#include "FreeRTOS.h"

static inline QueueHandle_t xQueueCreate(UBaseType_t len, UBaseType_t size) {
    (void)len; (void)size; return NULL;
}
static inline BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t t) {
    (void)q; (void)item; (void)t; return pdTRUE;
}
static inline BaseType_t xQueueReceive(QueueHandle_t q, void *buf, TickType_t t) {
    (void)q; (void)buf; (void)t; return pdFALSE;
}
static inline void vQueueDelete(QueueHandle_t q) { (void)q; }
static inline BaseType_t xQueueSendFromISR(QueueHandle_t q, const void *item,
                                           BaseType_t *pxHigherPriorityTaskWoken) {
    (void)q; (void)item; (void)pxHigherPriorityTaskWoken; return pdTRUE;
}
static inline BaseType_t xQueueReceiveFromISR(QueueHandle_t q, void *buf,
                                              BaseType_t *pxHigherPriorityTaskWoken) {
    (void)q; (void)buf; (void)pxHigherPriorityTaskWoken; return pdFALSE;
}

#endif /* QUEUE_H */
