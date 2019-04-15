#ifndef _I2C_H_
#define _I2C_H_

#include "platform.h"


uint8_t read_i2c(uint8_t slave_addr, uint8_t register_addr);

void read_multibyte_i2c(uint8_t slave_addr, uint8_t register_addr, uint8_t *data, uint8_t num_bytes);

void write_i2c(uint8_t slave_addr, uint8_t register_addr, uint8_t register_data);

void init_i2c(void);



#endif /* I2C_H_ */




