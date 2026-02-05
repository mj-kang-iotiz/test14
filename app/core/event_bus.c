#include "event_bus.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

#ifndef TAG
#define TAG "EVENT_BUS"
#endif

#include "log.h"

/*===========================================================================
 * Configuration
 *===========================================================================*/
#define EVENT_QUEUE_SIZE        16
#define DISPATCHER_STACK_SIZE   512
#define DISPATCHER_PRIORITY     (tskIDLE_PRIORITY + 3)

/*===========================================================================
 * Internal Types
 *===========================================================================*/
typedef struct {
    event_type_t type;
    event_handler_t handler;    /* 핸들러 콜백 (queue가 NULL일 때 사용) */
    QueueHandle_t queue;        /* 타겟 큐 (handler가 NULL일 때 사용) */
    void *user_data;
    bool active;
} subscriber_t;

/*===========================================================================
 * Internal Variables
 *===========================================================================*/
static subscriber_t subscribers[EVENT_BUS_MAX_SUBSCRIBERS];
static uint8_t subscriber_count = 0;
static SemaphoreHandle_t bus_mutex = NULL;

/* Async event queue and dispatcher task */
static QueueHandle_t event_queue = NULL;
static TaskHandle_t dispatcher_task = NULL;

/*===========================================================================
 * Forward Declarations
 *===========================================================================*/
static void event_bus_dispatcher_task(void *param);

/*===========================================================================
 * Implementation
 *===========================================================================*/

void event_bus_init(void) {
    memset(subscribers, 0, sizeof(subscribers));
    subscriber_count = 0;

    /* Create mutex for subscriber list protection */
    if (bus_mutex == NULL) {
        bus_mutex = xSemaphoreCreateMutex();
        if (bus_mutex == NULL) {
            LOG_ERR("Failed to create mutex");
            return;
        }
    }

    /* Create event queue */
    if (event_queue == NULL) {
        event_queue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(event_t));
        if (event_queue == NULL) {
            LOG_ERR("Failed to create event queue");
            return;
        }
    }

    /* Create dispatcher task */
    if (dispatcher_task == NULL) {
        BaseType_t ret = xTaskCreate(event_bus_dispatcher_task,
                                      "evt_bus",
                                      DISPATCHER_STACK_SIZE,
                                      NULL,
                                      DISPATCHER_PRIORITY,
                                      &dispatcher_task);
        if (ret != pdPASS) {
            LOG_ERR("Failed to create dispatcher task");
            return;
        }
    }

    LOG_INFO("Event bus initialized (async mode)");
}

/**
 * @brief 내부 함수: 빈 슬롯 찾기
 * @note bus_mutex가 이미 획득된 상태에서 호출
 */
static int find_empty_slot(void) {
    for (uint8_t i = 0; i < subscriber_count; i++) {
        if (!subscribers[i].active) {
            return i;
        }
    }
    if (subscriber_count >= EVENT_BUS_MAX_SUBSCRIBERS) {
        return -1;
    }
    return subscriber_count++;
}

bool event_bus_subscribe(event_type_t type, event_handler_t handler, void *user_data) {
    if (handler == NULL) {
        return false;
    }

    if (bus_mutex != NULL) {
        xSemaphoreTake(bus_mutex, portMAX_DELAY);
    }

    /* Check for duplicate */
    for (uint8_t i = 0; i < subscriber_count; i++) {
        if (subscribers[i].active &&
            subscribers[i].type == type &&
            subscribers[i].handler == handler) {
            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }
            LOG_WARN("Already subscribed");
            return false;
        }
    }

    /* Find empty slot */
    int slot = find_empty_slot();
    if (slot < 0) {
        if (bus_mutex != NULL) {
            xSemaphoreGive(bus_mutex);
        }
        LOG_ERR("Max subscribers reached");
        return false;
    }

    subscribers[slot].type = type;
    subscribers[slot].handler = handler;
    subscribers[slot].queue = NULL;
    subscribers[slot].user_data = user_data;
    subscribers[slot].active = true;

    if (bus_mutex != NULL) {
        xSemaphoreGive(bus_mutex);
    }

    LOG_DEBUG("Subscribed to event 0x%04X (handler)", type);
    return true;
}

bool event_bus_subscribe_with_queue(event_type_t type, QueueHandle_t queue) {
    if (queue == NULL) {
        return false;
    }

    if (bus_mutex != NULL) {
        xSemaphoreTake(bus_mutex, portMAX_DELAY);
    }

    /* Check for duplicate */
    for (uint8_t i = 0; i < subscriber_count; i++) {
        if (subscribers[i].active &&
            subscribers[i].type == type &&
            subscribers[i].queue == queue) {
            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }
            LOG_WARN("Queue already subscribed");
            return false;
        }
    }

    /* Find empty slot */
    int slot = find_empty_slot();
    if (slot < 0) {
        if (bus_mutex != NULL) {
            xSemaphoreGive(bus_mutex);
        }
        LOG_ERR("Max subscribers reached");
        return false;
    }

    subscribers[slot].type = type;
    subscribers[slot].handler = NULL;
    subscribers[slot].queue = queue;
    subscribers[slot].user_data = NULL;
    subscribers[slot].active = true;

    if (bus_mutex != NULL) {
        xSemaphoreGive(bus_mutex);
    }

    LOG_DEBUG("Subscribed to event 0x%04X (queue)", type);
    return true;
}

void event_bus_unsubscribe(event_type_t type, event_handler_t handler) {
    if (bus_mutex != NULL) {
        xSemaphoreTake(bus_mutex, portMAX_DELAY);
    }

    for (uint8_t i = 0; i < subscriber_count; i++) {
        if (subscribers[i].active &&
            subscribers[i].type == type &&
            subscribers[i].handler == handler) {
            subscribers[i].active = false;
            LOG_DEBUG("Unsubscribed from event 0x%04X (handler)", type);
            break;
        }
    }

    if (bus_mutex != NULL) {
        xSemaphoreGive(bus_mutex);
    }
}

void event_bus_unsubscribe_queue(event_type_t type, QueueHandle_t queue) {
    if (bus_mutex != NULL) {
        xSemaphoreTake(bus_mutex, portMAX_DELAY);
    }

    for (uint8_t i = 0; i < subscriber_count; i++) {
        if (subscribers[i].active &&
            subscribers[i].type == type &&
            subscribers[i].queue == queue) {
            subscribers[i].active = false;
            LOG_DEBUG("Unsubscribed from event 0x%04X (queue)", type);
            break;
        }
    }

    if (bus_mutex != NULL) {
        xSemaphoreGive(bus_mutex);
    }
}

void event_bus_publish(const event_t *event) {
    if (event == NULL || event_queue == NULL) {
        return;
    }

    /* Non-blocking: just enqueue and return immediately */
    if (xQueueSend(event_queue, event, pdMS_TO_TICKS(10)) != pdTRUE) {
        LOG_WARN("Event queue full, event 0x%04X dropped", event->type);
    }
}

/**
 * @brief Dispatcher task - dequeues events and dispatches to subscribers
 *
 * - Handler subscribers: handler() called in dispatcher context
 * - Queue subscribers: event copied to target queue (non-blocking)
 *
 * Runs in its own task context, so handlers don't block publishers.
 */
static void event_bus_dispatcher_task(void *param) {
    (void)param;
    event_t event;

    LOG_INFO("Event bus dispatcher started");

    while (1) {
        /* Block until event available */
        if (xQueueReceive(event_queue, &event, portMAX_DELAY) == pdTRUE) {

            /* Take mutex to safely iterate subscribers */
            if (bus_mutex != NULL) {
                xSemaphoreTake(bus_mutex, portMAX_DELAY);
            }

            /* Dispatch to all matching subscribers */
            for (uint8_t i = 0; i < subscriber_count; i++) {
                if (subscribers[i].active && subscribers[i].type == event.type) {

                    if (subscribers[i].queue != NULL) {
                        /* Queue-based subscriber: copy event to target queue */
                        QueueHandle_t target_queue = subscribers[i].queue;

                        /* Release mutex before queue operation */
                        if (bus_mutex != NULL) {
                            xSemaphoreGive(bus_mutex);
                        }

                        /* Non-blocking send to target queue */
                        if (xQueueSend(target_queue, &event, 0) != pdTRUE) {
                            LOG_WARN("Target queue full for event 0x%04X", event.type);
                        }

                        /* Re-acquire mutex */
                        if (bus_mutex != NULL) {
                            xSemaphoreTake(bus_mutex, portMAX_DELAY);
                        }

                    } else if (subscribers[i].handler != NULL) {
                        /* Handler-based subscriber: call handler directly */
                        event_handler_t handler = subscribers[i].handler;
                        void *user_data = subscribers[i].user_data;

                        /* Release mutex before calling handler */
                        if (bus_mutex != NULL) {
                            xSemaphoreGive(bus_mutex);
                        }

                        handler(&event, user_data);

                        /* Re-acquire mutex for next iteration */
                        if (bus_mutex != NULL) {
                            xSemaphoreTake(bus_mutex, portMAX_DELAY);
                        }
                    }
                }
            }

            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }
        }
    }
}
