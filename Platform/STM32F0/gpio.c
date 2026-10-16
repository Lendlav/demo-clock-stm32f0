/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

static const uint16_t button_pins[3] = {
  GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2
};

static uint8_t stable_buttons;
static uint8_t candidate_buttons;
static uint8_t debounce_samples[3];
static uint8_t hold_ticks[3];

uint8_t stm32f0_buttons_poll(void)
{
  uint8_t events = 0U;

  for (uint8_t index = 0U; index < 3U; ++index) {
    const uint8_t mask = (uint8_t)(1U << index);
    const uint8_t pressed =
        (HAL_GPIO_ReadPin(GPIOA, button_pins[index]) == GPIO_PIN_RESET) ? 1U : 0U;

    if ((pressed != 0U) != ((candidate_buttons & mask) != 0U)) {
      candidate_buttons ^= mask;
      debounce_samples[index] = 1U;
    } else if (debounce_samples[index] < 2U) {
      ++debounce_samples[index];
    }

    if ((debounce_samples[index] >= 2U) &&
        (((stable_buttons & mask) != 0U) != ((candidate_buttons & mask) != 0U))) {
      stable_buttons ^= mask;
      debounce_samples[index] = 0U;
      hold_ticks[index] = 0U;
      if ((stable_buttons & mask) != 0U) {
        events |= mask;
      }
    }

    /* MODE advances once; holding + or - repeats after 500 ms, then every 150 ms. */
    if ((index != 0U) &&
        ((stable_buttons & candidate_buttons & mask) != 0U) &&
        ((events & mask) == 0U)) {
      if (hold_ticks[index] < 10U) {
        ++hold_ticks[index];
        if (hold_ticks[index] == 10U) {
          events |= mask;
        }
      } else if (++hold_ticks[index] >= 13U) {
        hold_ticks[index] = 10U;
        events |= mask;
      }
    }
  }

  return events;
}

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Analog mode avoids leaving unused package pins with floating inputs. */
  GPIO_InitStruct.Pin = GPIO_PIN_All &
                        ~(GPIO_PIN_9 | GPIO_PIN_10 |
                          GPIO_PIN_13 | GPIO_PIN_14);
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Wire each active-low button between PA0..PA2 and GND. */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
