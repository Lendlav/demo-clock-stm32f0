#ifndef CLOCK_APPLICATION_H
#define CLOCK_APPLICATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "clock_types.h"

#define CLOCK_TICKER_WIDTH 20U

typedef struct {
    char time[9];
    char date[9];
    char weekday[4];
    char ticker[CLOCK_TICKER_WIDTH + 1U];
    uint16_t battery_millivolts;
} ClockView;

typedef struct {
    uint32_t last_update_ms;
    uint32_t last_blink_ms;
    uint32_t last_scroll_ms;
    uint32_t battery_elapsed_ms;
    uint16_t battery_millivolts;
    size_t ticker_offset;
    bool separator_is_brace;
} ClockApplication;

void clock_application_init(ClockApplication *application, uint32_t now_ms);
void clock_application_update(ClockApplication *application,
                              const ClockDateTime *date_time,
                              uint32_t now_ms,
                              ClockView *view);

#endif
