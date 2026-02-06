/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32h5xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
extern DMA_NodeTypeDef Node_GPDMA1_Channel5;

extern DMA_QListTypeDef List_GPDMA1_Channel5;

extern DMA_HandleTypeDef handle_GPDMA1_Channel5;

extern DMA_NodeTypeDef Node_GPDMA1_Channel6;

extern DMA_QListTypeDef List_GPDMA1_Channel6;

extern DMA_HandleTypeDef handle_GPDMA1_Channel6;

extern DMA_NodeTypeDef Node_GPDMA2_Channel6;

extern DMA_QListTypeDef List_GPDMA2_Channel6;

extern DMA_HandleTypeDef handle_GPDMA2_Channel6;

extern DMA_NodeTypeDef Node_GPDMA1_Channel4;

extern DMA_QListTypeDef List_GPDMA1_Channel4;

extern DMA_HandleTypeDef handle_GPDMA1_Channel4;

extern DMA_NodeTypeDef Node_GPDMA2_Channel4;

extern DMA_QListTypeDef List_GPDMA2_Channel4;

extern DMA_HandleTypeDef handle_GPDMA2_Channel4;

extern DMA_NodeTypeDef Node_GPDMA2_Channel5;

extern DMA_QListTypeDef List_GPDMA2_Channel5;

extern DMA_HandleTypeDef handle_GPDMA2_Channel5;

extern DMA_NodeTypeDef Node_GPDMA1_Channel7;

extern DMA_QListTypeDef List_GPDMA1_Channel7;

extern DMA_HandleTypeDef handle_GPDMA1_Channel7;

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void) {
    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */

    /* System interrupt init*/
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}

/**
  * @brief FDCAN MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hfdcan: FDCAN handle pointer
  * @retval None
  */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (hfdcan->Instance == FDCAN1) {
        /* USER CODE BEGIN FDCAN1_MspInit 0 */

        /* USER CODE END FDCAN1_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_FDCAN_CLK_ENABLE();

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**FDCAN1 GPIO Configuration
    PD0     ------> FDCAN1_RX
    PD1     ------> FDCAN1_TX
    */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* FDCAN1 interrupt Init */
        HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
        /* USER CODE BEGIN FDCAN1_MspInit 1 */

        /* USER CODE END FDCAN1_MspInit 1 */
    }
}

/**
  * @brief FDCAN MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hfdcan: FDCAN handle pointer
  * @retval None
  */
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *hfdcan) {
    if (hfdcan->Instance == FDCAN1) {
        /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

        /* USER CODE END FDCAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_FDCAN_CLK_DISABLE();

        /**FDCAN1 GPIO Configuration
    PD0     ------> FDCAN1_RX
    PD1     ------> FDCAN1_TX
    */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1);

        /* FDCAN1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
        /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

        /* USER CODE END FDCAN1_MspDeInit 1 */
    }
}

/**
  * @brief UART MSP Initialization
  * This function configures the hardware resources used in this example
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    DMA_NodeConfTypeDef NodeConfig;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (huart->Instance == UART4) {
        /* USER CODE BEGIN UART4_MspInit 0 */

        /* USER CODE END UART4_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4;
        PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_UART4_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**UART4 GPIO Configuration
    PA0     ------> UART4_TX
    PA1     ------> UART4_RX
    */
        GPIO_InitStruct.Pin = BLE_UART4_TX_Pin | BLE_UART4_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* UART4 DMA Init */
        /* GPDMA1_REQUEST_UART4_RX Init */
        NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
        NodeConfig.Init.Request = GPDMA1_REQUEST_UART4_RX;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_FIXED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel5) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel5, NULL, &Node_GPDMA1_Channel5) !=
            HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel5) != HAL_OK) {
            Error_Handler();
        }

        handle_GPDMA1_Channel5.Instance = GPDMA1_Channel5;
        handle_GPDMA1_Channel5.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel5.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA1_Channel5.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA1_Channel5.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA1_Channel5.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel5) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel5, &List_GPDMA1_Channel5) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA1_Channel5);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel5, DMA_CHANNEL_NPRIV) != HAL_OK) {
            Error_Handler();
        }

        /* UART4 interrupt Init */
        HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
        /* USER CODE BEGIN UART4_MspInit 1 */

        /* USER CODE END UART4_MspInit 1 */
    }
    else if (huart->Instance == UART5) {
        /* USER CODE BEGIN UART5_MspInit 0 */

        /* USER CODE END UART5_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART5;
        PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_UART5_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**UART5 GPIO Configuration
    PB5     ------> UART5_RX
    PB6     ------> UART5_TX
    */
        GPIO_InitStruct.Pin = RS485_UART5_RX_Pin | RS485_UART5_TX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_UART5;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* UART5 DMA Init */
        /* GPDMA1_REQUEST_UART5_RX Init */
        NodeConfig.NodeType = DMA_GPDMA_2D_NODE;
        NodeConfig.Init.Request = GPDMA1_REQUEST_UART5_RX;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_FIXED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.RepeatBlockConfig.RepeatCount = 1;
        NodeConfig.RepeatBlockConfig.SrcAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.DestAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.BlkSrcAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.BlkDestAddrOffset = 0;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel6) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel6, NULL, &Node_GPDMA1_Channel6) !=
            HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel6) != HAL_OK) {
            Error_Handler();
        }

        handle_GPDMA1_Channel6.Instance = GPDMA1_Channel6;
        handle_GPDMA1_Channel6.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel6.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA1_Channel6.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA1_Channel6.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA1_Channel6.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel6) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel6, &List_GPDMA1_Channel6) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA1_Channel6);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel6, DMA_CHANNEL_NPRIV) != HAL_OK) {
            Error_Handler();
        }

        /* UART5 interrupt Init */
        HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(UART5_IRQn);
        /* USER CODE BEGIN UART5_MspInit 1 */

        /* USER CODE END UART5_MspInit 1 */
    }
    else if (huart->Instance == UART8) {
        /* USER CODE BEGIN UART8_MspInit 0 */

        /* USER CODE END UART8_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART8;
        PeriphClkInitStruct.Uart8ClockSelection = RCC_UART8CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_UART8_CLK_ENABLE();

        __HAL_RCC_GPIOE_CLK_ENABLE();
        /**UART8 GPIO Configuration
    PE2     ------> UART8_TX
    PE0     ------> UART8_RX
    */
        GPIO_InitStruct.Pin = RS232_UART8_TX_Pin | RS232_UART8_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        /* UART8 DMA Init */
        /* GPDMA2_REQUEST_UART8_RX Init */
        NodeConfig.NodeType = DMA_GPDMA_2D_NODE;
        NodeConfig.Init.Request = GPDMA2_REQUEST_UART8_RX;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_FIXED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.RepeatBlockConfig.RepeatCount = 1;
        NodeConfig.RepeatBlockConfig.SrcAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.DestAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.BlkSrcAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.BlkDestAddrOffset = 0;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA2_Channel6) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA2_Channel6, NULL, &Node_GPDMA2_Channel6) !=
            HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA2_Channel6) != HAL_OK) {
            Error_Handler();
        }

        handle_GPDMA2_Channel6.Instance = GPDMA2_Channel6;
        handle_GPDMA2_Channel6.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA2_Channel6.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA2_Channel6.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA2_Channel6.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA2_Channel6.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA2_Channel6) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA2_Channel6, &List_GPDMA2_Channel6) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA2_Channel6);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA2_Channel6, DMA_CHANNEL_NPRIV) != HAL_OK) {
            Error_Handler();
        }

        /* UART8 interrupt Init */
        HAL_NVIC_SetPriority(UART8_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(UART8_IRQn);
        /* USER CODE BEGIN UART8_MspInit 1 */

        /* USER CODE END UART8_MspInit 1 */
    }
    else if (huart->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**USART1 GPIO Configuration
    PB14     ------> USART1_TX
    PB15     ------> USART1_RX
    */
        GPIO_InitStruct.Pin = LTE_UART1_TX_Pin | LTE_UART1_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* USART1 DMA Init */
        /* GPDMA1_REQUEST_USART1_RX Init */
        NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
        NodeConfig.Init.Request = GPDMA1_REQUEST_USART1_RX;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_FIXED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel4) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel4, NULL, &Node_GPDMA1_Channel4) !=
            HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel4) != HAL_OK) {
            Error_Handler();
        }

        handle_GPDMA1_Channel4.Instance = GPDMA1_Channel4;
        handle_GPDMA1_Channel4.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel4.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA1_Channel4.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA1_Channel4.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA1_Channel4.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel4) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel4, &List_GPDMA1_Channel4) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA1_Channel4);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel4, DMA_CHANNEL_NPRIV) != HAL_OK) {
            Error_Handler();
        }

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    }
    else if (huart->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
        PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
        GPIO_InitStruct.Pin = GPS_UART2_TX_Pin | GPS_UART2_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 DMA Init */
        /* GPDMA2_REQUEST_USART2_RX Init */
        NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
        NodeConfig.Init.Request = GPDMA2_REQUEST_USART2_RX;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_FIXED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA2_Channel4) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA2_Channel4, NULL, &Node_GPDMA2_Channel4) !=
            HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA2_Channel4) != HAL_OK) {
            Error_Handler();
        }

        handle_GPDMA2_Channel4.Instance = GPDMA2_Channel4;
        handle_GPDMA2_Channel4.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA2_Channel4.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA2_Channel4.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA2_Channel4.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA2_Channel4.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA2_Channel4) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA2_Channel4, &List_GPDMA2_Channel4) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA2_Channel4);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA2_Channel4, DMA_CHANNEL_NPRIV) != HAL_OK) {
            Error_Handler();
        }

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    }
    else if (huart->Instance == USART3) {
        /* USER CODE BEGIN USART3_MspInit 0 */

        /* USER CODE END USART3_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
        PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART3_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**USART3 GPIO Configuration
    PC4     ------> USART3_RX
    PB10     ------> USART3_TX
    */
        GPIO_InitStruct.Pin = LORA_UART3_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
        HAL_GPIO_Init(LORA_UART3_RX_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LORA_UART3_TX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
        HAL_GPIO_Init(LORA_UART3_TX_GPIO_Port, &GPIO_InitStruct);

        /* USART3 DMA Init */
        /* GPDMA2_REQUEST_USART3_RX Init */
        NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
        NodeConfig.Init.Request = GPDMA2_REQUEST_USART3_RX;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_FIXED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA2_Channel5) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA2_Channel5, NULL, &Node_GPDMA2_Channel5) !=
            HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA2_Channel5) != HAL_OK) {
            Error_Handler();
        }

        handle_GPDMA2_Channel5.Instance = GPDMA2_Channel5;
        handle_GPDMA2_Channel5.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA2_Channel5.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA2_Channel5.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA2_Channel5.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA2_Channel5.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA2_Channel5) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA2_Channel5, &List_GPDMA2_Channel5) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA2_Channel5);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA2_Channel5, DMA_CHANNEL_NPRIV) != HAL_OK) {
            Error_Handler();
        }

        /* USART3 interrupt Init */
        HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
        /* USER CODE BEGIN USART3_MspInit 1 */

        /* USER CODE END USART3_MspInit 1 */
    }
    else if (huart->Instance == USART6) {
        /* USER CODE BEGIN USART6_MspInit 0 */

        /* USER CODE END USART6_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
        PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART6_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
        GPIO_InitStruct.Pin = DEBUG_UART6_TX_Pin | DEBUG_UART6_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* USART6 DMA Init */
        /* GPDMA1_REQUEST_USART6_RX Init */
        NodeConfig.NodeType = DMA_GPDMA_2D_NODE;
        NodeConfig.Init.Request = GPDMA1_REQUEST_USART6_RX;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_FIXED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.RepeatBlockConfig.RepeatCount = 1;
        NodeConfig.RepeatBlockConfig.SrcAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.DestAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.BlkSrcAddrOffset = 0;
        NodeConfig.RepeatBlockConfig.BlkDestAddrOffset = 0;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel7) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel7, NULL, &Node_GPDMA1_Channel7) !=
            HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel7) != HAL_OK) {
            Error_Handler();
        }

        handle_GPDMA1_Channel7.Instance = GPDMA1_Channel7;
        handle_GPDMA1_Channel7.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel7.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA1_Channel7.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA1_Channel7.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA1_Channel7.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel7) != HAL_OK) {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel7, &List_GPDMA1_Channel7) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA1_Channel7);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel7, DMA_CHANNEL_NPRIV) != HAL_OK) {
            Error_Handler();
        }

        /* USART6 interrupt Init */
        HAL_NVIC_SetPriority(USART6_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART6_IRQn);
        /* USER CODE BEGIN USART6_MspInit 1 */

        /* USER CODE END USART6_MspInit 1 */
    }
}

/**
  * @brief UART MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART4) {
        /* USER CODE BEGIN UART4_MspDeInit 0 */

        /* USER CODE END UART4_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_UART4_CLK_DISABLE();

        /**UART4 GPIO Configuration
    PA0     ------> UART4_TX
    PA1     ------> UART4_RX
    */
        HAL_GPIO_DeInit(GPIOA, BLE_UART4_TX_Pin | BLE_UART4_RX_Pin);

        /* UART4 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);

        /* UART4 interrupt DeInit */
        HAL_NVIC_DisableIRQ(UART4_IRQn);
        /* USER CODE BEGIN UART4_MspDeInit 1 */

        /* USER CODE END UART4_MspDeInit 1 */
    }
    else if (huart->Instance == UART5) {
        /* USER CODE BEGIN UART5_MspDeInit 0 */

        /* USER CODE END UART5_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_UART5_CLK_DISABLE();

        /**UART5 GPIO Configuration
    PB5     ------> UART5_RX
    PB6     ------> UART5_TX
    */
        HAL_GPIO_DeInit(GPIOB, RS485_UART5_RX_Pin | RS485_UART5_TX_Pin);

        /* UART5 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);

        /* UART5 interrupt DeInit */
        HAL_NVIC_DisableIRQ(UART5_IRQn);
        /* USER CODE BEGIN UART5_MspDeInit 1 */

        /* USER CODE END UART5_MspDeInit 1 */
    }
    else if (huart->Instance == UART8) {
        /* USER CODE BEGIN UART8_MspDeInit 0 */

        /* USER CODE END UART8_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_UART8_CLK_DISABLE();

        /**UART8 GPIO Configuration
    PE2     ------> UART8_TX
    PE0     ------> UART8_RX
    */
        HAL_GPIO_DeInit(GPIOE, RS232_UART8_TX_Pin | RS232_UART8_RX_Pin);

        /* UART8 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);

        /* UART8 interrupt DeInit */
        HAL_NVIC_DisableIRQ(UART8_IRQn);
        /* USER CODE BEGIN UART8_MspDeInit 1 */

        /* USER CODE END UART8_MspDeInit 1 */
    }
    else if (huart->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
    PB14     ------> USART1_TX
    PB15     ------> USART1_RX
    */
        HAL_GPIO_DeInit(GPIOB, LTE_UART1_TX_Pin | LTE_UART1_RX_Pin);

        /* USART1 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);

        /* USART1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    }
    else if (huart->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
        HAL_GPIO_DeInit(GPIOA, GPS_UART2_TX_Pin | GPS_UART2_RX_Pin);

        /* USART2 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);

        /* USART2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    }
    else if (huart->Instance == USART3) {
        /* USER CODE BEGIN USART3_MspDeInit 0 */

        /* USER CODE END USART3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART3_CLK_DISABLE();

        /**USART3 GPIO Configuration
    PC4     ------> USART3_RX
    PB10     ------> USART3_TX
    */
        HAL_GPIO_DeInit(LORA_UART3_RX_GPIO_Port, LORA_UART3_RX_Pin);

        HAL_GPIO_DeInit(LORA_UART3_TX_GPIO_Port, LORA_UART3_TX_Pin);

        /* USART3 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);

        /* USART3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART3_IRQn);
        /* USER CODE BEGIN USART3_MspDeInit 1 */

        /* USER CODE END USART3_MspDeInit 1 */
    }
    else if (huart->Instance == USART6) {
        /* USER CODE BEGIN USART6_MspDeInit 0 */

        /* USER CODE END USART6_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART6_CLK_DISABLE();

        /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
        HAL_GPIO_DeInit(GPIOC, DEBUG_UART6_TX_Pin | DEBUG_UART6_RX_Pin);

        /* USART6 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);

        /* USART6 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART6_IRQn);
        /* USER CODE BEGIN USART6_MspDeInit 1 */

        /* USER CODE END USART6_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
