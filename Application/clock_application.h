#ifndef CLOCK_APPLICATION_H
#define CLOCK_APPLICATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "clock_types.h"

#define CLOCK_TICKER_WIDTH 23U

/* Display-ready data for one clock and ticker frame. */
typedef struct {
    char time[9];
    char date[9];
    char weekday[4];
    char ticker[CLOCK_TICKER_WIDTH + 1U];
    uint8_t ticker_pixel_offset;
    uint16_t battery_millivolts;
    int16_t temperature_celsius;
    bool temperature_valid;
    uint16_t supply_millivolts;
    bool supply_valid;
    uint8_t brightness_level;
    char status[9];
    uint8_t octahedron_angle;
    uint8_t octahedron_x;
    uint8_t octahedron_y;
    bool octahedron_mode;
    uint8_t cube_angle;
    uint8_t cube_x;
    uint8_t cube_y;
    uint8_t cross_angle;
    uint8_t cross_x;
    uint8_t cross_y;
} ClockView;

typedef struct {
    uint32_t ticker_elapsed_ms;
    uint32_t separator_elapsed_ms;
    uint32_t battery_elapsed_ms;
    size_t ticker_offset;
    uint8_t ticker_pixel_offset;
    uint16_t battery_millivolts;
    bool separator_is_brace;
} ClockApplication;

void clock_application_init(ClockApplication *application);

/* Advance animation timers and create display-ready data from the RTC snapshot. */
void clock_application_update(ClockApplication *application,
                              const ClockDateTime *date_time,
                              uint32_t elapsed_ms,
                              int16_t temperature_celsius,
                              bool temperature_valid,
                              ClockView *view);

#endif
