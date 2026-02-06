---
type: refactor
modules: [gps]
---
# GGA Raw 패킷 전달 구조 리팩토링

## 요약
GGA raw 데이터를 다수 소비자(BLE, NTRIP 등)에게 필요할 때만 전달하는 콜백 등록 방식으로 변경

## 배경
현재 `USE_STORE_RAW_GGA` 조건부 컴파일로 `gga_raw[120]` 고정 버퍼에 memcpy 후 `gga_is_rdy` 단일 플래그로 관리.
BLE가 읽으면 NTRIP은 읽을 수 없고, mutex 보호도 없어 race condition 위험.

## 현재 문제점
1. **단일 소비자**: `gga_is_rdy` 플래그 1개 → 다수 소비자 동시 지원 불가
2. **Thread-safety 부재**: GPS Task에서 쓰고 다른 Task에서 읽을 때 보호 없음
3. **버퍼 크기 하드코딩**: `gga_raw[120]` 직접 하드코딩
4. **레거시 잔재**: `_gps_gga_raw_add()` deprecated 함수 잔존

## 설계 방안: 콜백 등록 방식 (Observer)

### 핵심 구조
```c
/* gps.h */
#define GPS_GGA_RAW_MAX_SUBSCRIBERS  4

typedef void (*gps_gga_raw_cb_t)(const char *raw, size_t len, void *user_data);

/* gps_t 구조체에 추가 */
struct {
    gps_gga_raw_cb_t cb[GPS_GGA_RAW_MAX_SUBSCRIBERS];
    void *user_data[GPS_GGA_RAW_MAX_SUBSCRIBERS];
    uint8_t count;
} gga_raw_subscribers;
```

### 등록/해제 API
```c
bool gps_register_gga_raw(gps_t *gps, gps_gga_raw_cb_t cb, void *user_data);
bool gps_unregister_gga_raw(gps_t *gps, gps_gga_raw_cb_t cb);
```

### 호출 위치 (gps_nmea.c - nmea_try_parse 내부, GGA 파싱 완료 후)
```c
/* 기존 USE_STORE_RAW_GGA 블록 대체 */
if (msg_id == GPS_NMEA_MSG_GGA && gps->gga_raw_subscribers.count > 0) {
    /* \r\n 포함하여 raw 전달 */
    char raw_buf[GPS_NMEA_MAX_LEN + 2];
    memcpy(raw_buf, buf, cr_pos);
    raw_buf[cr_pos] = '\r';
    raw_buf[cr_pos + 1] = '\n';

    for (uint8_t i = 0; i < gps->gga_raw_subscribers.count; i++) {
        gps->gga_raw_subscribers.cb[i](raw_buf, cr_pos + 2,
            gps->gga_raw_subscribers.user_data[i]);
    }
}
```

### 소비자 사용 예시
```c
/* BLE 앱 초기화 시 */
static void ble_on_gga_raw(const char *raw, size_t len, void *ctx) {
    ble_ctx_t *ble = (ble_ctx_t *)ctx;
    /* 자기 버퍼에 복사만 (GPS Task 컨텍스트이므로 빨리 끝내야 함) */
    memcpy(ble->gga_buf, raw, len);
    ble->gga_len = len;
    ble->gga_ready = true;
}

gps_register_gga_raw(gps, ble_on_gga_raw, &ble_ctx);

/* NTRIP 앱 초기화 시 */
gps_register_gga_raw(gps, ntrip_on_gga_raw, &ntrip_ctx);
```

### 장점
- 구독자 없으면 memcpy 자체를 안 함 (zero cost when unused)
- GPS Task 컨텍스트에서 콜백 호출 → 별도 mutex 불필요 (thread-safe)
- 각 소비자가 자기 버퍼에 독립 복사 → 서로 간섭 없음
- `gga_raw[120]`, `gga_is_rdy`, `USE_STORE_RAW_GGA`, `_gps_gga_raw_add()` 모두 제거 가능

### 주의사항
- 콜백 안에서 오래 걸리는 작업(UART 전송 등) 금지. 자기 버퍼에 복사만 수행
- `GPS_GGA_RAW_MAX_SUBSCRIBERS` 초과 등록 시 에러 반환

## 요구사항
- [ ] `gps_t`에 `gga_raw_subscribers` 구조체 추가
- [ ] `gps_register_gga_raw()`, `gps_unregister_gga_raw()` API 구현
- [ ] `gps_nmea.c`에서 GGA 파싱 완료 후 등록된 콜백 호출 로직 추가
- [ ] `USE_STORE_RAW_GGA` 관련 코드 제거 (`gps_nmea.h`, `gps_nmea.c`, `gps.c`)
- [ ] `_gps_gga_raw_add()` 레거시 함수 제거
- [ ] `gps_init()`에서 `gga_raw_subscribers.count = 0` 초기화 확인 (memset으로 이미 처리됨)

## 관련 파일
- `lib/gps/gps.h` — `gps_t` 구조체, API 선언
- `lib/gps/gps.c` — `gps_register/unregister` 구현, `_gps_gga_raw_add()` 제거
- `lib/gps/gps_nmea.c:207-217` — `USE_STORE_RAW_GGA` 블록 → 콜백 호출로 교체
- `lib/gps/gps_nmea.h:70-74` — `gga_raw`, `gga_is_rdy` 필드 제거

## 완료 조건
- 구독자 없을 때 GGA raw 복사 안 함
- 다수 소비자(2개 이상)가 동시에 독립적으로 GGA raw 수신 가능
- `USE_STORE_RAW_GGA` 조건부 컴파일 완전 제거
