# GPS 앱 (app/gps)

## 설계 의도
- 드라이버(`lib/gps`)와 비즈니스 로직 분리
- 이벤트 수신 → LED 업데이트 + event_bus 전파
- Base/Rover 역할에 따라 초기화 명령어 다름

## 파일 역할
| 파일 | 역할 |
|------|------|
| `gps_app.c/h` | 앱 시작/종료, 이벤트 처리 |
| `gps_port.c/h` | HAL 연결 (UART/DMA) - **MCU 마이그레이션 시 수정** |
| `gps_role.c/h` | Base/Rover 역할 관리 |
| `base_auto_fix.c/h` | Base RTK 자동 고정 |

## 핵심 API
| 함수 | 설명 |
|------|------|
| `gps_app_start()` | 앱 시작 (board_config 기반 자동 설정) |
| `gps_app_stop()` | 앱 종료 |
| `gps_get_handle()` | GPS 핸들 조회 (레거시 호환) |
| `gps_send_command_sync()` | 명령어 동기 전송 |
| `gps_send_command_async()` | 명령어 비동기 전송 |

## 데이터 흐름
```
lib/gps 이벤트 → gps_app_evt_handler() → LED + event_bus → 다른 앱(LoRa, RS485, GSM)
```

## 주의사항
- UM982 초기화 명령어는 `um982_base_cmds[]`, `um982_rover_cmds[]` 배열에 정의
- 명령어 실패 시 재시도 로직: `gps_send_cmd_with_retry()`

## 구현 규칙 (신규 코드 작성 시)
- 드라이버 직접 접근 X → `gps_get_handle()` 사용
- 새 GPS 칩 추가 시: 초기화 명령어 배열 + async 함수 추가
