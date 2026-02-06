/**
 * @file stm32f4xx_hal.h
 * @brief STM32 HAL stub for host testing (intercepted by log.h)
 */
#ifndef STM32F4XX_HAL_H
#define STM32F4XX_HAL_H

#include <stdint.h>

extern uint32_t mock_tick_count;
static inline uint32_t HAL_GetTick(void) { return mock_tick_count; }

#endif /* STM32F4XX_HAL_H */
