# GPS 파서 성능 최적화

## 요약
`gps_parser_process` 및 관련 유틸리티의 성능 개선

## 배경
Chain 방식 파서(NMEA → Unicore ASCII → Unicore Binary → RTCM)의 기본 구조는 양호.
첫 바이트 기반 O(1) rejection, 상태 관리 정상 확인됨.
세부적인 성능 개선 포인트 정리.

## 현재 문제점

### 1. 미등록 NMEA 바이트 단위 skip (최우선)
> `tasks/gps_nmea_dead_code_cleanup.md`에서 함께 처리

### 2. `ringbuffer_find_char` 내부 구현 비효율
현재 `gps_parser.c:24-39`에서 `ringbuffer_peek`를 한 글자씩 N번 호출:
```c
for (size_t i = 0; i < search_len; i++) {
    ringbuffer_peek(rb, &c, 1, i);  // 매번 offset 계산 + bounds check
}
```
**개선**: ringbuffer 모듈 내부에서 연속 메모리 구간에 `memchr` 적용.
외부 API(`ringbuffer_find_char`)는 변경 없음, 내부 구현만 최적화.
```c
/* ringbuffer 모듈 내부에서 최적화 */
bool ringbuffer_find_char(ringbuffer_t *rb, char ch, size_t max, size_t *pos) {
    /* 구간1: head ~ capacity (또는 tail) */
    size_t linear_len = /* head부터 wrap 전까지 연속 길이 */;
    char *found = memchr(&rb->buf[rb->head], ch, linear_len);
    if (found) { *pos = found - &rb->buf[rb->head]; return true; }
    /* 구간2: 0 ~ tail (wrap-around) */
    ...
}
```
**우선순위**: 낮음 (기능 정상, 성능 최적화)

### 3. `gps_process_task` 디버그 로깅 비효율
`gps.c:251-257`:
```c
static char buf[2048];
size_t len = ringbuffer_size(&gps->rx_buf);
if (len > 0) {
    len = (len > sizeof(buf)) ? sizeof(buf) : len;
    ringbuffer_peek(&gps->rx_buf, buf, len, 0);
    LOG_DEBUG_RAW("", buf, len);
}
```
- 매 수신마다 전체 ringbuffer를 2048바이트 버퍼로 복사
- 프로덕션에서는 불필요한 오버헤드
- `static` 변수이므로 multi-instance 사용 시 문제 (현재는 단일이므로 즉시 이슈는 아님)

**개선**: `#if defined(DEBUG)` 등으로 감싸거나 제거

## 요구사항
- [ ] `ringbuffer_find_char`를 ringbuffer 모듈로 이동하고 `memchr` 기반으로 최적화 (우선순위 낮음)
- [ ] `gps_process_task`의 디버그 로깅 블록을 `#if defined(DEBUG)` 등으로 보호 또는 제거
- [ ] `static char buf[2048]` 제거 또는 조건부 컴파일로 보호

## 관련 파일
- `lib/gps/gps_parser.c:24-39` — `ringbuffer_find_char` 구현
- `lib/gps/gps.c:251-257` — 디버그 로깅 블록
- `lib/parser/ringbuffer.c` (또는 해당 모듈) — `ringbuffer_find_char` 이동 대상

## 완료 조건
- 프로덕션 빌드에서 불필요한 2048바이트 복사 제거됨
- `ringbuffer_find_char`가 ringbuffer 모듈 내부에서 효율적으로 동작
