/* Cortex-M0 startup for STM32F030F4/F030x6. */

.syntax unified
.cpu cortex-m0
.thumb

.global g_pfnVectors
.global Reset_Handler
.extern SystemInit
.extern main

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word 0
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* STM32F030x6 external interrupt vectors (IRQ numbers 0 through 27). */
    .word WWDG_IRQHandler
    .word 0
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_1_IRQHandler
    .word EXTI2_3_IRQHandler
    .word EXTI4_15_IRQHandler
    .word 0
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_3_IRQHandler
    .word DMA1_Channel4_5_IRQHandler
    .word ADC1_IRQHandler
    .word TIM1_BRK_UP_TRG_COM_IRQHandler
    .word TIM1_CC_IRQHandler
    .word 0
    .word TIM3_IRQHandler
    .word 0
    .word 0
    .word TIM14_IRQHandler
    .word 0
    .word TIM16_IRQHandler
    .word TIM17_IRQHandler
    .word I2C1_IRQHandler
    .word 0
    .word SPI1_IRQHandler
    .word 0
    .word USART1_IRQHandler
.size g_pfnVectors, . - g_pfnVectors

.section .text.Reset_Handler,"ax",%progbits
.type Reset_Handler, %function
.thumb_func
Reset_Handler:
    bl SystemInit

    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0]
    str r3, [r1]
    adds r0, #4
    adds r1, #4
    b 1b

2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:
    cmp r1, r2
    bcs 4f
    str r3, [r1]
    adds r1, #4
    b 3b

4:
    bl main
5:
    b 5b
.size Reset_Handler, . - Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.type Default_Handler, %function
.thumb_func
Default_Handler:
    b Default_Handler
.size Default_Handler, . - Default_Handler

.macro weak_default handler
    .weak \handler
    .thumb_set \handler, Default_Handler
.endm

weak_default NMI_Handler
weak_default HardFault_Handler
weak_default SVC_Handler
weak_default PendSV_Handler
weak_default SysTick_Handler
weak_default WWDG_IRQHandler
weak_default RTC_IRQHandler
weak_default FLASH_IRQHandler
weak_default RCC_IRQHandler
weak_default EXTI0_1_IRQHandler
weak_default EXTI2_3_IRQHandler
weak_default EXTI4_15_IRQHandler
weak_default DMA1_Channel1_IRQHandler
weak_default DMA1_Channel2_3_IRQHandler
weak_default DMA1_Channel4_5_IRQHandler
weak_default ADC1_IRQHandler
weak_default TIM1_BRK_UP_TRG_COM_IRQHandler
weak_default TIM1_CC_IRQHandler
weak_default TIM3_IRQHandler
weak_default TIM14_IRQHandler
weak_default TIM16_IRQHandler
weak_default TIM17_IRQHandler
weak_default I2C1_IRQHandler
weak_default SPI1_IRQHandler
weak_default USART1_IRQHandler
