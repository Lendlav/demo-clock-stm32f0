#ifndef CLOCK_TYPES_H
#define CLOCK_TYPES_H

#include <stdint.h>

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t weekday;
} ClockDateTime;

#endif
