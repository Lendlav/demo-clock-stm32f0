#include "power.h"

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "board.h"
#include "rtc.h"
#include "stm32f0xx_hal.h"
#include "task.h"

#define SLEEP_GUARD_TICKS 3U
#define CLOCK_READY_TIMEOUT 100000U

static volatile uint32_t alarm_event_count;

static bool rtc_read_once(RTC_TimeTypeDef *time)
{
    RTC_DateTypeDef date;

    /* Reading the date releases the calendar shadow lock after GetTime. */
    return (HAL_RTC_GetTime(&hrtc, time, RTC_FORMAT_BIN) == HAL_OK) &&
           (HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) == HAL_OK);
}

static bool rtc_read_snapshot(RTC_TimeTypeDef *time)
{
    RTC_TimeTypeDef previous;
    RTC_TimeTypeDef current;

    if (!rtc_read_once(&previous)) {
        return false;
    }

    /* Retry a boundary-crossing SSR/TR read so the pair describes one instant. */
    for (uint8_t attempt = 0U; attempt < 3U; ++attempt) {
        int32_t second_delta;
        int64_t rtc_counts;
        uint32_t rtc_period;

        if (!rtc_read_once(&current)) {
            return false;
        }

        rtc_period = current.SecondFraction + 1U;
        second_delta = (int32_t)current.Seconds - (int32_t)previous.Seconds;
        if (second_delta < 0) {
            second_delta += 60;
        }

        rtc_counts = ((int64_t)second_delta * rtc_period) +
                     (int64_t)previous.SubSeconds -
                     (int64_t)current.SubSeconds;
        if ((rtc_counts >= 0) &&
            (rtc_counts <= (int64_t)rtc_period)) {
            *time = current;
            return true;
        }
        previous = current;
    }

    return false;
}

static uint32_t ticks_until_alarm(uint32_t current_subseconds,
                                  uint32_t alarm_subseconds,
                                  uint32_t rtc_period)
{
    uint32_t rtc_counts =
        (current_subseconds + rtc_period - alarm_subseconds) % rtc_period;
    uint64_t scaled_ticks;

    /* Equal values mean the next match is one complete RTC second away. */
    if (rtc_counts == 0U) {
        rtc_counts = rtc_period;
    }

    scaled_ticks = ((uint64_t)rtc_counts * configTICK_RATE_HZ +
                    rtc_period - 1U) / rtc_period;
    return (uint32_t)scaled_ticks;
}

static uint32_t elapsed_rtc_ticks(const RTC_TimeTypeDef *before,
                                  const RTC_TimeTypeDef *after,
                                  uint32_t rtc_period)
{
    int32_t second_delta = (int32_t)after->Seconds - (int32_t)before->Seconds;
    int64_t rtc_counts;

    if (second_delta < 0) {
        second_delta += 60;
    }

    rtc_counts = ((int64_t)second_delta * rtc_period) +
                 (int64_t)before->SubSeconds -
                 (int64_t)after->SubSeconds;
    if (rtc_counts <= 0) {
        return 0U;
    }

    return (uint32_t)(((uint64_t)rtc_counts * configTICK_RATE_HZ) /
                      rtc_period);
}

static bool wait_for_register(volatile uint32_t *reg,
                              uint32_t mask,
                              uint32_t expected_value)
{
    uint32_t timeout = CLOCK_READY_TIMEOUT;

    while (((*reg) & mask) != expected_value) {
        if (timeout == 0U) {
            return false;
        }
        --timeout;
    }

    return true;
}

static void restore_system_clock(void)
{
    RCC->CR |= RCC_CR_HSION;
    if (!wait_for_register(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY)) {
        Error_Handler();
    }

    /* STOP restarts on HSI; the original PLL factors remain programmed. */
    RCC->CR |= RCC_CR_PLLON;
    if (!wait_for_register(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY)) {
        Error_Handler();
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    if (!wait_for_register(&RCC->CFGR,
                           RCC_CFGR_SWS,
                           RCC_CFGR_SWS_PLL)) {
        Error_Handler();
    }

    SystemCoreClockUpdate();
}

void stm32f0_power_init(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_NVIC_SetPriority(RTC_IRQn,
                         configLIBRARY_LOWEST_INTERRUPT_PRIORITY,
                         0U);
    HAL_NVIC_EnableIRQ(RTC_IRQn);
}

void stm32f0_power_tickless_sleep(uint32_t expected_idle_ticks)
{
    RTC_TimeTypeDef before;
    RTC_TimeTypeDef after;
    RTC_AlarmTypeDef alarm = {0};
    uint32_t rtc_period;
    uint32_t alarm_subseconds;
    uint32_t sleep_ticks;
    uint32_t active_ticks;
    uint32_t remaining_ticks;
    uint32_t measured_ticks;
    uint32_t alarm_distance_ticks;
    uint32_t alarm_events_after_setup;
    uint32_t systick_control;
    uint32_t primask;
    uint32_t started_at_ms;
    uint64_t rtc_counts;

    if (expected_idle_ticks <= SLEEP_GUARD_TICKS + 1U) {
        return;
    }

    started_at_ms = HAL_GetTick();
    if (!rtc_read_snapshot(&before) ||
        (before.SubSeconds > before.SecondFraction)) {
        return;
    }

    rtc_period = before.SecondFraction + 1U;
    sleep_ticks = expected_idle_ticks - SLEEP_GUARD_TICKS;
    if (sleep_ticks >= configTICK_RATE_HZ) {
        sleep_ticks = configTICK_RATE_HZ - SLEEP_GUARD_TICKS;
    }

    /* Match one RTC subsecond value, with calendar fields ignored. */
    rtc_counts = ((uint64_t)sleep_ticks * rtc_period +
                  configTICK_RATE_HZ - 1U) / configTICK_RATE_HZ;
    if ((rtc_counts == 0U) || (rtc_counts >= rtc_period)) {
        return;
    }

    alarm_subseconds = (before.SubSeconds + rtc_period -
                        (uint32_t)rtc_counts) % rtc_period;
    alarm.Alarm = RTC_ALARM_A;
    alarm.AlarmTime.SubSeconds = alarm_subseconds;
    alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    alarm.AlarmMask = RTC_ALARMMASK_ALL;
    alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    alarm.AlarmDateWeekDay = 1U;

    if (HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN) != HAL_OK) {
        return;
    }
    alarm_events_after_setup = alarm_event_count;

    /* The scheduler is already suspended by FreeRTOS's tickless-idle path. */
    primask = __get_PRIMASK();
    __disable_irq();

    if ((eTaskConfirmSleepModeStatus() == eAbortSleep) ||
        (alarm_event_count != alarm_events_after_setup) ||
        ((RTC->ISR & RTC_ISR_ALRAF) != 0U) ||
        ((EXTI->PR & RTC_EXTI_LINE_ALARM_EVENT) != 0U) ||
        ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0U) ||
        !rtc_read_snapshot(&before)) {
        __set_PRIMASK(primask);
        return;
    }

    active_ticks = (uint32_t)(((uint64_t)(HAL_GetTick() - started_at_ms) *
                               configTICK_RATE_HZ) / 1000U);
    if (active_ticks + SLEEP_GUARD_TICKS >= expected_idle_ticks) {
        __set_PRIMASK(primask);
        return;
    }

    remaining_ticks = expected_idle_ticks - active_ticks - SLEEP_GUARD_TICKS;
    alarm_distance_ticks = ticks_until_alarm(before.SubSeconds,
                                             alarm_subseconds,
                                             rtc_period);
    if ((alarm_distance_ticks == 0U) ||
        (alarm_distance_ticks > remaining_ticks) ||
        ((RTC->ISR & RTC_ISR_ALRAF) != 0U) ||
        ((EXTI->PR & RTC_EXTI_LINE_ALARM_EVENT) != 0U)) {
        __set_PRIMASK(primask);
        return;
    }

    systick_control = SysTick->CTRL;
    if ((systick_control & SysTick_CTRL_ENABLE_Msk) == 0U) {
        __set_PRIMASK(primask);
        return;
    }

    /* Stop the 1 kHz scheduler source only after confirming no tick is pending. */
    SysTick->CTRL = systick_control & ~SysTick_CTRL_ENABLE_Msk;
    if ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0U) {
        SysTick->CTRL = systick_control;
        __set_PRIMASK(primask);
        return;
    }

    __DSB();
    __ISB();
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    if (rtc_read_snapshot(&after)) {
        measured_ticks = elapsed_rtc_ticks(&before, &after, rtc_period);
    } else {
        measured_ticks = 0U;
    }

    restore_system_clock();

    /* Restart SysTick with a full millisecond interval at the restored HCLK. */
    SysTick->LOAD = (SystemCoreClock / configTICK_RATE_HZ) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = systick_control;

    if (measured_ticks >= remaining_ticks) {
        measured_ticks = remaining_ticks;
    }
    if (measured_ticks != 0U) {
        vTaskStepTick((TickType_t)measured_ticks);
    }

    __set_PRIMASK(primask);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *rtc)
{
    if ((rtc != NULL) && (rtc->Instance == RTC)) {
        ++alarm_event_count;
    }
}
