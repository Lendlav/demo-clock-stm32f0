#include "clock_application.h"

#include <string.h>

#define BATTERY_MIN_MV 3218U
#define BATTERY_MAX_MV 4096U
#define BATTERY_STEP_MS 20U
#define BLINK_INTERVAL_MS 500U
#define TICKER_INTERVAL_MS 100U

static const char ticker_message[] =
    " This is the run string where you can write everything you want to write<<<";

static const char *const weekday_names[] = {
    "???", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

static void write_two_digits(char *destination, uint8_t value)
{
    value = (uint8_t)(value % 100U);
    destination[0] = (char)('0' + (value / 10U));
    destination[1] = (char)('0' + (value % 10U));
}

static void update_ticker(ClockApplication *application, ClockView *view)
{
    const size_t message_length = sizeof(ticker_message) - 1U;
    size_t index = application->ticker_offset;

    for (size_t column = 0U; column < CLOCK_TICKER_WIDTH; ++column) {
        view->ticker[column] = ticker_message[index];
        index = (index + 1U) % message_length;
    }
    view->ticker[CLOCK_TICKER_WIDTH] = '\0';
}

static void update_clock_view(ClockApplication *application,
                              const ClockDateTime *date_time,
                              ClockView *view)
{
    write_two_digits(&view->time[0], date_time->hour);
    view->time[2] = application->separator_is_brace ? '}' : '~';
    write_two_digits(&view->time[3], date_time->minute);
    view->time[5] = application->separator_is_brace ? '}' : '~';
    write_two_digits(&view->time[6], date_time->second);
    view->time[8] = '\0';

    write_two_digits(&view->date[0], date_time->day);
    view->date[2] = '-';
    write_two_digits(&view->date[3], date_time->month);
    view->date[5] = '-';
    write_two_digits(&view->date[6], date_time->year);
    view->date[8] = '\0';

    const size_t weekday_count = sizeof(weekday_names) / sizeof(weekday_names[0]);
    const char *weekday = date_time->weekday < weekday_count
                              ? weekday_names[date_time->weekday]
                              : weekday_names[0];
    memcpy(view->weekday, weekday, 3U);
    view->weekday[3] = '\0';

    view->battery_millivolts = application->battery_millivolts;
    update_ticker(application, view);
}

void clock_application_init(ClockApplication *application, uint32_t now_ms)
{
    memset(application, 0, sizeof(*application));
    application->last_update_ms = now_ms;
    application->last_blink_ms = now_ms;
    application->last_scroll_ms = now_ms;
    application->battery_millivolts = BATTERY_MIN_MV;
    application->separator_is_brace = true;
}

void clock_application_update(ClockApplication *application,
                              const ClockDateTime *date_time,
                              uint32_t now_ms,
                              ClockView *view)
{
    const uint32_t elapsed_ms = now_ms - application->last_update_ms;
    application->last_update_ms = now_ms;
    uint32_t battery_steps = elapsed_ms / BATTERY_STEP_MS;
    application->battery_elapsed_ms += elapsed_ms % BATTERY_STEP_MS;
    if (application->battery_elapsed_ms >= BATTERY_STEP_MS) {
        application->battery_elapsed_ms -= BATTERY_STEP_MS;
        ++battery_steps;
    }

    const uint32_t battery_range = BATTERY_MAX_MV - BATTERY_MIN_MV + 1U;
    const uint32_t battery_offset = application->battery_millivolts - BATTERY_MIN_MV;
    application->battery_millivolts = (uint16_t)(BATTERY_MIN_MV +
        (battery_offset + (battery_steps % battery_range)) % battery_range);

    const uint32_t blink_steps =
        (uint32_t)(now_ms - application->last_blink_ms) / BLINK_INTERVAL_MS;
    if ((blink_steps % 2U) != 0U) {
        application->separator_is_brace = !application->separator_is_brace;
    }
    application->last_blink_ms += blink_steps * BLINK_INTERVAL_MS;

    const size_t message_length = sizeof(ticker_message) - 1U;
    const uint32_t scroll_steps =
        (uint32_t)(now_ms - application->last_scroll_ms) / TICKER_INTERVAL_MS;
    application->last_scroll_ms += scroll_steps * TICKER_INTERVAL_MS;
    application->ticker_offset = (application->ticker_offset +
        (scroll_steps % message_length)) % message_length;

    update_clock_view(application, date_time, view);
}
