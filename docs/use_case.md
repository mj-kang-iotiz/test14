# Use Cases
 
## 시스템 개요
 
### 장치 타입
- **Base Station**: RTK 보정 데이터(RTCM) 생성 및 송신
- **Rover**: RTK 보정 데이터 수신하여 고정밀 위치 측정
 
### 외부 통신 인터페이스
RS485, RS232, FDCAN 중 **하나만** 선택하여 외부 장치와 명령어/데이터 송수신

### 주의사항
- 각 유스케이스의 세부 동작은 구현에 따라 변경될 수 있음
- **핵심 흐름:**
  - **Base**: BLE/NTRIP/수동좌표 중 하나로 RTK Fix 달성 → 좌표 확정 → Fixed Station 모드로 RTCM 생성 → LoRa 브로드캐스트
  - **Rover**: LoRa/BLE/NTRIP 중 하나로 RTCM 수신 → RTK Fix 유지 → 고정밀 위치 출력


 
---
 
## Base Station
 
### UC-B01: BLE 경유 RTCM 보정 → Base 동작
- **트리거**: BLE 연결 + 외부 앱에서 NTRIP 사용
- **동작**:
  1. GPS에서 GGA 데이터 생성
  2. BLE로 GGA 외부 전송
  3. 외부 앱이 NTRIP 서버에서 RTCM 수신
  4. BLE로 RTCM 데이터 수신
  5. GPS에 RTCM 전달 → RTK Fix 진입
  6. 알고리즘 수행 → 고정 좌표 계산
  7. Fixed Station 모드로 전환
  8. LoRa로 RTCM 송신 시작
- **결과**: Base Station으로서 RTCM 브로드캐스트
- **관련 모듈**: `app/ble/`, `app/gps/`, `app/lora/`
 
### UC-B02: NTRIP 직접 연결 → Base 동작
- **트리거**: GSM 초기화 완료 + NTRIP 설정값 존재
- **동작**:
  1. GSM으로 NTRIP 서버 연결
  2. GGA 전송 → RTCM 수신
  3. GPS에 RTCM 전달 → RTK Fix 진입
  4. 알고리즘 수행 → 고정 좌표 계산
  5. Fixed Station 모드로 전환
  6. LoRa로 RTCM 송신 시작
- **결과**: Base Station으로서 RTCM 브로드캐스트
- **관련 모듈**: `app/gsm/`, `app/gps/`, `app/lora/`
 
### UC-B03: 고정 좌표 직접 입력 → Base 동작
- **트리거**: BLE로 고정 좌표값 수신
- **동작**:
  1. BLE에서 좌표값 파싱
  2. GPS에 고정 좌표 설정
  3. 즉시 Fixed Station 모드로 동작
  4. LoRa로 RTCM 송신 시작
- **결과**: Base Station으로서 RTCM 브로드캐스트
- **관련 모듈**: `app/ble/`, `app/gps/`, `app/lora/`
 
---
 
## Rover
 
### UC-R01: LoRa RTCM 수신 → RTK 보정
- **트리거**: LoRa에서 RTCM 데이터 수신
- **동작**:
  1. LoRa에서 RTCM 파싱
  2. GPS에 RTCM 전달
  3. RTK Fix/Float 상태 진입
- **결과**: 고정밀 위치 데이터 출력
- **관련 모듈**: `app/lora/`, `app/gps/`
 
### UC-R02: BLE 경유 RTCM 수신 → RTK 보정
- **트리거**: BLE로 RTCM 데이터 수신
- **동작**:
  1. BLE에서 GGA 외부 전송
  2. 외부 앱 경유 RTCM 수신
  3. GPS에 RTCM 전달
  4. RTK Fix/Float 상태 진입
- **결과**: 고정밀 위치 데이터 출력
- **관련 모듈**: `app/ble/`, `app/gps/`
 
### UC-R03: NTRIP 직접 연결 → RTK 보정
- **트리거**: GSM 초기화 완료 + NTRIP 설정값 존재
- **동작**:
  1. GSM으로 NTRIP 서버 연결
  2. GGA 전송 → RTCM 수신
  3. GPS에 RTCM 전달
  4. RTK Fix/Float 상태 진입
- **결과**: 고정밀 위치 데이터 출력
- **관련 모듈**: `app/gsm/`, `app/gps/`
 
---
 
## 공통
 
### UC-C01: 외부 통신 명령어 처리 (RS485/RS232/FDCAN)
- **트리거**: 외부 통신으로 명령어 수신 또는 타이머
- **조건**: RS485, RS232, FDCAN 중 하나만 활성화
- **동작**:
  1. 명령어 수신 시 파싱 및 처리
  2. 주기적으로 상태/위치 데이터 송신
- **관련 모듈**: `app/rs485/`, `lib/rs485/`, `lib/rs232/`
 
### UC-C02: BLE 명령어 처리
- **트리거**: BLE로 명령어 수신 또는 타이머
- **동작**:
  1. 명령어 파싱 및 처리
  2. 주기적으로 상태/위치 데이터 송신
  3. (Rover) RTCM 데이터 수신 시 GPS 전달
- **관련 모듈**: `app/ble/`, `lib/ble/`
 
### UC-C03: GPS 이벤트 처리
- **트리거**: GPS에서 NMEA/UBX 패킷 수신
- **동작**:
  1. 패킷 파싱 (GGA, RMC 등)
  2. 위치/상태 데이터 업데이트
  3. 이벤트 버스로 다른 모듈에 알림
- **관련 모듈**: `app/gps/`, `lib/gps/`
 
### UC-C04: 설정 파라미터 관리
- **트리거**: 파라미터 변경 명령 또는 부팅
- **동작**:
  1. Flash에서 파라미터 로드
  2. 변경 시 Flash에 저장
  3. 각 모듈에 설정값 반영
- **관련 모듈**: `app/params/`
 
### UC-C05: 이벤트 버스
- **트리거**: 모듈에서 이벤트 발생
- **동작**:
  1. 이벤트 발행
  2. 구독 중인 모듈들에 전달
- **관련 모듈**: `app/core/`