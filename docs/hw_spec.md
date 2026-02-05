# Hardware Specification

## MCU
- **Part**: STM32H562VGT6
- **Package**: LQFP100
- **Clock**: 247MHz (HSE 26MHz)

## Components

| 통신 | 모델 | 인터페이스 | 통신속도 | 비고 |
|-----|------|----------|------|--------|
| LTE | EC25 | USART1 | 115200bps |  |
| GPS | UM982 | USART2 | 115200bps | Base/Rover 공용 |
| LoRa | RAK4270 | USART3 | 115200bps |  |
| BLE | BoT-nLE521 | UART4 | 115200bps |  |
| RS485 | THVD1520DR | UART5 | 115200bps | |
| DEBUG | | USART6 | 115200bps | 거의 사용x RTT로 대체 |
| RS232 | MAX3232ECWE | UART8 | 115200bps | |
| FDCAN | MCP2562 | FDCAN | 모름 | |

## Pin Map

### LTE (USART1)
| 기능 | 핀 | 비고 |
|------|---|------|
| TX | PB14 | - | 
| RX | PB15 | - |
| DTR | PA10 | - | 
| RESET | PA9 | - |
| PWRKEY | PA8 | - |
| AP_READY | PC9 | - |

### GPS (USART2)
| 기능 | 핀 | 비고 |
|------|---|------|
| TX | PA2 | - | 
| RX | PA3 | - |
| RESET | PA5 | - |

### LoRa (USART3)
| 기능 | 핀 | 비고 |
|------|---|------|
| TX | PB10 | - | 
| RX | PC4 | - |
| RESET | PC5 | - |

### BLE (UART4)
| 기능 | 핀 | 비고 |
|------|---|------|
| TX | PA0 | - | 
| RX | PA1 | - |
| EN | PE6 | - |
| SLEEP | PE5 | - |

### RS485 (UART5)
| 기능 | 핀 | 비고 |
|------|---|------|
| TX | PB6 | - | 
| RX | PB5 | - |
| RE | PB7 | - |
| DE | PB8 | - |

### DEBUG (USART6)
| 기능 | 핀 | 비고 |
|------|---|------|
| TX | PC6 | - | 
| RX | PC7 | - |

### RS232 (UART8)
| 기능 | 핀 | 비고 |
|------|---|------|
| TX | PE2 | - | 
| RX | PE0 | - |

### FDCAN (FDCAN)
| 기능 | 핀 | 비고 |
|------|---|------|
| TX | PD1 | - | 
| RX | PD0 | - |
| STBY | PD2 | - |

### LED
| 기능 | 핀 | 용도 |
|------|---|------|
| LED1_R | PE9 | main문 진입 이후 |
| LED1_G | PE11 | main문 진입 이후 |
| LED2_R | PB0 | LTE or LoRa 통신 동작 여부 |
| LED2_G | PB1 | LTE or LoRa 통신 동작 여부 |
| LED3_R | PE13 | 알고리즘 동작 상태 (rover라면 fix base라면 base station 모드) |
| LED3_G | PE14 | 알고리즘 동작 상태 (rover라면 fix base라면 base station 모드) |
| LED4_R | PD12 | ROVER OR BASE |
| LED4_G | PD13 | ROVER OR BASE |

### ROVER OR BASE
| 기능 | 핀 | 비고 |
|------|---|------|
| ROVER_OR_BASE | PD7 | - |

## DMA Channel Assignment
| Channel | 용도 |
|---------|------|
| GPDMA1_CH4 | LTE RX |
| GPDMA1_CH5 | BLE RX |
| GPDMA1_CH6 | RS485 RX |
| GPDMA1_CH7 | DEBUG RX |
| GPDMA2_CH4 | GPS RX |
| GPDMA2_CH5 | LoRa RX |
| GPDMA2_CH6 | RS232 RX |


## Interrupt Priority
| IRQ | 우선순위 | 용도 |
|-----|---------|------|
| USART1 | 5 | LTE |
| USART2 | 5 | GPS |
| USART3 | 5 | LoRa |
| UART4 | 5 | BLE |
| UART5 | 5 | RS485 |
| UART6 | 5 | DEBUG |
| UART8 | 5 | RS232 |
| FDCAN1_0 | 5 | FDCAN |
| FDCAN1_1 | 5 | FDCAN |
| GPDMA1,2 | 5 | - |
