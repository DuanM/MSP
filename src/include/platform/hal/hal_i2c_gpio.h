 /**
 * provides an abstraction for hardware.
 *
 * @file hal_i2c_gpio.h
 * @author sj
 *
 * @addtogroup HAL_I2C_GPIO HAL miscellaneous
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_I2C_GPIO_H
#define __HAL_I2C_GPIO_H

/**
 * 	use gpio to simulate i2c bus
 */

#define PS             0
#define PZ             1

#define PZ_SHDN_CFG    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN7)
#define PZ_SHDN_HIGH   GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN7)
#define PZ_SHDN_LOW    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN7)
#define PZ_SCL_CFG     GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN1)
#define PZ_SCL_HIGH    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN1)
#define PZ_SCL_LOW     GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1)
#define PZ_SDA_INPUT   GPIO_setAsInputPin(GPIO_PORT_P8, GPIO_PIN0)
#define PZ_SDA_IPV     GPIO_getInputPinValue(GPIO_PORT_P8, GPIO_PIN0)
#define PZ_SDA_OUTPUT  GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0)
#define PZ_SDA_HIGH    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0)
#define PZ_SDA_LOW     GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0)

#define PS_RESET_CFG   GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN4)
#define PS_RESET_HIGH  GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN4)
#define PS_RESET_LOW   GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN4)
#define PS_SCL_CFG     GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN6)
#define PS_SCL_HIGH    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN6)
#define PS_SCL_LOW     GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN6)
#define PS_SDA_INPUT   GPIO_setAsInputPin(GPIO_PORT_P7, GPIO_PIN5)
#define PS_SDA_IPV     GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN5)
#define PS_SDA_OUTPUT  GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN5)
#define PS_SDA_HIGH    GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN5)
#define PS_SDA_LOW     GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN5)


void hal_i2c_gpio_init(void);

void hal_i2c_start(bool flag);

void hal_i2c_stop(bool flag);

bool hal_i2c_check_ack(bool flag);

void hal_i2c_send_byte(uint8_t buf, bool flag);

void hal_i2c_adjust_gain_level(uint8_t address, uint8_t instruction, uint8_t value, bool flag);

void hal_i2c_delay_170ns(uint8_t times);

#endif
