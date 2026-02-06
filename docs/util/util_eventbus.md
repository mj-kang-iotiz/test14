# Event Bus

모듈 간 비동기 Pub/Sub. 큐 기반만 지원.

## API
```c
void event_bus_init(void);
bool event_bus_subscribe(event_type_t type, QueueHandle_t queue);
void event_bus_unsubscribe(event_type_t type, QueueHandle_t queue);
void event_bus_publish(const event_t *event);
```

## 설정
```c
// event_bus.h
#define EVENT_BUS_MAX_SUBSCRIBERS   16  // 최대 구독자 수 (연결 리스트, 개수 제한)
```

## 사용 패턴
```c
// 구독
QueueHandle_t q = xQueueCreate(8, sizeof(event_t));
event_bus_subscribe(EVENT_GPS_FIX_CHANGED, q);

// 태스크에서 수신
event_t ev;
if (xQueueReceive(q, &ev, portMAX_DELAY) == pdTRUE) {
    switch (ev.type) { ... }
}

// 발행
event_t ev = { .type = EVENT_GPS_FIX_CHANGED, .data.gps_fix = {...} };
event_bus_publish(&ev);

// 해제
event_bus_unsubscribe(EVENT_GPS_FIX_CHANGED, q);
```

## 이벤트 타입 (event_bus.h)
| Range | Category |
|-------|----------|
| 0x0100 | GPS (FIX_CHANGED, GGA_UPDATE) |
| 0x0200 | NTRIP (CONNECTED, DISCONNECTED) |
| 0x0300 | LoRa (RTCM_FOR_LORA) |
| 0x0F00 | System (SHUTDOWN) |

## 주의
- 큐 full → 이벤트 드랍 (로그 경고)
- 큰 데이터는 이벤트로 알림만, 실제 데이터는 링버퍼에서 읽음
