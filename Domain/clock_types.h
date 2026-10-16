#ifndef CLOCK_TYPES_H
#define CLOCK_TYPES_H

#include <stdint.h>

/* Hardware-independent calendar snapshot; year is the RTC two-digit year. */
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    /* RTC weekday numbering: 1 = Monday through 7 = Sunday. */
    uint8_t weekday;
} ClockDateTime;

#endif
