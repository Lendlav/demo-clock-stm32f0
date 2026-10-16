#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

#include "stm32f0xx.h"

extern uint32_t SystemCoreClock;

#define configUSE_PREEMPTION                       1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    0
#define configUSE_IDLE_HOOK                        0
#define configUSE_TICK_HOOK                        0
#define configUSE_TICKLESS_IDLE                    1
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP      2U
#define configSUPPORT_STATIC_ALLOCATION            0
#define configSUPPORT_DYNAMIC_ALLOCATION           1
#define configCPU_CLOCK_HZ                         (SystemCoreClock)
#define configTICK_RATE_HZ                         ((TickType_t)1000U)
#define configMAX_PRIORITIES                       4U
#define configMINIMAL_STACK_SIZE                   ((uint16_t)96U)
#define configTOTAL_HEAP_SIZE                      ((size_t)1792U)
#define configMAX_TASK_NAME_LEN                    16U
#define configUSE_16_BIT_TICKS                     0
#define configUSE_MUTEXES                          0
#define configUSE_RECURSIVE_MUTEXES                0
#define configUSE_COUNTING_SEMAPHORES              0
#define configQUEUE_REGISTRY_SIZE                  0U
#define configUSE_CO_ROUTINES                      0
#define configMAX_CO_ROUTINE_PRIORITIES            1U
#define configUSE_TIMERS                           0
#define configUSE_TRACE_FACILITY                   0
#define configUSE_STATS_FORMATTING_FUNCTIONS       0
#define configUSE_MALLOC_FAILED_HOOK               1
#define configCHECK_FOR_STACK_OVERFLOW             2

#define INCLUDE_vTaskPrioritySet                  0
#define INCLUDE_uxTaskPriorityGet                 0
#define INCLUDE_vTaskDelete                       0
#define INCLUDE_vTaskCleanUpResources             0
#define INCLUDE_vTaskSuspend                      1
#define INCLUDE_vTaskDelayUntil                   1
#define INCLUDE_vTaskDelay                        1
#define INCLUDE_xTaskGetSchedulerState            1

#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS                            __NVIC_PRIO_BITS
#else
#define configPRIO_BITS                            2U
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   ((1U << configPRIO_BITS) - 1U)
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY \
    configLIBRARY_LOWEST_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))

#define configASSERT(condition)                  \
    do {                                         \
        if ((condition) == 0) {                  \
            taskDISABLE_INTERRUPTS();            \
            for (;;) {                           \
            }                                    \
        }                                        \
    } while (0)

#define vPortSVCHandler                            SVC_Handler
#define xPortPendSVHandler                         PendSV_Handler

void stm32f0_power_tickless_sleep(uint32_t expected_idle_ticks);
#define portSUPPRESS_TICKS_AND_SLEEP(expected_idle_ticks) \
    stm32f0_power_tickless_sleep((uint32_t)(expected_idle_ticks))

#endif /* FREERTOS_CONFIG_H */
