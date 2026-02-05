/**
 * @file deferred_work.h
 * @brief Deferred Work Queue - blocking 작업을 별도 태스크에서 순차 처리
 *
 * @deprecated 이 모듈은 더 이상 사용하지 않습니다.
 *             대신 event_bus_subscribe_with_queue()를 사용하세요.
 *
 *             새로운 방식:
 *             1. 모듈이 자체 이벤트 큐 생성
 *             2. event_bus_subscribe_with_queue(EVENT_TYPE, my_queue)로 구독
 *             3. 모듈의 자체 태스크에서 큐 처리 (블로킹 OK)
 *
 *             예시: lora_app.c의 lora_event_task 참조
 *
 * ============================================================================
 * [DEPRECATED - 아래는 이전 방식의 문서입니다]
 * ============================================================================
 *
 * Event Bus와 독립적으로 동작하며, 시간이 오래 걸리는 작업
 * (UART 송신 후 응답 대기, TCP 전송 등)을 단일 Worker Task에서 처리합니다.
 *
 * 사용 예:
 *   deferred_work_t work = {
 *       .type = WORK_BLE_SEND_DATA,
 *       .data_len = len
 *   };
 *   memcpy(work.data, buf, len);
 *   deferred_work_post(&work);
 */

#ifndef DEFERRED_WORK_H
#define DEFERRED_WORK_H

#include <stdbool.h>
#include <stdint.h>

/*===========================================================================
 * Configuration
 *===========================================================================*/
#define DEFERRED_WORK_DATA_SIZE     256     /* 작업 데이터 최대 크기 */
#define DEFERRED_WORK_QUEUE_DEPTH   16      /* 큐 깊이 */

/*===========================================================================
 * Work Types
 *===========================================================================*/
typedef enum {
    /*--- BLE 작업 (0x0100 ~ 0x01FF) ---*/
    WORK_BLE_SEND_DATA = 0x0100,
    WORK_BLE_SEND_GGA,
    WORK_BLE_SEND_STATUS,
    WORK_BLE_SEND_CMD_RESPONSE,

    /*--- LTE/GSM 작업 (0x0200 ~ 0x02FF) ---*/
    WORK_LTE_SEND_DATA = 0x0200,
    WORK_LTE_SEND_GGA,
    WORK_LTE_SEND_STATUS,

    /*--- LoRa 작업 (0x0300 ~ 0x03FF) ---*/
    WORK_LORA_SEND_DATA = 0x0300,
    WORK_LORA_SEND_RTCM,
    WORK_LORA_SEND_P2P,

    /*--- RS485 작업 (0x0400 ~ 0x04FF) ---*/
    WORK_RS485_SEND_DATA = 0x0400,
    WORK_RS485_SEND_CMD,

    /*--- RS232 작업 (0x0500 ~ 0x05FF) ---*/
    WORK_RS232_SEND_DATA = 0x0500,

    /*--- FDCAN 작업 (0x0600 ~ 0x06FF) ---*/
    WORK_FDCAN_SEND_FRAME = 0x0600,

    /*--- GPS 작업 (0x0700 ~ 0x07FF) ---*/
    WORK_GPS_SEND_CMD = 0x0700,
    WORK_GPS_SEND_RTCM,

    /*--- Generic/Custom 작업 (0xFF00 ~ 0xFFFF) ---*/
    WORK_CUSTOM = 0xFF00,

    WORK_TYPE_MAX
} work_type_t;

/*===========================================================================
 * Work Item Structure
 *===========================================================================*/
typedef struct {
    work_type_t type;                           /* 작업 타입 */
    uint8_t data[DEFERRED_WORK_DATA_SIZE];      /* 작업 데이터 */
    uint16_t data_len;                          /* 데이터 길이 */
    uint32_t timeout_ms;                        /* 타임아웃 (0 = 기본값 사용) */
    uint8_t priority;                           /* 우선순위 (예약, 현재 미사용) */
    uint8_t retry_count;                        /* 재시도 횟수 (예약) */
} deferred_work_t;

/*===========================================================================
 * Work Handler Callback
 *===========================================================================*/
/**
 * @brief 작업 핸들러 함수 타입
 * @param work 처리할 작업
 * @return true 성공, false 실패
 *
 * 핸들러 내에서 blocking 작업 수행 가능 (UART 응답 대기 등)
 */
typedef bool (*work_handler_t)(const deferred_work_t *work);

/*===========================================================================
 * API
 *===========================================================================*/

/**
 * @brief Deferred Work Queue 초기화
 * @return true 성공
 * @return false 실패 (큐 또는 태스크 생성 실패)
 *
 * main 또는 init 태스크에서 한 번 호출
 */
bool deferred_work_init(void);

/**
 * @brief Deferred Work Queue 종료
 *
 * Worker 태스크 중지 및 리소스 해제
 */
void deferred_work_deinit(void);

/**
 * @brief 작업 핸들러 등록
 * @param type 작업 타입
 * @param handler 핸들러 함수
 * @return true 성공
 * @return false 실패 (이미 등록됨 또는 잘못된 타입)
 *
 * 각 모듈 초기화 시 자신의 작업 타입에 대한 핸들러 등록
 * 예: ble_app_start()에서 WORK_BLE_* 핸들러 등록
 */
bool deferred_work_register_handler(work_type_t type, work_handler_t handler);

/**
 * @brief 작업 핸들러 해제
 * @param type 작업 타입
 */
void deferred_work_unregister_handler(work_type_t type);

/**
 * @brief 작업 등록 (non-blocking)
 * @param work 등록할 작업
 * @return true 성공 (큐에 추가됨)
 * @return false 실패 (큐 가득 참)
 *
 * ISR에서는 호출하지 마세요. 태스크 컨텍스트에서만 사용.
 */
bool deferred_work_post(const deferred_work_t *work);

/**
 * @brief 작업 등록 (timeout 있음)
 * @param work 등록할 작업
 * @param wait_ms 큐 공간 대기 시간 (ms)
 * @return true 성공
 * @return false 실패 (타임아웃)
 */
bool deferred_work_post_timeout(const deferred_work_t *work, uint32_t wait_ms);

/**
 * @brief ISR에서 작업 등록
 * @param work 등록할 작업
 * @param pxHigherPriorityTaskWoken 컨텍스트 스위치 필요 여부
 * @return true 성공
 * @return false 실패
 */
bool deferred_work_post_from_isr(const deferred_work_t *work,
                                  int32_t *pxHigherPriorityTaskWoken);

/**
 * @brief 큐에 대기 중인 작업 수 조회
 * @return 대기 중인 작업 수
 */
uint32_t deferred_work_get_pending_count(void);

/**
 * @brief 큐 비우기
 *
 * 대기 중인 모든 작업 삭제 (주의해서 사용)
 */
void deferred_work_flush(void);

/*===========================================================================
 * Helper Macros
 *===========================================================================*/

/**
 * @brief 간단한 데이터 전송 작업 생성 헬퍼
 */
#define DEFERRED_WORK_INIT(_type, _data_ptr, _data_len) \
    (deferred_work_t) {                                  \
        .type = (_type),                                 \
        .data_len = (_data_len),                         \
        .timeout_ms = 0,                                 \
        .priority = 0,                                   \
        .retry_count = 0                                 \
    }

/**
 * @brief 작업 생성 및 데이터 복사 헬퍼
 */
static inline bool deferred_work_post_data(work_type_t type,
                                           const void *data,
                                           uint16_t len) {
    if (len > DEFERRED_WORK_DATA_SIZE) {
        return false;
    }
    deferred_work_t work = {
        .type = type,
        .data_len = len,
        .timeout_ms = 0
    };
    if (data && len > 0) {
        __builtin_memcpy(work.data, data, len);
    }
    return deferred_work_post(&work);
}

#endif /* DEFERRED_WORK_H */
