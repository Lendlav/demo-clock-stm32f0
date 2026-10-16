#ifndef STM32F0_CLOCK_DISPLAY_H
#define STM32F0_CLOCK_DISPLAY_H

#include "clock_application.h"

void clock_display_init(void);
void clock_display_render(const ClockView *view);

#endif
