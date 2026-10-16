#include "clock_application.h"

#include <string.h>

#define BATTERY_MIN_MV 3218U
#define BATTERY_MAX_MV 4096U
#define BATTERY_STEP_MS 20U
#define TICKER_INTERVAL_MS 50U
#define SEPARATOR_INTERVAL_MS 500U

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

static void update_ticker(const ClockApplication *application, ClockView *view)
{
    const size_t message_length = sizeof(ticker_message) - 1U;
    size_t index = application->ticker_offset;

    for (size_t column = 0U; column < CLOCK_TICKER_WIDTH; ++column) {
        view->ticker[column] = ticker_message[index];
        index = (index + 1U) % message_length;
    }

    view->ticker[CLOCK_TICKER_WIDTH] = '\0';
    view->ticker_pixel_offset = application->ticker_pixel_offset;
}

void clock_application_init(ClockApplication *application)
{
    memset(application, 0, sizeof(*application));
    application->battery_millivolts = BATTERY_MIN_MV;
    application->separator_is_brace = true;
}

void clock_application_update(ClockApplication *application,
                              const ClockDateTime *date_time,
                              uint32_t elapsed_ms,
                              int16_t temperature_celsius,
                              bool temperature_valid,
                              ClockView *view)
{
    const uint32_t message_length = (uint32_t)(sizeof(ticker_message) - 1U);
    const uint64_t battery_elapsed =
        (uint64_t)application->battery_elapsed_ms + elapsed_ms;
    const uint32_t battery_steps = (uint32_t)(battery_elapsed / BATTERY_STEP_MS);
    const uint32_t battery_range = BATTERY_MAX_MV - BATTERY_MIN_MV + 1U;
    const uint32_t battery_offset =
        application->battery_millivolts - BATTERY_MIN_MV;
    application->battery_elapsed_ms = (uint32_t)(battery_elapsed % BATTERY_STEP_MS);
    application->battery_millivolts = (uint16_t)(BATTERY_MIN_MV +
        ((battery_offset + battery_steps) % battery_range));

    const uint64_t ticker_elapsed =
        (uint64_t)application->ticker_elapsed_ms + elapsed_ms;
    const uint32_t scroll_steps = (uint32_t)(ticker_elapsed / TICKER_INTERVAL_MS);
    application->ticker_elapsed_ms = (uint32_t)(ticker_elapsed % TICKER_INTERVAL_MS);

    const uint32_t pixel_position = application->ticker_pixel_offset + scroll_steps;
    application->ticker_pixel_offset = (uint8_t)(pixel_position % 6U);
    application->ticker_offset =
        (application->ticker_offset + (pixel_position / 6U)) % message_length;

    const uint64_t separator_elapsed =
        (uint64_t)application->separator_elapsed_ms + elapsed_ms;
    const uint32_t blink_steps =
        (uint32_t)(separator_elapsed / SEPARATOR_INTERVAL_MS);
    application->separator_elapsed_ms =
        (uint32_t)(separator_elapsed % SEPARATOR_INTERVAL_MS);
    if ((blink_steps & 1U) != 0U) {
        application->separator_is_brace = !application->separator_is_brace;
    }

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

    update_ticker(application, view);
    view->battery_millivolts = application->battery_millivolts;
    view->temperature_celsius = temperature_celsius;
    view->temperature_valid = temperature_valid;
}
