#include "rtc_adapter.h"

#include <stddef.h>

#include "rtc.h"

static bool is_leap_year(uint8_t year)
{
    /* The RTC stores years 00..99, all interpreted as 2000..2099. */
    return (year % 4U) == 0U;
}

static uint8_t days_in_month(uint8_t year, uint8_t month)
{
    static const uint8_t month_lengths[12] = {
        31U, 28U, 31U, 30U, 31U, 30U,
        31U, 31U, 30U, 31U, 30U, 31U
    };
    uint8_t days = month_lengths[month - 1U];

    if ((month == 2U) && is_leap_year(year)) {
        ++days;
    }
    return days;
}

static void step_value(uint8_t *value,
                       uint8_t minimum,
                       uint8_t maximum,
                       int8_t direction)
{
    if ((*value < minimum) || (*value > maximum)) {
        *value = minimum;
    } else if (direction > 0) {
        *value = (*value == maximum) ? minimum : (uint8_t)(*value + 1U);
    } else {
        *value = (*value == minimum) ? maximum : (uint8_t)(*value - 1U);
    }
}

static uint8_t weekday_for_date(uint8_t year, uint8_t month, uint8_t day)
{
    static const uint8_t month_offsets[12] = {
        0U, 3U, 2U, 5U, 0U, 3U, 5U, 1U, 4U, 6U, 2U, 4U
    };
    uint16_t full_year = (uint16_t)2000U + year;
    uint8_t weekday;

    if (month < 3U) {
        --full_year;
    }
    weekday = (uint8_t)((full_year + full_year / 4U - full_year / 100U +
                         full_year / 400U + month_offsets[month - 1U] + day) % 7U);
    return (weekday == 0U) ? 7U : weekday;
}

static bool rtc_set_time(const ClockDateTime *date_time)
{
    RTC_TimeTypeDef time = {0};

    time.Hours = date_time->hour;
    time.Minutes = date_time->minute;
    time.Seconds = date_time->second;
    time.TimeFormat = RTC_HOURFORMAT12_AM;
    time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    time.StoreOperation = RTC_STOREOPERATION_RESET;
    return HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN) == HAL_OK;
}

static bool rtc_set_date(const ClockDateTime *date_time)
{
    RTC_DateTypeDef date = {0};

    date.WeekDay = weekday_for_date(date_time->year, date_time->month,
                                    date_time->day);
    date.Date = date_time->day;
    date.Month = date_time->month;
    date.Year = date_time->year;
    return HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN) == HAL_OK;
}

bool stm32f0_rtc_read(ClockDateTime *date_time)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    /* Read time before date; HAL's date read releases the RTC shadow registers. */
    if ((date_time == NULL) ||
        (HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK) ||
        (HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)) {
        return false;
    }

    date_time->hour = time.Hours;
    date_time->minute = time.Minutes;
    date_time->second = time.Seconds;
    date_time->day = date.Date;
    date_time->month = date.Month;
    date_time->year = date.Year;
    date_time->weekday = date.WeekDay;
    return true;
}

bool stm32f0_rtc_adjust(ClockDateTime *date_time,
                        Stm32f0RtcSetting setting,
                        int8_t direction)
{
    uint8_t *value;
    uint8_t maximum;

    if ((date_time == NULL) || ((direction != -1) && (direction != 1))) {
        return false;
    }

    switch (setting) {
    case STM32F0_RTC_SETTING_HOUR:
        step_value(&date_time->hour, 0U, 23U, direction);
        return rtc_set_time(date_time);
    case STM32F0_RTC_SETTING_MINUTE:
        step_value(&date_time->minute, 0U, 59U, direction);
        return rtc_set_time(date_time);
    case STM32F0_RTC_SETTING_DAY:
        value = &date_time->day;
        if ((date_time->month < 1U) || (date_time->month > 12U)) {
            date_time->month = 1U;
        }
        if (date_time->year > 99U) {
            date_time->year = 0U;
        }
        maximum = days_in_month(date_time->year, date_time->month);
        break;
    case STM32F0_RTC_SETTING_MONTH:
        value = &date_time->month;
        maximum = 12U;
        break;
    case STM32F0_RTC_SETTING_YEAR:
        value = &date_time->year;
        maximum = 99U;
        break;
    default:
        return false;
    }

    if (date_time->year > 99U) {
        date_time->year = 0U;
    }
    if ((date_time->month < 1U) || (date_time->month > 12U)) {
        date_time->month = 1U;
    }
    if (date_time->day < 1U) {
        date_time->day = 1U;
    }

    step_value(value, 1U, maximum, direction);
    maximum = days_in_month(date_time->year, date_time->month);
    if (date_time->day > maximum) {
        date_time->day = maximum;
    }
    date_time->weekday = weekday_for_date(date_time->year,
                                          date_time->month,
                                          date_time->day);
    return rtc_set_date(date_time);
}
