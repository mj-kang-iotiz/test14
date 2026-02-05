# GPS 드라이버 (lib/gps)

## 설계 의도
- Chain Parser: 프로토콜 자동 감지 (NMEA/Unicore/RTCM 혼합 수신)
- 이벤트 기반: 20Hz 연속 수신 처리
- 공용 데이터(`gps->data`): 여러 프로토콜이 같은 필드 업데이트

## 파일 역할
| 파일 | 역할 |
|------|------|
| `gps.c/h` | 초기화, 태스크, 명령어 전송, 메인 구조체 정의 |
| `gps_parser.c/h` | 파서 체인 메인 루프 |
| `gps_nmea.c/h` | NMEA 파서 |
| `gps_unicore.c/h` | Unicore Binary 파서 (UM982) |
| `rtcm.c/h` | RTCM 파서. LoRa와 관련된 결합도가 높아 수정이 필요 |
| `gps_event.h` | GPS 프로토콜 및 이벤트 타입 정의 |
| `gps_types.h` | 기본 타입, HAL ops 인터페이스 |
| `gps_proto_def.h` | X-Macro 프로토콜 테이블 (NMEA/Unicore/RTCM) |

## 핵심 API
| 함수 | 설명 |
|------|------|
| `gps_init()` | 드라이버 초기화 |
| `gps_send_cmd_sync()` | 명령어 동기 전송 (mutex 보호) |
| `gps_parser_process()` | 파서 체인 실행 (태스크에서 호출) |

## 데이터 흐름
```
UART+DMA(Idle) → rx_buf(ringbuffer) → gps_parser_process() → gps_event → app
```

## 주의사항
- BESTNAV가 GGA보다 정확 (위치/속도 둘 다 포함)
- 듀얼 안테나 헤딩 사용
- 명령어 전송: 반드시 `gps_send_cmd_sync()` (mutex 보호)
- 더이상 F9P는 사용하지 않음 UM982만 사용
- GGA raw 패킷 저장 필요 (ntrip이나 외부 인터페이스로 전송)
- GPS 이벤트에서 NMEA, unicore protocol 데이터중 어느것으로 받을지 설정이 필요(default: unicore protocol 사용)
    - NMEA와 UNICORE protocol둘다 데이터 표현이 가능하지만 UNICORE protocol 이 더 정확하고 빠르다. 따라서 특별한 경우 아니면 UNICORE protocol 사용을 권장한다.
- gps_hal_ops_t 인터페이스를 이용하여 hw 결합도를 낮춰야 함.

## 구현 규칙 (신규 코드 작성 시)
- 데이터 복사 X → 링버퍼 사용
- HAL 직접 호출 X → `gps->ops` 통해 호출
- 새 메시지 추가: 아래 확장 패턴 따를 것

## 확장 패턴
| 추가 대상 | 수정 위치 |
|----------|----------|
| 새 NMEA | `gps_nmea.c` NMEA_MSG_TABLE |
| 새 Unicore | `gps_unicore.c` msg_table |
| 새 이벤트 | `gps_event.h` enum 추가 |