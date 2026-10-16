#ifndef STM32F0_RTC_ADAPTER_H
#define STM32F0_RTC_ADAPTER_H

#include <stdbool.h>

#include "clock_types.h"

typedef enum {
    STM32F0_RTC_SETTING_NONE = 0,
    STM32F0_RTC_SETTING_HOUR,
    STM32F0_RTC_SETTING_MINUTE,
    STM32F0_RTC_SETTING_DAY,
    STM32F0_RTC_SETTING_MONTH,
    STM32F0_RTC_SETTING_YEAR
} Stm32f0RtcSetting;

/* Read an already-set RTC snapshot; return false on null input or HAL error. */
bool stm32f0_rtc_read(ClockDateTime *date_time);

/* Adjust one calendar field by one step and write it to the RTC. */
bool stm32f0_rtc_adjust(ClockDateTime *date_time,
                        Stm32f0RtcSetting setting,
                        int8_t direction);

#endif
