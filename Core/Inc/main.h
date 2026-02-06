/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_USART1_UART_Init(void);
void MX_FDCAN1_Init(void);
void MX_USART2_UART_Init(void);
void MX_UART4_Init(void);
void MX_UART5_Init(void);
void MX_UART8_Init(void);
void MX_USART6_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RS232_UART8_TX_Pin       GPIO_PIN_2
#define RS232_UART8_TX_GPIO_Port GPIOE
#define BLE_SLEEP_Pin            GPIO_PIN_5
#define BLE_SLEEP_GPIO_Port      GPIOE
#define BLE_UART_EN_Pin          GPIO_PIN_6
#define BLE_UART_EN_GPIO_Port    GPIOE
#define BLE_UART4_TX_Pin         GPIO_PIN_0
#define BLE_UART4_TX_GPIO_Port   GPIOA
#define BLE_UART4_RX_Pin         GPIO_PIN_1
#define BLE_UART4_RX_GPIO_Port   GPIOA
#define GPS_UART2_TX_Pin         GPIO_PIN_2
#define GPS_UART2_TX_GPIO_Port   GPIOA
#define GPS_UART2_RX_Pin         GPIO_PIN_3
#define GPS_UART2_RX_GPIO_Port   GPIOA
#define UM982_RESET_Pin          GPIO_PIN_5
#define UM982_RESET_GPIO_Port    GPIOA
#define LORA_UART3_RX_Pin        GPIO_PIN_4
#define LORA_UART3_RX_GPIO_Port  GPIOC
#define LORA_NRST_Pin            GPIO_PIN_5
#define LORA_NRST_GPIO_Port      GPIOC
#define LED_D2_R_Pin             GPIO_PIN_0
#define LED_D2_R_GPIO_Port       GPIOB
#define LED_D2_G_Pin             GPIO_PIN_1
#define LED_D2_G_GPIO_Port       GPIOB
#define LED_D1_R_Pin             GPIO_PIN_9
#define LED_D1_R_GPIO_Port       GPIOE
#define LED_D1_G_Pin             GPIO_PIN_11
#define LED_D1_G_GPIO_Port       GPIOE
#define LED_D3_R_Pin             GPIO_PIN_13
#define LED_D3_R_GPIO_Port       GPIOE
#define LED_D3_G_Pin             GPIO_PIN_14
#define LED_D3_G_GPIO_Port       GPIOE
#define LORA_UART3_TX_Pin        GPIO_PIN_10
#define LORA_UART3_TX_GPIO_Port  GPIOB
#define LTE_UART1_TX_Pin         GPIO_PIN_14
#define LTE_UART1_TX_GPIO_Port   GPIOB
#define LTE_UART1_RX_Pin         GPIO_PIN_15
#define LTE_UART1_RX_GPIO_Port   GPIOB
#define LED_D4_R_Pin             GPIO_PIN_12
#define LED_D4_R_GPIO_Port       GPIOD
#define LED_D4_G_Pin             GPIO_PIN_13
#define LED_D4_G_GPIO_Port       GPIOD
#define DEBUG_UART6_TX_Pin       GPIO_PIN_6
#define DEBUG_UART6_TX_GPIO_Port GPIOC
#define DEBUG_UART6_RX_Pin       GPIO_PIN_7
#define DEBUG_UART6_RX_GPIO_Port GPIOC
#define LTE_AP_READY_Pin         GPIO_PIN_9
#define LTE_AP_READY_GPIO_Port   GPIOC
#define LTE_PWRKEY_Pin           GPIO_PIN_8
#define LTE_PWRKEY_GPIO_Port     GPIOA
#define LTE_RESET_Pin            GPIO_PIN_9
#define LTE_RESET_GPIO_Port      GPIOA
#define LTE_DTR_Pin              GPIO_PIN_10
#define LTE_DTR_GPIO_Port        GPIOA
#define FDCAN_STBY_Pin           GPIO_PIN_2
#define FDCAN_STBY_GPIO_Port     GPIOD
#define ROVER_OR_BASE_Pin        GPIO_PIN_7
#define ROVER_OR_BASE_GPIO_Port  GPIOD
#define RS485_UART5_RX_Pin       GPIO_PIN_5
#define RS485_UART5_RX_GPIO_Port GPIOB
#define RS485_UART5_TX_Pin       GPIO_PIN_6
#define RS485_UART5_TX_GPIO_Port GPIOB
#define RS485_RE_Pin             GPIO_PIN_7
#define RS485_RE_GPIO_Port       GPIOB
#define RS485_DE_Pin             GPIO_PIN_8
#define RS485_DE_GPIO_Port       GPIOB
#define RS232_UART8_RX_Pin       GPIO_PIN_0
#define RS232_UART8_RX_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
