#ifndef STM32F0_POWER_H
#define STM32F0_POWER_H

#include <stdint.h>

void stm32f0_power_init(void);
void stm32f0_power_tickless_sleep(uint32_t expected_idle_ticks);

#endif
