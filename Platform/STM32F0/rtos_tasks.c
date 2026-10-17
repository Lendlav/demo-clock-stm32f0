#include "rtos_tasks.h"

#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "clock_application.h"
#include "clock_display.h"
#include "board.h"
#include "gpio.h"
#include "rtc_adapter.h"
#include "temperature_sensor.h"

#define CLOCK_TASK_PERIOD_MS 50U
#define SENSOR_SAMPLE_INTERVAL_MS 5000U
#define CUBE_MIN_X_Q8 (7 * 256)
#define CUBE_MAX_X_Q8 (120 * 256)
#define CUBE_MIN_Y_Q8 (7 * 256)
#define CUBE_MAX_Y_Q8 (24 * 256)

typedef enum {
    CLOCK_MENU_NONE = 0,
    CLOCK_MENU_HOUR,
    CLOCK_MENU_MINUTE,
    CLOCK_MENU_DAY,
    CLOCK_MENU_MONTH,
    CLOCK_MENU_YEAR,
    CLOCK_MENU_BRIGHTNESS,
    CLOCK_MENU_SUPPLY,
    CLOCK_MENU_ANIMATION
} ClockMenuSetting;

static QueueHandle_t view_queue;
static ClockApplication application;
static ClockMenuSetting active_setting;
static uint8_t brightness_level;
static uint32_t animation_elapsed_ms;
static uint8_t cube_angle;
static int32_t cube_x_q8;
static int32_t cube_y_q8;
static int16_t cube_velocity_x_q8;
static int16_t cube_velocity_y_q8;

static void advance_cube_position(void)
{
    cube_x_q8 += cube_velocity_x_q8;
    cube_y_q8 += cube_velocity_y_q8;

    if (cube_x_q8 <= CUBE_MIN_X_Q8) {
        cube_x_q8 = CUBE_MIN_X_Q8;
        if (cube_velocity_x_q8 < 0) {
            cube_velocity_x_q8 = (int16_t)-cube_velocity_x_q8;
        }
    } else if (cube_x_q8 >= CUBE_MAX_X_Q8) {
        cube_x_q8 = CUBE_MAX_X_Q8;
        if (cube_velocity_x_q8 > 0) {
            cube_velocity_x_q8 = (int16_t)-cube_velocity_x_q8;
        }
    }

    if (cube_y_q8 <= CUBE_MIN_Y_Q8) {
        cube_y_q8 = CUBE_MIN_Y_Q8;
        if (cube_velocity_y_q8 < 0) {
            cube_velocity_y_q8 = (int16_t)-cube_velocity_y_q8;
        }
    } else if (cube_y_q8 >= CUBE_MAX_Y_Q8) {
        cube_y_q8 = CUBE_MAX_Y_Q8;
        if (cube_velocity_y_q8 > 0) {
            cube_velocity_y_q8 = (int16_t)-cube_velocity_y_q8;
        }
    }
}

static void advance_setting(void)
{
    if (active_setting == CLOCK_MENU_ANIMATION) {
        active_setting = CLOCK_MENU_NONE;
    } else {
        active_setting = (ClockMenuSetting)(active_setting + 1);
    }
}

static void show_active_setting(ClockView *view)
{
    static const char *const labels[] = {
        "???", "HR ", "MIN", "DAY", "MON", "YR "
    };

    view->status[0] = '\0';
    if ((active_setting >= CLOCK_MENU_HOUR) &&
        (active_setting <= CLOCK_MENU_YEAR)) {
        memcpy(view->weekday, labels[active_setting], 3U);
        view->weekday[3] = '\0';
    } else if (active_setting == CLOCK_MENU_BRIGHTNESS) {
        const uint8_t percent = (uint8_t)(
            (brightness_level * 100U + CLOCK_DISPLAY_BRIGHTNESS_MAX_LEVEL / 2U) /
            CLOCK_DISPLAY_BRIGHTNESS_MAX_LEVEL);
        view->status[0] = 'B';
        view->status[1] = 'R';
        view->status[2] = 'I';
        view->status[3] = ' ';
        view->status[4] = (percent < 100U) ? ' ' : '1';
        view->status[5] = (char)('0' + ((percent / 10U) % 10U));
        view->status[6] = (char)('0' + (percent % 10U));
        view->status[7] = '%';
        view->status[8] = '\0';
    } else if (active_setting == CLOCK_MENU_SUPPLY) {
        view->status[0] = 'V';
        view->status[1] = 'D';
        view->status[2] = 'D';
        view->status[3] = ' ';
        if (view->supply_valid) {
            view->status[4] = (char)('0' + (view->supply_millivolts / 1000U));
            view->status[5] = '.';
            view->status[6] = (char)('0' +
                ((view->supply_millivolts % 1000U) / 100U));
            view->status[7] = (char)('0' +
                ((view->supply_millivolts % 100U) / 10U));
            view->status[8] = '\0';
        } else {
            view->status[4] = '-';
            view->status[5] = '-';
            view->status[6] = '-';
            view->status[7] = '\0';
        }
    }
}

/* Sample the RTC and publish display-ready data at a fixed cadence. */
static void clock_update_task(void *argument)
{
    ClockDateTime date_time;
    ClockView view;
    TickType_t wake_time = xTaskGetTickCount();
    TickType_t previous_tick = wake_time;
    int16_t temperature_celsius = 0;
    uint16_t supply_millivolts = 0U;
    bool sensors_valid = false;
    uint32_t sensor_elapsed_ms = SENSOR_SAMPLE_INTERVAL_MS;

    (void)argument;

    for (;;) {
        const TickType_t now = xTaskGetTickCount();
        const TickType_t elapsed_ticks = now - previous_tick;
        const uint32_t elapsed_ms = (uint32_t)(
            ((uint64_t)elapsed_ticks * 1000U) / configTICK_RATE_HZ);
        previous_tick = now;

        sensor_elapsed_ms += elapsed_ms;
        if (sensor_elapsed_ms >= SENSOR_SAMPLE_INTERVAL_MS) {
            int16_t measured_temperature;
            uint16_t measured_supply;
            if (stm32f0_internal_sensors_read(&measured_temperature,
                                               &measured_supply)) {
                temperature_celsius = measured_temperature;
                supply_millivolts = measured_supply;
                sensors_valid = true;
            }
            sensor_elapsed_ms = 0U;
        }

        const uint8_t button_events = stm32f0_buttons_poll();
        if ((button_events & STM32F0_BUTTON_MODE) != 0U) {
            advance_setting();
        }

        if (active_setting == CLOCK_MENU_ANIMATION) {
            animation_elapsed_ms += elapsed_ms;
            while (animation_elapsed_ms >= CLOCK_TASK_PERIOD_MS) {
                animation_elapsed_ms -= CLOCK_TASK_PERIOD_MS;
                cube_angle = (uint8_t)((cube_angle + 1U) & 63U);
                advance_cube_position();
            }
        }

        if (stm32f0_rtc_read(&date_time)) {
            int8_t direction = 0;
            if ((button_events & STM32F0_BUTTON_UP) != 0U) {
                direction = 1;
            } else if ((button_events & STM32F0_BUTTON_DOWN) != 0U) {
                direction = -1;
            }

            if ((direction != 0) &&
                (active_setting >= CLOCK_MENU_HOUR) &&
                (active_setting <= CLOCK_MENU_YEAR)) {
                (void)stm32f0_rtc_adjust(&date_time,
                                         (Stm32f0RtcSetting)active_setting,
                                         direction);
            } else if ((direction > 0) &&
                       (active_setting == CLOCK_MENU_BRIGHTNESS) &&
                       (brightness_level < CLOCK_DISPLAY_BRIGHTNESS_MAX_LEVEL)) {
                ++brightness_level;
            } else if ((direction < 0) &&
                       (active_setting == CLOCK_MENU_BRIGHTNESS) &&
                       (brightness_level > CLOCK_DISPLAY_BRIGHTNESS_MIN_LEVEL)) {
                --brightness_level;
            }

            clock_application_update(&application,
                                     &date_time,
                                     elapsed_ms,
                                     active_setting == CLOCK_MENU_ANIMATION,
                                     temperature_celsius,
                                     sensors_valid,
                                     &view);
            view.supply_millivolts = supply_millivolts;
            view.supply_valid = sensors_valid;
            view.brightness_level = brightness_level;
            view.cube_angle = cube_angle;
            view.cube_x = (uint8_t)((cube_x_q8 + 128) / 256);
            view.cube_y = (uint8_t)((cube_y_q8 + 128) / 256);
            show_active_setting(&view);
            /* Keep only the newest animation frame if the display is still busy. */
            (void)xQueueOverwrite(view_queue, &view);
        }

        vTaskDelayUntil(&wake_time, pdMS_TO_TICKS(CLOCK_TASK_PERIOD_MS));
    }
}

static void display_task(void *argument)
{
    ClockView view;

    (void)argument;

    /* Keep blocking OLED transfers out of the clock-update task. */
    for (;;) {
        if (xQueueReceive(view_queue, &view, portMAX_DELAY) == pdPASS) {
            clock_display_render(&view);
        }
    }
}

void stm32f0_rtos_start(void)
{
    view_queue = xQueueCreate(1U, sizeof(ClockView));
    configASSERT(view_queue != NULL);
    clock_application_init(&application);
    active_setting = CLOCK_MENU_NONE;
    brightness_level = CLOCK_DISPLAY_BRIGHTNESS_DEFAULT_LEVEL;
    animation_elapsed_ms = 0U;
    cube_angle = 0U;
    cube_x_q8 = 28 * 256;
    cube_y_q8 = 10 * 256;
    cube_velocity_x_q8 = -192;
    cube_velocity_y_q8 = 128;

    /* RTC sampling has higher priority; display rendering can block on I2C. */
    configASSERT(xTaskCreate(clock_update_task,
                             "clock-update",
                             96U,
                             NULL,
                             2U,
                             NULL) == pdPASS);
    configASSERT(xTaskCreate(display_task,
                             "display",
                             128U,
                             NULL,
                             1U,
                             NULL) == pdPASS);

    vTaskStartScheduler();
    Error_Handler();
}

void vApplicationMallocFailedHook(void)
{
    Error_Handler();
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    Error_Handler();
}
