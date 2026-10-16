#include "temperature_sensor.h"

#include "stm32f0xx_hal.h"

#define ADC_POLL_LIMIT          1000000U
#define ADC_AVERAGE_COUNT       4U
#define ADC_MAX_VALUE           4095U
#define TS_CAL1_ADDRESS         0x1FFFF7B8U
#define VREFINT_CAL_ADDRESS     0x1FFFF7BAU
#define TS_CALIBRATION_TEMP_C   30

static bool adc_initialized;

static bool wait_for_bit(volatile uint32_t *reg,
                         uint32_t mask,
                         bool expected_set)
{
    uint32_t remaining = ADC_POLL_LIMIT;

    do {
        const bool bit_is_set = ((*reg & mask) != 0U);
        if (bit_is_set == expected_set) {
            return true;
        }
        --remaining;
    } while (remaining != 0U);

    return false;
}

bool stm32f0_internal_sensors_init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* PCLK/4 is 8 MHz at the current 32 MHz system clock. */
    ADC1->CR = 0U;
    ADC1->CFGR1 = 0U;
    ADC1->CFGR2 = ADC_CFGR2_CKMODE_1;
    ADC1->SMPR = ADC_SMPR_SMP;
    ADC1->CHSELR = 0U;
    ADC->CCR &= ~(ADC_CCR_TSEN | ADC_CCR_VREFEN);

    /* The ADC must be disabled while its offset calibration runs. */
    ADC1->CR |= ADC_CR_ADCAL;
    adc_initialized = wait_for_bit(&ADC1->CR, ADC_CR_ADCAL, false);
    __HAL_RCC_ADC1_CLK_DISABLE();

    return adc_initialized;
}

static void adc_shutdown(void)
{
    if ((ADC1->CR & ADC_CR_ADSTART) != 0U) {
        ADC1->CR |= ADC_CR_ADSTP;
        (void)wait_for_bit(&ADC1->CR, ADC_CR_ADSTART, false);
    }

    if ((ADC1->CR & ADC_CR_ADEN) != 0U) {
        ADC1->CR |= ADC_CR_ADDIS;
        (void)wait_for_bit(&ADC1->CR, ADC_CR_ADEN, false);
    }

    ADC1->CHSELR = 0U;
    ADC->CCR &= ~(ADC_CCR_TSEN | ADC_CCR_VREFEN);
    __HAL_RCC_ADC1_CLK_DISABLE();
}

static bool read_internal_channels(uint16_t *temperature_raw,
                                   uint16_t *vref_raw)
{
    uint32_t temperature_sum = 0U;
    uint32_t vref_sum = 0U;
    bool success = false;

    if (!adc_initialized) {
        return false;
    }

    __HAL_RCC_ADC1_CLK_ENABLE();
    ADC->CCR |= ADC_CCR_TSEN | ADC_CCR_VREFEN;
    ADC1->CHSELR = ADC_CHSELR_CHSEL16 | ADC_CHSELR_CHSEL17;

    /* Allow the temperature sensor and voltage reference to settle. */
    HAL_Delay(1U);

    ADC1->ISR = ADC_ISR_ADRDY | ADC_ISR_EOC | ADC_ISR_EOS;
    ADC1->CR |= ADC_CR_ADEN;
    if (!wait_for_bit(&ADC1->ISR, ADC_ISR_ADRDY, true)) {
        goto cleanup;
    }
    ADC1->ISR = ADC_ISR_ADRDY;

    for (uint8_t sample = 0U; sample < ADC_AVERAGE_COUNT; ++sample) {
        /* Clear end-of-sequence before starting the next two-channel scan. */
        ADC1->ISR = ADC_ISR_EOS;
        ADC1->CR |= ADC_CR_ADSTART;

        if (!wait_for_bit(&ADC1->ISR, ADC_ISR_EOC, true)) {
            goto cleanup;
        }
        temperature_sum += ADC1->DR & ADC_MAX_VALUE;

        if (!wait_for_bit(&ADC1->ISR, ADC_ISR_EOC, true)) {
            goto cleanup;
        }
        vref_sum += ADC1->DR & ADC_MAX_VALUE;
    }

    *temperature_raw = (uint16_t)((temperature_sum + ADC_AVERAGE_COUNT / 2U) /
                                  ADC_AVERAGE_COUNT);
    *vref_raw = (uint16_t)((vref_sum + ADC_AVERAGE_COUNT / 2U) /
                           ADC_AVERAGE_COUNT);
    success = true;

cleanup:
    adc_shutdown();
    return success;
}

bool stm32f0_internal_sensors_read(int16_t *temperature_celsius,
                                   uint16_t *supply_millivolts)
{
    uint16_t temperature_raw = 0U;
    uint16_t vref_raw = 0U;

    if ((temperature_celsius == NULL) || (supply_millivolts == NULL)) {
        return false;
    }

    const uint16_t ts_cal = *(const uint16_t *)TS_CAL1_ADDRESS;
    const uint16_t vref_cal = *(const uint16_t *)VREFINT_CAL_ADDRESS;

    if ((ts_cal == 0U) || (ts_cal == 0xFFFFU) ||
        (vref_cal == 0U) || (vref_cal == 0xFFFFU) ||
        !read_internal_channels(&temperature_raw, &vref_raw) ||
        (vref_raw == 0U)) {
        return false;
    }

    /* Normalize the sample to 3.3 V using the part-specific VREF calibration. */
    const uint32_t temperature_at_3v3 =
        ((uint32_t)temperature_raw * vref_cal + vref_raw / 2U) / vref_raw;
    const int32_t delta_raw = (int32_t)ts_cal - (int32_t)temperature_at_3v3;

    /* 1 ADC count is about 0.1874 C at 3.3 V and a 4.3 mV/C average slope. */
    const int32_t temperature_centi_c =
        TS_CALIBRATION_TEMP_C * 100 + (delta_raw * 1874) / 100;
    const int32_t rounded_celsius = (temperature_centi_c >= 0)
                                        ? (temperature_centi_c + 50) / 100
                                        : (temperature_centi_c - 50) / 100;

    const uint32_t measured_supply_millivolts =
        (3300UL * vref_cal + vref_raw / 2U) / vref_raw;

    if ((rounded_celsius < -40) || (rounded_celsius > 125) ||
        (measured_supply_millivolts > UINT16_MAX)) {
        return false;
    }

    *temperature_celsius = (int16_t)rounded_celsius;
    *supply_millivolts = (uint16_t)measured_supply_millivolts;
    return true;
}
