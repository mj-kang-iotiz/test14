# Host Test Infrastructure

STM32 프로젝트의 호스트(PC) 기반 단위/모듈 테스트 환경.
HAL/RTOS에 의존하지 않는 순수 로직을 호스트 gcc로 빌드하여 테스트.

## 실행 방법

```bash
bash test/run_tests.sh          # 빌드 + 전체 테스트 실행
bash test/run_tests.sh clean    # 빌드 디렉토리 삭제
bash test/run_tests.sh verbose  # 상세 출력
```

## 디렉토리 구조

```
test/
├── CMakeLists.txt         # 빌드 설정
├── run_tests.sh           # 빌드+실행 스크립트
├── README.md              # 이 문서
│
├── unity/                 # Unity 테스트 프레임워크 (v2.6.0)
│   ├── unity.c
│   ├── unity.h
│   └── unity_internals.h
│
├── mock/                  # Mock/Stub (호스트 빌드용)
│   ├── FreeRTOS.h         # FreeRTOS 타입 stub
│   ├── task.h             # xTaskGetTickCount 등 stub
│   ├── semphr.h           # Semaphore stub
│   ├── queue.h            # Queue stub
│   ├── stm32f4xx_hal.h    # HAL_GetTick stub (log.h용)
│   ├── cmsis_compiler.h   # __disable_irq, __NOP stub
│   ├── mock_common.c      # mock_tick_count, dev_assert_failed (abort 버전)
│   ├── gps_stubs.c        # unicore/rtcm 파서 stub
│   └── gps_stubs_nmea.c   # nmea 파서 stub (test_ringbuffer용)
│
├── fixture/               # 테스트 데이터 (static const 배열)
│   └── nmea/
│       └── nmea_fixture.h # GGA, THS, GSV 등 NMEA sentence
│
├── unit/                  # 단위 테스트 (PURE 모듈)
│   ├── test_parser.c      # lib/parser/parser.c
│   └── test_ringbuffer.c  # lib/utils/src/ringbuffer.c
│
└── module/                # 모듈 테스트 (MOCKABLE 모듈, mock 사용)
    └── test_gps_nmea.c    # lib/gps/gps_nmea.c
```

## 테스트 분류

| 분류 | 위치 | 대상 | Mock 필요 |
|------|------|------|-----------|
| **unit** | `test/unit/` | PURE 모듈 (parser, ringbuffer) | 없음 |
| **module** | `test/module/` | MOCKABLE 모듈 (gps_nmea 등) | FreeRTOS/HAL stub |

## 파일 매핑 규칙

소스 파일과 테스트 파일은 **1:1 매핑**:

```
lib/parser/parser.c          → test/unit/test_parser.c
lib/utils/src/ringbuffer.c   → test/unit/test_ringbuffer.c
lib/gps/gps_nmea.c           → test/module/test_gps_nmea.c
lib/gps/gps_unicore.c        → test/module/test_gps_unicore.c    (미구현)
lib/gps/rtcm.c               → test/module/test_gps_rtcm.c       (미구현)
lib/ble/ble_parser.c          → test/module/test_ble_parser.c     (미구현)
```

## Fixture 데이터 관리

- `test/fixture/{protocol}/` 하위에 `.h` 파일로 관리
- `static const char[]` 또는 `static const uint8_t[]`로 정의
- 테스트 코드에서 `#include "nmea/nmea_fixture.h"` 로 사용
- 네이밍: `{MSG_TYPE}_{CASE}` (예: `GGA_RTK_FIX`, `RTCM_1074_VALID`)

## 새 테스트 추가 방법

### 1. 기존 모듈에 테스트 케이스 추가

해당 `test_*.c` 파일에 함수 추가 후 `main()`의 `RUN_TEST()`에 등록:

```c
void test_new_case(void) {
    TEST_ASSERT_EQUAL(expected, actual);
}

// main() 안에:
RUN_TEST(test_new_case);
```

### 2. 새 모듈 테스트 추가

1. `test/{unit|module}/test_{module}.c` 파일 생성
2. 필요시 `test/fixture/` 에 테스트 데이터 추가
3. `test/CMakeLists.txt` 에 `add_executable` + `add_test` 추가
4. `bash test/run_tests.sh` 로 빌드 확인

## 알려진 제한

- `gps_nmea.c` 에 `GPS_NMEA_MSG_RMC` dead code가 있어 CMake에서 임시 define 추가
  - `tasks/gps_nmea_dead_code_cleanup.md` 완료 후 제거 가능
- `log.h` 가 `stm32f4xx_hal.h` 참조 (F4 → H5 마이그레이션 미완료)
  - mock 헤더로 우회 중
