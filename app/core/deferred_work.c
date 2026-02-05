/**
 * @file deferred_work.c
 * @brief Deferred Work Queue 구현
 */

#include "deferred_work.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

#ifndef TAG
#define TAG "DEFERRED"
#endif

#include "log.h"

/*===========================================================================
 * Configuration
 *===========================================================================*/
#define WORKER_STACK_SIZE       512
#define WORKER_PRIORITY         (tskIDLE_PRIORITY + 1)
#define WORKER_TASK_NAME        "worker"

/*===========================================================================
 * Handler Table Configuration
 *===========================================================================*/
/**
 * 핸들러 테이블 구조:
 * - 각 카테고리별로 base + offset 방식으로 인덱싱
 * - 메모리 효율을 위해 카테고리별 최대 16개 타입 지원
 */
#define HANDLER_CATEGORY_COUNT  8       /* 카테고리 수 (0x01 ~ 0x08) */
#define HANDLER_PER_CATEGORY    16      /* 카테고리당 핸들러 수 */
#define HANDLER_TABLE_SIZE      (HANDLER_CATEGORY_COUNT * HANDLER_PER_CATEGORY)

/*===========================================================================
 * Internal Variables
 *===========================================================================*/
static QueueHandle_t work_queue = NULL;
static TaskHandle_t worker_task_handle = NULL;
static volatile bool worker_running = false;

/* 핸들러 테이블 */
static work_handler_t handler_table[HANDLER_TABLE_SIZE] = {0};
static SemaphoreHandle_t handler_mutex = NULL;

/*===========================================================================
 * Internal Functions
 *===========================================================================*/

/**
 * @brief work_type_t를 핸들러 테이블 인덱스로 변환
 * @param type 작업 타입
 * @return 인덱스 (유효하지 않으면 -1)
 */
static int type_to_index(work_type_t type) {
    uint16_t category = (type >> 8) & 0xFF;     /* 상위 바이트: 카테고리 */
    uint16_t offset = type & 0xFF;               /* 하위 바이트: 오프셋 */

    /* 카테고리 범위 검사 (0x01 ~ HANDLER_CATEGORY_COUNT) */
    if (category < 1 || category > HANDLER_CATEGORY_COUNT) {
        /* Custom 타입 (0xFF00~)은 마지막 슬롯 사용 */
        if (category == 0xFF && offset < HANDLER_PER_CATEGORY) {
            return (HANDLER_CATEGORY_COUNT - 1) * HANDLER_PER_CATEGORY + offset;
        }
        return -1;
    }

    /* 오프셋 범위 검사 */
    if (offset >= HANDLER_PER_CATEGORY) {
        return -1;
    }

    return (category - 1) * HANDLER_PER_CATEGORY + offset;
}

/**
 * @brief Worker Task - 큐에서 작업을 꺼내 핸들러 호출
 */
static void worker_task_func(void *param) {
    (void)param;
    deferred_work_t work;

    LOG_INFO("Worker task started");

    while (worker_running) {
        /* 큐에서 작업 대기 (blocking) */
        if (xQueueReceive(work_queue, &work, portMAX_DELAY) == pdTRUE) {
            int idx = type_to_index(work.type);

            if (idx >= 0 && handler_table[idx] != NULL) {
                /* 핸들러 호출 - blocking 작업 OK */
                bool result = handler_table[idx](&work);

                if (!result) {
                    LOG_WARN("Work 0x%04X failed", work.type);
                }
            } else {
                LOG_WARN("No handler for work type 0x%04X", work.type);
            }
        }
    }

    LOG_INFO("Worker task stopped");
    vTaskDelete(NULL);
}

/*===========================================================================
 * Public API
 *===========================================================================*/

bool deferred_work_init(void) {
    /* 이미 초기화된 경우 */
    if (work_queue != NULL) {
        LOG_WARN("Already initialized");
        return true;
    }

    /* 핸들러 테이블 초기화 */
    memset(handler_table, 0, sizeof(handler_table));

    /* 뮤텍스 생성 */
    handler_mutex = xSemaphoreCreateMutex();
    if (handler_mutex == NULL) {
        LOG_ERR("Failed to create handler mutex");
        return false;
    }

    /* 작업 큐 생성 */
    work_queue = xQueueCreate(DEFERRED_WORK_QUEUE_DEPTH, sizeof(deferred_work_t));
    if (work_queue == NULL) {
        LOG_ERR("Failed to create work queue");
        vSemaphoreDelete(handler_mutex);
        handler_mutex = NULL;
        return false;
    }

    /* Worker 태스크 생성 */
    worker_running = true;
    BaseType_t ret = xTaskCreate(
        worker_task_func,
        WORKER_TASK_NAME,
        WORKER_STACK_SIZE,
        NULL,
        WORKER_PRIORITY,
        &worker_task_handle
    );

    if (ret != pdPASS) {
        LOG_ERR("Failed to create worker task");
        vQueueDelete(work_queue);
        work_queue = NULL;
        vSemaphoreDelete(handler_mutex);
        handler_mutex = NULL;
        worker_running = false;
        return false;
    }

    LOG_INFO("Deferred work queue initialized (depth=%d)", DEFERRED_WORK_QUEUE_DEPTH);
    return true;
}

void deferred_work_deinit(void) {
    if (!worker_running) {
        return;
    }

    /* Worker 태스크 중지 요청 */
    worker_running = false;

    /* 더미 작업을 보내서 태스크가 종료 조건을 확인하도록 함 */
    if (work_queue != NULL) {
        deferred_work_t dummy = {.type = WORK_TYPE_MAX};
        xQueueSend(work_queue, &dummy, pdMS_TO_TICKS(100));
    }

    /* 태스크 종료 대기 */
    vTaskDelay(pdMS_TO_TICKS(100));

    /* 리소스 해제 */
    if (work_queue != NULL) {
        vQueueDelete(work_queue);
        work_queue = NULL;
    }

    if (handler_mutex != NULL) {
        vSemaphoreDelete(handler_mutex);
        handler_mutex = NULL;
    }

    worker_task_handle = NULL;

    LOG_INFO("Deferred work queue deinitialized");
}

bool deferred_work_register_handler(work_type_t type, work_handler_t handler) {
    if (handler == NULL) {
        return false;
    }

    int idx = type_to_index(type);
    if (idx < 0) {
        LOG_ERR("Invalid work type 0x%04X", type);
        return false;
    }

    if (handler_mutex != NULL) {
        xSemaphoreTake(handler_mutex, portMAX_DELAY);
    }

    if (handler_table[idx] != NULL) {
        LOG_WARN("Handler for 0x%04X already registered, replacing", type);
    }

    handler_table[idx] = handler;

    if (handler_mutex != NULL) {
        xSemaphoreGive(handler_mutex);
    }

    LOG_DEBUG("Handler registered for work type 0x%04X", type);
    return true;
}

void deferred_work_unregister_handler(work_type_t type) {
    int idx = type_to_index(type);
    if (idx < 0) {
        return;
    }

    if (handler_mutex != NULL) {
        xSemaphoreTake(handler_mutex, portMAX_DELAY);
    }

    handler_table[idx] = NULL;

    if (handler_mutex != NULL) {
        xSemaphoreGive(handler_mutex);
    }

    LOG_DEBUG("Handler unregistered for work type 0x%04X", type);
}

bool deferred_work_post(const deferred_work_t *work) {
    if (work == NULL || work_queue == NULL) {
        return false;
    }

    /* Non-blocking post */
    if (xQueueSend(work_queue, work, 0) != pdTRUE) {
        LOG_WARN("Work queue full, work 0x%04X dropped", work->type);
        return false;
    }

    return true;
}

bool deferred_work_post_timeout(const deferred_work_t *work, uint32_t wait_ms) {
    if (work == NULL || work_queue == NULL) {
        return false;
    }

    if (xQueueSend(work_queue, work, pdMS_TO_TICKS(wait_ms)) != pdTRUE) {
        LOG_WARN("Work queue full (timeout), work 0x%04X dropped", work->type);
        return false;
    }

    return true;
}

bool deferred_work_post_from_isr(const deferred_work_t *work,
                                  int32_t *pxHigherPriorityTaskWoken) {
    if (work == NULL || work_queue == NULL) {
        return false;
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t ret = xQueueSendFromISR(work_queue, work, &xHigherPriorityTaskWoken);

    if (pxHigherPriorityTaskWoken != NULL) {
        *pxHigherPriorityTaskWoken = xHigherPriorityTaskWoken;
    }

    return ret == pdTRUE;
}

uint32_t deferred_work_get_pending_count(void) {
    if (work_queue == NULL) {
        return 0;
    }
    return (uint32_t)uxQueueMessagesWaiting(work_queue);
}

void deferred_work_flush(void) {
    if (work_queue == NULL) {
        return;
    }

    deferred_work_t work;
    while (xQueueReceive(work_queue, &work, 0) == pdTRUE) {
        /* 버림 */
    }

    LOG_INFO("Work queue flushed");
}
