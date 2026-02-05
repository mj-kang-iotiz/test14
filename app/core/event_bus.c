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
typedef struct subscriber_node {
    event_type_t type;
    event_handler_t handler;    /* 핸들러 콜백 (queue가 NULL일 때 사용) */
    QueueHandle_t queue;        /* 타겟 큐 (handler가 NULL일 때 사용) */
    void *user_data;
    struct subscriber_node *next;
} subscriber_node_t;

/*===========================================================================
 * Internal Variables
 *===========================================================================*/
static subscriber_node_t *subscriber_list = NULL;   /* 연결 리스트 헤드 */
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
 * @brief 내부 함수: 새 노드 생성
 * @return 새 노드 포인터 (실패 시 NULL)
 */
static subscriber_node_t *create_node(void) {
    subscriber_node_t *node = pvPortMalloc(sizeof(subscriber_node_t));
    if (node != NULL) {
        node->next = NULL;
    }
    return node;
}

/**
 * @brief 내부 함수: 노드 해제
 */
static void free_node(subscriber_node_t *node) {
    if (node != NULL) {
        vPortFree(node);
    }
}

bool event_bus_subscribe(event_type_t type, event_handler_t handler, void *user_data) {
    if (handler == NULL) {
        return false;
    }

    if (bus_mutex != NULL) {
        xSemaphoreTake(bus_mutex, portMAX_DELAY);
    }

    /* Check for duplicate */
    subscriber_node_t *curr = subscriber_list;
    while (curr != NULL) {
        if (curr->type == type && curr->handler == handler) {
            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }
            LOG_WARN("Already subscribed");
            return false;
        }
        curr = curr->next;
    }

    /* Create new node */
    subscriber_node_t *node = create_node();
    if (node == NULL) {
        if (bus_mutex != NULL) {
            xSemaphoreGive(bus_mutex);
        }
        LOG_ERR("Failed to allocate subscriber node");
        return false;
    }

    node->type = type;
    node->handler = handler;
    node->queue = NULL;
    node->user_data = user_data;

    /* Add to head of list */
    node->next = subscriber_list;
    subscriber_list = node;

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
    subscriber_node_t *curr = subscriber_list;
    while (curr != NULL) {
        if (curr->type == type && curr->queue == queue) {
            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }
            LOG_WARN("Queue already subscribed");
            return false;
        }
        curr = curr->next;
    }

    /* Create new node */
    subscriber_node_t *node = create_node();
    if (node == NULL) {
        if (bus_mutex != NULL) {
            xSemaphoreGive(bus_mutex);
        }
        LOG_ERR("Failed to allocate subscriber node");
        return false;
    }

    node->type = type;
    node->handler = NULL;
    node->queue = queue;
    node->user_data = NULL;

    /* Add to head of list */
    node->next = subscriber_list;
    subscriber_list = node;

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

    subscriber_node_t *prev = NULL;
    subscriber_node_t *curr = subscriber_list;

    while (curr != NULL) {
        if (curr->type == type && curr->handler == handler) {
            /* Remove from list */
            if (prev == NULL) {
                subscriber_list = curr->next;
            } else {
                prev->next = curr->next;
            }

            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }

            free_node(curr);
            LOG_DEBUG("Unsubscribed from event 0x%04X (handler)", type);
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    if (bus_mutex != NULL) {
        xSemaphoreGive(bus_mutex);
    }
}

void event_bus_unsubscribe_queue(event_type_t type, QueueHandle_t queue) {
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
            } else {
                prev->next = curr->next;
            }

            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }

            free_node(curr);
            LOG_DEBUG("Unsubscribed from event 0x%04X (queue)", type);
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

            /* Dispatch to all matching subscribers (mutex held during iteration) */
            subscriber_node_t *curr = subscriber_list;
            while (curr != NULL) {
                subscriber_node_t *next = curr->next;  /* Save next before potential changes */

                if (curr->type == event.type) {

                    if (curr->queue != NULL) {
                        /* Queue-based subscriber: copy event to target queue */
                        /* Non-blocking send to target queue */
                        if (xQueueSend(curr->queue, &event, 0) != pdTRUE) {
                            LOG_WARN("Target queue full for event 0x%04X", event.type);
                        }

                    } else if (curr->handler != NULL) {
                        /* Handler-based subscriber: call handler directly */
                        event_handler_t handler = curr->handler;
                        void *user_data = curr->user_data;

                        /* Release mutex before calling handler (may block briefly) */
                        if (bus_mutex != NULL) {
                            xSemaphoreGive(bus_mutex);
                        }

                        handler(&event, user_data);

                        /* Re-acquire mutex for next iteration */
                        if (bus_mutex != NULL) {
                            xSemaphoreTake(bus_mutex, portMAX_DELAY);
                        }

                        /* Note: list may have changed, but we saved 'next' */
                    }
                }
                curr = next;
            }

            if (bus_mutex != NULL) {
                xSemaphoreGive(bus_mutex);
            }
        }
    }
}
