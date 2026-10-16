/*
 * Copyright (c) 2019 STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 4. This software must execute solely and exclusively on devices manufactured
 *    by or for STMicroelectronics.
 * 5. Redistribution and use other than as permitted by this license is void
 *    and will automatically terminate your rights under this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
 */

#include "board.h"
#include "clock_display.h"
#include "gpio.h"
#include "i2c.h"
#include "power.h"
#include "rtc.h"
#include "rtos_tasks.h"
#include "stm32f0xx_hal.h"
#include "temperature_sensor.h"

int main(void)
{
    HAL_Init();
    /* Configure the system clock before initializing clock-dependent peripherals. */
    SystemClock_Config();

    /* Initialize hardware before starting tasks that use the RTC and OLED. */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_RTC_Init();
    stm32f0_power_init();
    (void)stm32f0_internal_sensors_init();

    HAL_Delay(200U);
    clock_display_init();
    stm32f0_rtos_start();
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    RCC_ClkInitTypeDef clock = {0};
    RCC_PeriphCLKInitTypeDef peripheral_clock = {0};

    /* Use the PLL for the core and LSI as the independent RTC clock source. */
    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSI |
                                RCC_OSCILLATORTYPE_LSI;
    oscillator.HSIState = RCC_HSI_ON;
    oscillator.HSICalibrationValue = 16U;
    oscillator.LSIState = RCC_LSI_ON;
    oscillator.PLL.PLLState = RCC_PLL_ON;
    oscillator.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    oscillator.PLL.PLLMUL = RCC_PLL_MUL4;
    oscillator.PLL.PREDIV = RCC_PREDIV_DIV1;
    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) {
        Error_Handler();
    }

    clock.ClockType = RCC_CLOCKTYPE_HCLK |
                      RCC_CLOCKTYPE_SYSCLK |
                      RCC_CLOCKTYPE_PCLK1;
    clock.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clock.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clock.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }

    peripheral_clock.PeriphClockSelection = RCC_PERIPHCLK_I2C1 |
                                             RCC_PERIPHCLK_RTC;
    peripheral_clock.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
    peripheral_clock.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if (HAL_RCCEx_PeriphCLKConfig(&peripheral_clock) != HAL_OK) {
        Error_Handler();
    }

    /* Keep HAL_GetTick in milliseconds for the application timing calculations. */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    HAL_NVIC_SetPriority(SysTick_IRQn, 3U, 0U);
}

void Error_Handler(void)
{
    __disable_irq();
    for (;;) {
    }
}
