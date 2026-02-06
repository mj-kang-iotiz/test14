# GUGU MACHINE GUIDANCE V2 FW

## Project Overview
STM32H562VGT6 기반 RTK(Real-Time Kinematic) Rover 펌웨어 프로젝트.
Base Station과 Rover 장치 모두 지원하며, 고정밀 GNSS 위치 측정을 위한 시스템.

> **Migration Status**: STM32F405 → STM32H562VGT6 마이그레이션 진행 중

## Tech Stack
- **MCU**: STM32H562VGT6 (Cortex-M33, 250MHz, TrustZone)
- **Previous MCU**: STM32F405 (Cortex-M4, 168MHz) - 마이그레이션 원본
- **IDE**: STM32CubeIDE
- **HAL**: STM32H5xx HAL Driver only (LL 미사용)
- **RTOS**: FreeRTOS (LTS) v11 (직접 포팅, CMSIS wrapper 미사용)
- **Debug**: SEGGER SystemView
- **Language**: C

## Directory Structure
```
├── Core/              # STM32CubeMX 생성 코드
│   ├── Inc/           # 헤더 (main.h, stm32h5xx_hal_conf.h, stm32h5xx_it.h)
│   ├── Src/           # 소스 (main.c, stm32h5xx_hal_msp.c, stm32h5xx_it.c)
│   ├── Startup/       # 스타트업 어셈블리
│   └── ThreadSafe/    # Thread-safe 구현
├── Drivers/           # STM32 드라이버
│   ├── CMSIS/         # ARM CMSIS
│   └── STM32H5xx_HAL_Driver/  # HAL 드라이버 (LL 미사용)
├── config/            # 보드 설정, FreeRTOS 설정
├── lib/               # 하드웨어 추상화 라이브러리 (HAL 래퍼)
│   ├── ble/           # BLE 통신 드라이버
│   ├── gps/           # GPS 모듈 드라이버 (UM982)
│   ├── gsm/           # GSM 모듈 드라이버
│   ├── led/           # LED 제어
│   ├── log/           # 로깅 유틸리티
│   ├── lora/          # LoRa 통신 드라이버
│   ├── parser/        # 데이터 파싱 유틸리티
│   ├── rs485/         # RS485 통신 드라이버
│   └── utils/         # 공통 유틸리티
├── app/               # 애플리케이션 레벨 모듈
│   ├── core/          # 앱 메인 로직
│   ├── ble/           # BLE 앱 로직
│   ├── gps/           # GPS 앱 로직 (RTCM 처리 포함)
│   ├── gsm/           # GSM 앱 로직
│   ├── lora/          # LoRa 앱 로직
│   ├── params/        # Flash 파라미터 저장/로드
│   └── rs485/         # RS485 앱 로직
├── third_party/       # 외부 라이브러리
│   ├── FreeRTOS-LTS/  # FreeRTOS (직접 포팅)
│   └── SystemView/    # SEGGER SystemView
├── docs/              # 문서
└── *.ioc              # STM32CubeMX 프로젝트 파일
```

## Build
```bash
# STM32CubeIDE에서 빌드
# Project → Build Project (Ctrl+B)

# 빌드 설정
# - Debug/Release configuration 선택
# - Board type 매크로 설정: C/C++ Build → Settings → Preprocessor
```

## Code Conventions
- 파일명: `snake_case.c`, `snake_case.h`
- 함수명: `module_function_name()` (예: `ble_app_start()`, `gps_get_handle()`)
- 타입명: `module_type_t` (예: `ble_instance_t`, `gps_cmd_request_t`)
- 매크로: `UPPER_SNAKE_CASE` (예: `BLE_RX_BUF_SIZE`)
- 구조체 멤버: `snake_case`
- 헤더 가드: `MODULE_H`
- 주석: Doxygen 스타일 (`@brief`, `@param`, `@return`)
- 섹션 구분: `/*==== Section ====*/`
- 탭: 4칸

## Module Dependencies
```
app/ (앱 레벨) → lib/ (HAL 래퍼) → Drivers/STM32H5xx_HAL_Driver/ (HAL)
                                 → Drivers/CMSIS/ (ARM CMSIS)
```

## Key Concepts
- **RTCM**: RTK 보정 데이터 프로토콜
- **NMEA**: GPS 표준 메시지 포맷
- **HAL**: Hardware Abstraction Layer (LL 드라이버 미사용 정책)

## STM32F405 → H562 Migration Notes
- **DMA → GPDMA**: 완전히 다른 아키텍처, Stream 기반 → Channel 기반
- **GPIO AF**: Alternate Function 번호 다름, CubeMX로 확인
- **클럭**: PLL 구조 다름, CubeMX로 재설정
- **IRQ 이름**: `DMA1_Stream0_IRQn` → `GPDMA1_Channel0_IRQn`
- **LL 드라이버 사용 금지**: HAL만 사용

## Development Notes
- CubeMX `.ioc` 파일로 peripheral 설정 관리
- HAL 함수 사용 시 반드시 반환값 확인
- FreeRTOS task에서 HAL 함수 호출 시 mutex 고려

## Testing

### 호스트 테스트 환경
- **프레임워크**: Unity v2.6.0 (`test/unity/`)
- **빌드**: CMake + 호스트 gcc (arm-gcc 아님)
- **실행**: `bash test/run_tests.sh`
- **상세**: `test/README.md` 참조

### 테스트 분류
- `test/unit/` — 단위 테스트 (PURE 모듈: parser, ringbuffer)
- `test/module/` — 모듈 테스트 (MOCKABLE 모듈: gps_nmea, gps_unicore, rtcm, ble_parser)
- `test/fixture/` — 테스트 데이터 (.h 파일, static const 배열)
- `test/mock/` — FreeRTOS/HAL stub 헤더

### 테스트 데이터 관리
- fixture는 프로토콜별 디렉토리: `test/fixture/{nmea,rtcm,unicore}/`
- 데이터는 `.h` 파일의 `static const` 배열로 정의
- 네이밍: `{MSG_TYPE}_{CASE}` (예: `GGA_RTK_FIX`, `RTCM_1074_VALID`)

### 파일 매핑 (소스 → 테스트)
```
lib/parser/parser.c          → test/unit/test_parser.c
lib/utils/src/ringbuffer.c   → test/unit/test_ringbuffer.c
lib/gps/gps_nmea.c           → test/module/test_gps_nmea.c
lib/gps/gps_unicore.c        → test/module/test_gps_unicore.c
lib/gps/rtcm.c               → test/module/test_gps_rtcm.c
```

### 태스크 수행 시 테스트 규칙
1. `lib/` 코드 변경 시 반드시 관련 테스트 확인 (없으면 작성)
2. 새 함수 추가 시 최소 3개 테스트: 정상, 경계, 에러
3. 기존 파서 수정 시 regression 테스트 필수
4. **`bash test/run_tests.sh` 실행 후 ALL PASS 확인 필수**
5. 실패하는 테스트를 커밋하지 말 것

### 테스트 작성 컨벤션
- 파일명: `test_{모듈명}.c`
- 함수명: `test_{동작}_{조건}` (예: `test_gga_parse_no_fix`)
- `setUp()`/`tearDown()`에서 ringbuffer, 구조체 초기화/정리
- fixture `#include`로 테스트 데이터 사용
- 새 테스트 추가 시 `CMakeLists.txt`에 `add_executable` + `add_test` 등록

### 태스크 md 테스트 섹션 템플릿
태스크 파일에 다음 섹션 추가:
```markdown
## 테스트
### 자동 테스트 (호스트)
- [ ] 테스트 파일: `test/{unit|module}/test_xxx.c`
- [ ] fixture 추가: `test/fixture/{protocol}/xxx.h` (필요 시)
- [ ] 테스트 케이스:
  - [ ] 정상: {설명}
  - [ ] 경계: {설명}
  - [ ] 에러: {설명}
  - [ ] regression: 기존 동작 확인
- [ ] `bash test/run_tests.sh` → ALL PASS
```

## Documentation Structure
- `docs/hw_spec.md` - 하드웨어 스펙
- `docs/use_case.md` - 사용 시나리오
- `docs/app/` - 앱 모듈별 문서
- `docs/driver/` - 드라이버별 문서
- `docs/util/` - 유틸리티 문서
- `docs/task/` - 작업별 문서
- `docs/task/archive/` - 완료된 작업
- `docs/todo/` - 할 일 목록

## Task Workflow
작업 요청은 `tasks/` 폴더의 md 파일로 관리.
- `tasks/*.md` - 진행할 작업
- `tasks/archive/` - 완료된 작업

작업 파일 형식:
```markdown
# [작업 제목]

## 요약
한 줄 요약

## 배경
왜 이 작업이 필요한지

## 요구사항
- [ ] 체크리스트 형태로 작성
- [ ] 구체적인 구현 항목

## 관련 파일
- `path/to/file.c`

## 완료 조건
어떤 상태가 되면 완료인지

## 참고
관련 문서, 링크 등
```

## Related Documents
- `docs/hw_spec.md`: 하드웨어 스펙
- `docs/use_case.md`: 사용 시나리오
- `docs/app/`: 앱 모듈별 문서
- `docs/driver/`: 드라이버별 문서
- `docs/util/`: 유틸리티 문서
- `docs/task/`: 작업별 문서
- `docs/task/archive/`: 완료된 작업
- `docs/todo/`: 할 일 목록