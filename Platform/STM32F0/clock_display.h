#ifndef STM32F0_CLOCK_DISPLAY_H
#define STM32F0_CLOCK_DISPLAY_H

#include "clock_application.h"

#define CLOCK_DISPLAY_BRIGHTNESS_MIN_LEVEL 1U
#define CLOCK_DISPLAY_BRIGHTNESS_MAX_LEVEL 16U
#define CLOCK_DISPLAY_BRIGHTNESS_DEFAULT_LEVEL 4U

void clock_display_init(void);
void clock_display_render(const ClockView *view);

#endif
