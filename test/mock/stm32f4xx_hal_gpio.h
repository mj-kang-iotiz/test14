/**
 * @file stm32f4xx_hal_gpio.h
 * @brief STM32 HAL GPIO stub for host testing
 */
#ifndef STM32F4XX_HAL_GPIO_H
#define STM32F4XX_HAL_GPIO_H

#include "stm32f4xx.h"

typedef enum {
    GPIO_PIN_RESET = 0U,
    GPIO_PIN_SET
} GPIO_PinState;

typedef struct {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
} GPIO_InitTypeDef;

static inline void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init) {
    (void)GPIOx; (void)GPIO_Init;
}

static inline GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    (void)GPIOx; (void)GPIO_Pin;
    return GPIO_PIN_RESET;
}

static inline void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                                     GPIO_PinState PinState) {
    (void)GPIOx; (void)GPIO_Pin; (void)PinState;
}

static inline void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    (void)GPIOx; (void)GPIO_Pin;
}

#endif /* STM32F4XX_HAL_GPIO_H */
