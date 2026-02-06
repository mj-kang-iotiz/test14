---
type: cleanup
modules: [gps]
---
# NMEA Dead Code 정리 및 미등록 메시지 처리 개선

## 요약
RMC 관련 dead code 제거, 미등록 NMEA 메시지의 바이트 단위 skip 문제 해결

## 배경
- `NMEA_MSG_TABLE`에 GGA, THS만 등록된 것 확인 완료
- RMC 관련 코드가 테이블에 미등록 상태로 dead code로 남아있음
- 미등록 NMEA 메시지(`$GPGSV` 등) 수신 시 비효율적으로 처리됨

## 현재 문제점

### 1. RMC Dead Code
- `gps_nmea.c:25` — `nmea_parse_rmc` 선언
- `gps_nmea.c:382-387` — `nmea_parse_rmc` stub 구현 (모든 파라미터 void cast)
- `gps_nmea.c:199-204` — `GPS_NMEA_MSG_RMC` 이벤트 핸들링 분기
- NMEA_MSG_TABLE에 RMC 미등록 → 위 코드 전부 도달 불가능

### 2. 미등록 NMEA 비효율 (상세)
`$GPGSV,3,1,12,...*7F\r\n` (80바이트) 수신 시 현재 동작:

```
반복1: nmea_try_parse → '$'확인 → 'GP'확인 → 'GSV' 테이블에 없음
       → PARSE_NOT_MINE 반환 → 1바이트 '$' skip
반복2: 'G' → 4파서 모두 NOT_MINE → skip
반복3: 'P' → 4파서 모두 NOT_MINE → skip
... 80번 반복 (4파서 × 80바이트 = 320회 파서 호출)
```

**해결**: `nmea_try_parse`에서 talker ID 확인 후 미등록 메시지면 전체 sentence skip

## 요구사항
- [ ] `nmea_parse_rmc` 선언 및 stub 구현 제거 (`gps_nmea.c:25`, `gps_nmea.c:382-387`)
- [ ] `GPS_NMEA_MSG_RMC` 이벤트 핸들링 분기 제거 (`gps_nmea.c:199-204`)
- [ ] `nmea_try_parse`에서 미등록 NMEA 전체 sentence skip 처리 추가:
  ```c
  if (msg_id == GPS_NMEA_MSG_NONE) {
      /* 유효한 NMEA이지만 미등록 → 전체 sentence skip */
      size_t cr_pos;
      if (ringbuffer_find_char(rb, '\r', GPS_NMEA_MAX_LEN, &cr_pos)) {
          size_t skip_len = cr_pos + 1;
          char next;
          if (ringbuffer_peek(rb, &next, 1, cr_pos + 1) && next == '\n') {
              skip_len++;
          }
          ringbuffer_advance(rb, skip_len);
          return PARSE_OK;
      }
      /* \r 없으면 데이터 부족 */
      if (ringbuffer_size(rb) >= GPS_NMEA_MAX_LEN) {
          return PARSE_INVALID;
      }
      return PARSE_NEED_MORE;
  }
  ```

## 관련 파일
- `lib/gps/gps_nmea.c` — RMC dead code 제거, 미등록 NMEA skip 처리
- `lib/gps/gps_proto_def.h` — NMEA_MSG_TABLE 확인 (변경 없음)

## 완료 조건
- `nmea_parse_rmc` 관련 코드 전부 제거됨
- `$GPGSV` 같은 미등록 NMEA 수신 시 1회에 전체 sentence skip
- 등록된 GGA, THS 파싱은 기존과 동일하게 동작
