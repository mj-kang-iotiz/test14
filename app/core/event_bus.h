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
#define EVENT_BUS_MAX_SUBSCRIBERS 16 /* 최대 구독자 수 */

/*===========================================================================
 * API
 *===========================================================================*/

/**
 * @brief Initialize event bus
 */
void event_bus_init(void);

/**
 * @brief Subscribe to an event type
 *
 * 이벤트가 발생하면 지정된 큐로 event_t가 복사됩니다.
 * 모듈의 자체 태스크에서 큐를 처리하므로 블로킹 작업이 가능합니다.
 *
 * 사용 예:
 *   QueueHandle_t my_queue = xQueueCreate(8, sizeof(event_t));
 *   event_bus_subscribe(EVENT_GPS_GGA_UPDATE, my_queue);
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
 * @return false Failed (max subscribers reached or duplicate)
 */
bool event_bus_subscribe(event_type_t type, QueueHandle_t queue);

/**
 * @brief Unsubscribe from an event type
 *
 * @param type Event type
 * @param queue Queue to remove
 */
void event_bus_unsubscribe(event_type_t type, QueueHandle_t queue);

/**
 * @brief Publish an event (async - enqueues and returns immediately)
 *
 * Event is placed in internal queue and processed by dispatcher task.
 * Subscribers receive event copy in their target queue.
 *
 * @param event Event to publish
 */
void event_bus_publish(const event_t *event);

#endif /* EVENT_BUS_H */
