#ifndef __HAL_MEM_H
#define __HAL_MEM_H

#define HAL_FLASH_BASE_ADDR		0x0003F000
#define HAL_FLASH_END_ADDR		0x0003FFFF
#define HAL_FLASH_SIZE			4096
#define HAL_FLASH_PAGE_NUM		4
#define HAL_FLASH_PAGE_SIZE		1024

#define HAL_FRAM_BASE_ADDR		0x0
#define HAL_FRAM_END_ADDR		0x3FFFF
#define HAL_FRAM_SIZE			0x40000

void hal_flash_init(void);

bool_t hal_flash_alloc(uint8_t index, uint8_t *buf, uint16_t len);

bool_t hal_flash_free(uint8_t index);

bool_t hal_flash_write(uint8_t index);

void *hal_flash_read(uint8_t index);

void hal_fram_init(void);

bool_t hal_fram_read(uint32_t addr, uint8_t *buf, uint32_t len);

bool_t hal_fram_write(uint32_t addr, uint8_t *buf, uint32_t len);

void hal_fram_get_ver(void);

void hal_fram_get_spi_cs(bool_t state);

void hal_fram_rdy_write(bool_t state);

#endif