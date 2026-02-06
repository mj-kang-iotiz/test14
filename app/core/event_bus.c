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
#define EVENT_QUEUE_SIZE      16
#define DISPATCHER_STACK_SIZE 512
#define DISPATCHER_PRIORITY   (tskIDLE_PRIORITY + 3)

/*===========================================================================
 * Internal Types
 *===========================================================================*/
typedef struct subscriber_node {
    event_type_t type;
    QueueHandle_t queue;
    struct subscriber_node *next;
} subscriber_node_t;

/*===========================================================================
 * Internal Variables
 *===========================================================================*/
static subscriber_node_t *subscriber_list = NULL; /* 연결 리스트 헤드 */
static uint8_t subscriber_count = 0;              /* 현재 구독자 수 */
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
    subscriber_list = NULL;
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
        BaseType_t ret = xTaskCreate(event_bus_dispatcher_task, "evt_bus", DISPATCHER_STACK_SIZE,
                                     NULL, DISPATCHER_PRIORITY, &dispatcher_task);
        if (ret != pdPASS) {
            LOG_ERR("Failed to create dispatcher task");
            return;
        }
    }

    LOG_INFO("Event bus initialized (max %d subscribers)", EVENT_BUS_MAX_SUBSCRIBERS);
}

bool event_bus_subscribe(event_type_t type, QueueHandle_t queue) {
    if (queue == NULL) {
        return false;
    }

    if (bus_mutex != NULL) {
        xSemaphoreTake(bus_mutex, portMAX_DELAY);
    }

    /* Check subscriber limit */
    if (subscriber_count >= EVENT_BUS_MAX_SUBSCRIBERS) {
        if (bus_mutex != NULL) {
            xSemaphoreGive(bus_mutex);
        }
        LOG_ERR("Max subscribers reached (%d)", EVENT_BUS_MAX_SUBSCRIBERS);
        return false;
    }

    /* Check for duplicate */
    subscriber_node_t *curr = subscriber_list;
    while (curr != NULL) {
        if (curr->type == type && curr->queue == queue) {
            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }
            LOG_WARN("Already subscribed");
            return false;
        }
        curr = curr->next;
    }

    /* Create new node */
    subscriber_node_t *node = pvPortMalloc(sizeof(subscriber_node_t));
    if (node == NULL) {
        if (bus_mutex != NULL) {
            xSemaphoreGive(bus_mutex);
        }
        LOG_ERR("Failed to allocate subscriber node");
        return false;
    }

    node->type = type;
    node->queue = queue;

    /* Add to head of list */
    node->next = subscriber_list;
    subscriber_list = node;
    subscriber_count++;

    if (bus_mutex != NULL) {
        xSemaphoreGive(bus_mutex);
    }

    LOG_DEBUG("Subscribed to event 0x%04X (%d/%d)", type, subscriber_count,
              EVENT_BUS_MAX_SUBSCRIBERS);
    return true;
}

void event_bus_unsubscribe(event_type_t type, QueueHandle_t queue) {
    if (bus_mutex != NULL) {
        xSemaphoreTake(bus_mutex, portMAX_DELAY);
    }

    subscriber_node_t *prev = NULL;
    subscriber_node_t *curr = subscriber_list;

    while (curr != NULL) {
        if (curr->type == type && curr->queue == queue) {
            /* Remove from list */
            if (prev == NULL) {
                subscriber_list = curr->next;
            }
            else {
                prev->next = curr->next;
            }

            vPortFree(curr);
            subscriber_count--;

            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }

            LOG_DEBUG("Unsubscribed from event 0x%04X (%d/%d)", type, subscriber_count,
                      EVENT_BUS_MAX_SUBSCRIBERS);
            return;
        }
        prev = curr;
        curr = curr->next;
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
            subscriber_node_t *curr = subscriber_list;
            while (curr != NULL) {
                if (curr->type == event.type) {
                    /* Non-blocking send to target queue */
                    if (xQueueSend(curr->queue, &event, 0) != pdTRUE) {
                        LOG_WARN("Target queue full for event 0x%04X", event.type);
                    }
                }
                curr = curr->next;
            }

            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }
        }
    }
}
