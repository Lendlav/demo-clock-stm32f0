#ifndef STM32F0_TEMPERATURE_SENSOR_H
#define STM32F0_TEMPERATURE_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

bool stm32f0_internal_sensors_init(void);
bool stm32f0_internal_sensors_read(int16_t *temperature_celsius,
                                   uint16_t *supply_millivolts);

#endif
