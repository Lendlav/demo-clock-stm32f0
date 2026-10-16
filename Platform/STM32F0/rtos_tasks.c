#include "rtos_tasks.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "clock_application.h"
#include "clock_display.h"
#include "board.h"
#include "rtc_adapter.h"
#include "stm32f0xx_hal.h"

#define CLOCK_TASK_PERIOD_MS 50U

static ClockApplication application;
static QueueHandle_t view_queue;

static void clock_update_task(void *argument)
{
    ClockDateTime date_time;
    ClockView view;
    TickType_t wake_time = xTaskGetTickCount();

    (void)argument;

    for (;;) {
        if (stm32f0_rtc_read(&date_time)) {
            clock_application_update(&application,
                                     &date_time,
                                     HAL_GetTick(),
                                     &view);
            (void)xQueueSendToBack(view_queue, &view, 0U);
        }

        vTaskDelayUntil(&wake_time, pdMS_TO_TICKS(CLOCK_TASK_PERIOD_MS));
    }
}

static void display_task(void *argument)
{
    ClockView view;

    (void)argument;

    for (;;) {
        if (xQueueReceive(view_queue, &view, portMAX_DELAY) == pdPASS) {
            clock_display_render(&view);
        }
    }
}

void stm32f0_rtos_start(void)
{
    clock_application_init(&application, HAL_GetTick());

    view_queue = xQueueCreate(1U, sizeof(ClockView));
    configASSERT(view_queue != NULL);

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
