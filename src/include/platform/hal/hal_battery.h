/**
 * drive for battery.
 *
 * @file hal_battery.h
 * @author wanger
 *
 * @addtogroup HAL_BATTERY HAL Hardware Iinitialization
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_BATTERY_H
#define __HAL_BATTERY_H

/**
 * Initializes adc14
 */
void hal_battery_init(void);

/**
 * get values of adc14
 *
 * @return values of adc14(N_adc), The conversion formula for the the voltage is:
 * vol = ((N_adc)/4095)*2.45V*2
 */
uint16_t hal_get_vol_value(void);





#endif













