# GPS TODO

> 작업 시작 시 `tasks` 폴더로 이동하여 진행
> 작업 완료 시 'tasks/archive' 폴더로 이동

## 확인 필요
- [x] GGA raw 패킷 저장하는 로직 개선방안 확인 (다수의 외부 통신으로(ble, ntrip 등) 전송 필요)
- [x] NMEA는 GGA, THS만 파싱
- [x] gps_parser.c에서 gps_parser_process 파싱 로직 성능이 괜찮은지, 정상인지 확인

### 검토 결과 (2026-02-06)

#### 1. GGA raw 패킷 저장 로직 — 개선 필요

**현재 구현** (`gps_nmea.c:207-217`, `gps_nmea.h:70-74`):
- `USE_STORE_RAW_GGA` 조건부 컴파일로 `gga_raw[120]` 고정 버퍼에 memcpy
- `gga_is_rdy` 단일 플래그로 소비 여부 관리

**문제점**:
1. **단일 소비자만 지원**: `gga_is_rdy` 플래그가 하나이므로 BLE가 읽으면 NTRIP은 읽을 수 없음. 다수의 소비자(BLE, NTRIP, RS485 등) 동시 지원 불가
2. **Thread-safety 부재**: GPS Task에서 쓰고 다른 Task(BLE, GSM 등)에서 읽으면 race condition 발생. mutex 보호 없음
3. **버퍼 크기 하드코딩**: `gga_raw[120]`이 직접 하드코딩. `gps_config.h`에서 관리해야 함
4. **레거시 함수 잔재**: `gps.c:293`의 `_gps_gga_raw_add()`는 바이트 단위 추가 방식의 deprecated 함수인데 아직 남아있음

**개선 방안**:
- 이벤트 핸들러 활용: GGA 파싱 시 `GPS_EVENT_FIX_UPDATED` 이벤트에 raw GGA 포인터/길이를 함께 전달하고, 각 소비자(BLE, NTRIP)가 이벤트 핸들러 내에서 자신의 버퍼로 복사하는 방식. GPS Task 컨텍스트에서 호출되므로 thread-safe
- 또는 소비자별 ready 플래그 배열 혹은 소비자별 링버퍼를 두는 Pub/Sub 패턴 적용
- `_gps_gga_raw_add()` 레거시 함수 제거

#### 2. NMEA는 GGA, THS만 파싱 — 맞음, 단 dead code 존재

**확인 결과** (`gps_proto_def.h:24-27`):
```c
#define NMEA_MSG_TABLE(X) \
    X(GGA, "GGA", nmea_parse_gga, 14, true) \
    X(THS, "THS", nmea_parse_ths,  2, true)
```
테이블에 GGA, THS만 등록되어 있어 이 두 메시지만 파싱하는 것이 맞음.

**문제점**:
1. **RMC dead code**: `gps_nmea.c:25`에 `nmea_parse_rmc` 함수가 선언/정의되어 있지만, NMEA_MSG_TABLE에 등록되지 않아 절대 호출되지 않음. stub 함수이므로 삭제하거나 테이블에 등록 필요
2. **RMC 이벤트 핸들링 dead code**: `gps_nmea.c:199-204`에 `GPS_NMEA_MSG_RMC` 분기가 있지만, RMC가 테이블에 없으므로 이 코드는 도달 불가능한 dead code
3. **미등록 NMEA 비효율**: `$GPGSV`, `$GPGSA` 같은 테이블 미등록 NMEA 메시지가 수신되면, `nmea_try_parse`가 `PARSE_NOT_MINE`을 반환 → 다른 파서도 모두 거부 → 1바이트씩 skip하며 전체 sentence를 소모. 전체 sentence를 한 번에 skip하는 것이 효율적

#### 3. gps_parser_process 파싱 로직 — 기본 구조 양호, 세부 개선 필요

**기본 구조 평가** (`gps_parser.c:67-123`):
- Chain 방식(NMEA → Unicore ASCII → Unicore Binary → RTCM)은 설계상 적절
- 각 파서가 첫 바이트로 빠르게 거부(`$`, `0xAA`, `0xD3`)하므로 O(1) rejection — 성능 양호
- `PARSE_OK`/`NEED_MORE`/`INVALID`/`NOT_MINE` 상태 관리는 정상적

**성능 이슈**:
1. **미등록 NMEA 바이트 단위 skip** (위 2번과 동일): `$GPGSV,...*XX\r\n` 같은 유효하지만 미등록인 NMEA sentence가 들어오면, `nmea_try_parse`가 `PARSE_NOT_MINE` 반환 → `gps_parser_process`에서 1바이트 skip → 다음 루프에서 `G`부터 다시 4개 파서 모두 시도. 약 80바이트 sentence × 4파서 = ~320회 불필요한 파서 호출. **해결**: `nmea_try_parse`에서 유효한 NMEA 구조(`$xxYYY,...*XX\r\n`)이지만 미등록 메시지인 경우 전체 sentence를 advance하고 `PARSE_OK` 또는 별도 결과를 반환
2. **`ringbuffer_find_char` 오버헤드**: `ringbuffer_peek`를 한 문자씩 호출하여 검색 (`gps_parser.c:28-31`). ringbuffer가 연속 메모리 영역에 있는 경우 `memchr`로 대체하면 더 빠름
3. **`gps_process_task` 디버그 로깅 비효율** (`gps.c:251-257`): `static char buf[2048]`로 매번 전체 ringbuffer를 peek/복사한 뒤 `LOG_DEBUG_RAW`를 호출. 프로덕션에서는 불필요한 오버헤드. `#if defined(DEBUG)` 등으로 감싸거나 제거 필요
4. **`static char buf[2048]`**: task 내 static 변수이므로 multi-instance 사용 시 문제. 다만 현재는 단일 task이므로 당장은 문제없음
- [ ] gps_hal_ops_t 는 low level 한 ops인데, 인터페이스 이렇게 구성하면 되는지 확인
- [ ] gps_hal_ops_t 가지고 hw 초기화 하는 부분이 결합도가 높으므로 어떻게 구성하는게 좋은지 확인
- [ ] 각종 try_parse 로직 및 성능이 좋은지 확인
- [ ] rtcm.c에서 lora 등등 각종 모듈과 너무 깊게 결합되어 있는데, 어떻게 구상하면 좋을지 확인 (현재는 GPS 부터 수정하고 싶고 나중에 LoRa 에 대한 처리를 하고싶음.)
- [ ] rtcm 패킷을 링버퍼에 저장하고 정상적으로 수신받아서 처리 가능한지 여부
- [ ] gps_init 초기화 로직이 타당한지 확인 필요. 명령어 전송해서 GPS 설정하는 부분은 어디서 구현해야할지 검토 필요(동기식이라 현재는 app단에서 구현되어 있을것으로 예상됨)
- [ ] gps_deinit 에서 리소스 해제가 문제없이 되는지 확인
- [ ] `gps_hal_ops_t`의 `recv` 함수 미사용 문제 확인 (현재 ISR에서 `ringbuffer_write` 직접 호출로 HAL 계층 위반 의심)

## lib/gps (드라이버)
- [ ] RTCM CRC 검증 추가
- [ ] `gps.h`, `rtcm_data` 내부 버퍼 사이즈(2048, 4096) 하드코딩 제거 → `gps_config.h` 매크로 사용
- [ ] 설정값 통합 관리: 타임아웃, 재시도 횟수, 버퍼 크기 등이 `gps_app.c`, `gps.h` 등에 산재됨 -> `gps_config.h`로 통합 필요
- [ ] `gps_unicore.c` 등의 deprecated 함수 제거 및 코드 정리
- [ ] 데이터 복사 → 링버퍼 교체 (해당 부분 확인 필요)
- [ ] GPS 이벤트에서 NMEA, unicore protocol 데이터중 어느것으로 받을지 설정이 필요 (어떤 메시지를 사용할것이고, 이걸 어떻게 쉽게 이벤트 핸들러 호출 할것인지 등등 결합도와 확장성도 생각해야함) 복잡하지만 gps_common_data_t 하고 gps_event_t 에 어떤 데이터를 보낼지도 필요. 그리고 둘의 기능이 중복된 느낌이라 확장성 높게 개선 필요
- [ ] $command 로 시작하는 명령어 전송 후, ack(아마 response:OK 수신) 까지 받아서 처리하는 로직 추가 (현재는 복잡하게 구성되어 있을수도 있음) 이미 구성되어있다면 이 로직이 정상적인지, 성능상 좋은지 확인하고 수정
- [ ] $command 로 시작하는 명령어 전송 함수 추가
- [ ] 원하는 rtcm 메시지 타입만 링버퍼에 저장하는 기능 추가
- [ ] 이벤트 핸들러 수정 필요. 단순 하나의 콜백함수 등록보다는 확장성 높게 수정 필요.
- [ ] 에러 처리 필요
- [ ] gps_process_task 에서 static char buf[2048] 이것보다는 우아하게 처리 필요
- [ ] 어떠한 문제로 인해 태스크가 종료되는 경우 이벤트 핸들러로 메시지 전송 필요

## app/gps (앱)
- [ ] gps_hal_ops_t 로 hw 초기화 하는 로직의 결합도 감소 필요 (cubeide에서 자동 생성된 코드 이용 예정. init 코드에 cubeide에서 생성한 코드 사용) 즉 gps_port.c에 정의한 것들이 결합도가 너무 높음
- [ ] STM32H562에 맞게끔 port 수정
- [ ] 동기식으로 GPS 초기화 하는 코드 추가 (실패시 에러 처리 필요)
- [ ] base_auto_fix.c 는 base 모드에서 ble나 ntrip으로 rtcm 데이터를 보정받아 rtk fix가 되면 알고리즘을 사용해서 위치값을 최대한 정확하게 설정해서 fixed base station으로 동작해야하는데, 이에 대한 알고리즘 및 동작 수정 필요. 수정 및 설정이 편해야하고(확장성) 네이밍도 이렇게 작성하면 되는지 확인 및 수정 필요
- [ ] um982_base_cmds 나 um982_rover_cmds 와 같은 명령어는 정상적으로 전송됐는지 확인하고, 에러 발생했을때 처리하는 로직 필요. 꼭 테이블 방식이 아니더라도 확장성과 고급지게 만들었으면 함
- [ ] 일종의 싱글톤 방식으로 구현하고 싶지 않음. 다수의 인스턴스로 사용해도 동작하게끔 수정 필요 (현재 `gps_port.c`의 `g_gps_instance` 전역변수 의존성 제거)

## MCU 마이그레이션 (F405 → H562)
- [ ] `gps_port.c` 에 hw 관련 로직 추가
- [ ] stm32f4 로 되어있는 hal 라이브러리를 stm32h5에 맞게 수정

---

## 추후 구현 사항 (지금은 적용 x)
- [ ] LED 동작 위치 변경 (이벤트 핸들러 or 이벤트 버스)
- [ ] uart baudrate 자동 감지 및 변경 (현재는 115200으로 고정)

## 작업 진행 방법

1. 위 항목 중 작업할 것 선택
2. `tasks/gps_xxx.md` 파일 생성 (구체적 요구사항)
3. 작업 완료 후 → 위 체크박스 체크
4. task 파일 → `tasks/archive/`로 이동

### 예시: tasks/gps_rtcm_crc.md
```markdown
# RTCM CRC 검증 추가

## 요약
rtcm.c에 CRC24Q 검증 로직 추가

## 요구사항
- [ ] CRC24Q 계산 함수 구현
- [ ] rtcm_try_parse()에서 CRC 검증 후 PARSE_INVALID 반환

## 관련 파일
- lib/gps/rtcm.c, rtcm.h

## 완료 조건
- 잘못된 RTCM 패킷 수신 시 stats.crc_errors 증가
```
