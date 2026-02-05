#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <stdbool.h>
#include <stdint.h>
#include "gps_nmea.h"
#include "FreeRTOS.h"
#include "queue.h"

/*===========================================================================
 * Event Types (Central Definition)
 *===========================================================================*/
typedef enum {
    /* GPS Events (0x0100 ~ 0x01FF) */
    EVENT_GPS_FIX_CHANGED = 0x0100,
    EVENT_GPS_GGA_UPDATE,

    /* NTRIP Events (0x0200 ~ 0x02FF) */
    EVENT_NTRIP_CONNECTED = 0x0200,
    EVENT_NTRIP_DISCONNECTED,

    /* LoRa Events (0x0300 ~ 0x03FF) */
    EVENT_RTCM_FOR_LORA = 0x0300,

    /* BLE Events (0x0400 ~ 0x04FF) */
    /* Reserved for future */

    /* RS485 Events (0x0500 ~ 0x05FF) */
    /* Reserved for future */

    /* RS232 Events (0x0600 ~ 0x06FF) */
    /* Reserved for future */

    /* FDCAN Events (0x0700 ~ 0x07FF) */
    /* Reserved for future */

    /* System Events (0x0F00 ~ 0x0FFF) */
    EVENT_SYSTEM_SHUTDOWN = 0x0F00,

    EVENT_TYPE_MAX
} event_type_t;

/*===========================================================================
 * Event Data Structures
 *===========================================================================*/
typedef struct {
    gps_fix_t fix;
    uint8_t gps_id;
} event_gps_fix_data_t;

typedef struct {
    double lat;
    double lon;
    double alt;
    uint8_t gps_id;
} event_gps_gga_data_t;

typedef struct {
    bool connected;
} event_ntrip_data_t;

typedef struct {
    uint8_t gps_id;
} event_rtcm_data_t;

/**
 * @brief Event structure
 */
typedef struct {
    event_type_t type;
    union {
        event_gps_fix_data_t gps_fix;
        event_gps_gga_data_t gps_gga;
        event_ntrip_data_t ntrip;
        event_rtcm_data_t rtcm;
    } data;
} event_t;

/*===========================================================================
 * Configuration
 *===========================================================================*/
#define EVENT_BUS_MAX_SUBSCRIBERS 16

/*===========================================================================
 * Callback Type
 *===========================================================================*/
/**
 * @brief 이벤트 핸들러 콜백 (디스패처 태스크 컨텍스트에서 실행)
 * @warning 이 핸들러에서 블로킹 작업을 하면 안됩니다!
 *          블로킹이 필요하면 event_bus_subscribe_with_queue()를 사용하세요.
 */
typedef void (*event_handler_t)(const event_t *event, void *user_data);

/*===========================================================================
 * API
 *===========================================================================*/

/**
 * @brief Initialize event bus
 */
void event_bus_init(void);

/**
 * @brief Subscribe to an event type (handler callback)
 *
 * 핸들러는 이벤트 버스 디스패처 태스크에서 실행됩니다.
 * 핸들러 내에서 블로킹 작업을 하면 다른 이벤트 처리가 지연됩니다.
 *
 * @param type Event type to subscribe
 * @param handler Callback function (must be non-blocking!)
 * @param user_data User data passed to callback (can be NULL)
 * @return true Success
 * @return false Failed (max subscribers reached)
 */
bool event_bus_subscribe(event_type_t type, event_handler_t handler, void *user_data);

/**
 * @brief Subscribe to an event type (queue-based)
 *
 * 이벤트가 발생하면 지정된 큐로 event_t가 복사됩니다.
 * 모듈의 자체 태스크에서 큐를 처리하므로 블로킹 작업이 가능합니다.
 *
 * 사용 예:
 *   QueueHandle_t my_queue = xQueueCreate(8, sizeof(event_t));
 *   event_bus_subscribe_with_queue(EVENT_GPS_GGA_UPDATE, my_queue);
 *
 *   // 모듈 태스크에서:
 *   event_t ev;
 *   if (xQueueReceive(my_queue, &ev, portMAX_DELAY) == pdTRUE) {
 *       // 블로킹 작업 OK
 *   }
 *
 * @param type Event type to subscribe
 * @param queue Target queue (must be created with sizeof(event_t))
 * @return true Success
 * @return false Failed
 */
bool event_bus_subscribe_with_queue(event_type_t type, QueueHandle_t queue);

/**
 * @brief Unsubscribe handler from an event type
 *
 * @param type Event type
 * @param handler Callback function to remove
 */
void event_bus_unsubscribe(event_type_t type, event_handler_t handler);

/**
 * @brief Unsubscribe queue from an event type
 *
 * @param type Event type
 * @param queue Queue to remove
 */
void event_bus_unsubscribe_queue(event_type_t type, QueueHandle_t queue);

/**
 * @brief Publish an event (async - enqueues and returns immediately)
 *
 * Event is placed in queue and processed by dispatcher task.
 * - Handler subscribers: handler() called in dispatcher context
 * - Queue subscribers: event copied to target queue
 *
 * Publisher is NOT blocked by handler execution.
 *
 * @param event Event to publish
 */
void event_bus_publish(const event_t *event);

#endif /* EVENT_BUS_H */
