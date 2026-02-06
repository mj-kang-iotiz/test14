/**
 * @file stm32f4xx.h
 * @brief STM32F4 device stub for host testing
 *
 * Provides minimal type definitions (GPIO_TypeDef, IRQn, pin/port macros)
 * so that app-level headers can be parsed without the real CMSIS/HAL.
 */
#ifndef STM32F4XX_H
#define STM32F4XX_H

#include <stdint.h>

/*==== IRQ numbers ====*/
typedef enum {
    NonMaskableInt_IRQn = -14,
    HardFault_IRQn = -13,
    SysTick_IRQn = -1,
    USART1_IRQn = 37,
    USART2_IRQn = 38,
    USART3_IRQn = 39,
    DMA1_Stream0_IRQn = 11,
} IRQn_Type;

/*==== GPIO stub ====*/
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

/*==== GPIO pin defines ====*/
#define GPIO_PIN_0  ((uint16_t)0x0001)
#define GPIO_PIN_1  ((uint16_t)0x0002)
#define GPIO_PIN_2  ((uint16_t)0x0004)
#define GPIO_PIN_3  ((uint16_t)0x0008)
#define GPIO_PIN_4  ((uint16_t)0x0010)
#define GPIO_PIN_5  ((uint16_t)0x0020)
#define GPIO_PIN_6  ((uint16_t)0x0040)
#define GPIO_PIN_7  ((uint16_t)0x0080)
#define GPIO_PIN_8  ((uint16_t)0x0100)
#define GPIO_PIN_9  ((uint16_t)0x0200)
#define GPIO_PIN_10 ((uint16_t)0x0400)
#define GPIO_PIN_11 ((uint16_t)0x0800)
#define GPIO_PIN_12 ((uint16_t)0x1000)
#define GPIO_PIN_13 ((uint16_t)0x2000)
#define GPIO_PIN_14 ((uint16_t)0x4000)
#define GPIO_PIN_15 ((uint16_t)0x8000)

/*==== GPIO port instances (dummy addresses) ====*/
#define GPIOA ((GPIO_TypeDef *)0x40020000UL)
#define GPIOB ((GPIO_TypeDef *)0x40020400UL)
#define GPIOC ((GPIO_TypeDef *)0x40020800UL)
#define GPIOD ((GPIO_TypeDef *)0x40020C00UL)
#define GPIOE ((GPIO_TypeDef *)0x40021000UL)

/*==== UART stub ====*/
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
} USART_TypeDef;

/*==== HAL status ====*/
typedef enum {
    HAL_OK = 0x00U,
    HAL_ERROR = 0x01U,
    HAL_BUSY = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

/*==== NVIC stubs ====*/
static inline void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority,
                                        uint32_t SubPriority) {
    (void)IRQn;
    (void)PreemptPriority;
    (void)SubPriority;
}
static inline void HAL_NVIC_EnableIRQ(IRQn_Type IRQn) {
    (void)IRQn;
}
static inline void HAL_NVIC_DisableIRQ(IRQn_Type IRQn) {
    (void)IRQn;
}

/*==== HAL delay ====*/
static inline void HAL_Delay(uint32_t Delay) {
    (void)Delay;
}

extern uint32_t mock_tick_count;
static inline uint32_t HAL_GetTick(void) {
    return mock_tick_count;
}

#endif /* STM32F4XX_H */
