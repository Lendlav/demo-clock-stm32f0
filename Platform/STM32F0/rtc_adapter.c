#include "rtc_adapter.h"

#include <stddef.h>

#include "rtc.h"

bool stm32f0_rtc_read(ClockDateTime *date_time)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

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
