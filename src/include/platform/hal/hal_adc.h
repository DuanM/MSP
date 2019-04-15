 /**
 * provides an abstraction for hardware.
 *
 * @file hal_adc.h
 * @author sj
 *
 * @addtogroup HAL_ADC HAL miscellaneous
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_ADC_H
#define __HAL_ADC_H

void hal_adc14_init(void);

void hal_adc14_get(uint16_t *measure_value, uint8_t size);

uint16_t hal_adc14_manual_get(void);


#endif
