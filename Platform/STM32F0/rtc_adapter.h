#ifndef STM32F0_RTC_ADAPTER_H
#define STM32F0_RTC_ADAPTER_H

#include <stdbool.h>

#include "clock_types.h"

bool stm32f0_rtc_read(ClockDateTime *date_time);

#endif
