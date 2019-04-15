
#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\flash.h>
#include <ti\devices\msp432p4xx\driverlib\spi.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>

uint32_t flash_addr_manage[HAL_FLASH_PAGE_NUM];
uint16_t flash_size_manage[HAL_FLASH_PAGE_NUM];

void hal_flash_init(void)
{
	for (uint8_t i=0; i<HAL_FLASH_PAGE_NUM; i++)
	{
		flash_addr_manage[i] = 0;
		flash_size_manage[i] = 0;
	}
}

bool_t hal_flash_alloc(uint8_t index, uint8_t *buf, uint16_t len)
{	
	if (len > HAL_FLASH_PAGE_SIZE || buf == PLAT_NULL || index >= HAL_FLASH_PAGE_NUM)
	{
		return PLAT_FALSE;
	}

	if (flash_addr_manage[index] == (uint32_t)buf)
	{
		flash_size_manage[index] = len;
		return PLAT_TRUE;
	}	
		
	if (flash_addr_manage[index] == 0)
	{			
		flash_addr_manage[index] = (uint32_t)buf;
		flash_size_manage[index] = len;
		return PLAT_TRUE;
	}

	return PLAT_FALSE;
}

bool_t hal_flash_free(uint8_t index)
{	
	if (index < HAL_FLASH_PAGE_NUM)
	{	
		flash_addr_manage[index] = 0;
		flash_size_manage[index] = 0;
		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}

bool_t hal_flash_write(uint8_t index)
{
	bool_t res = PLAT_FALSE;

	if (index >= HAL_FLASH_PAGE_NUM)
	{
		return PLAT_FALSE;
	}
	
	if (flash_addr_manage[index] == 0
		|| flash_size_manage[index] == 0)
	{
		return PLAT_FALSE;
	}

	res = FlashCtl_isSectorProtected(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);

	if (res)
	{
		res = FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);
		if (res == PLAT_FALSE)
		{
			DBG_TRACE("res == PLAT_FALSE\r\n");
			return PLAT_FALSE;
		}		
	}

	res = FlashCtl_eraseSector(HAL_FLASH_BASE_ADDR);
	if (res == PLAT_FALSE)
	{
		DBG_TRACE("res == PLAT_FALSE\r\n");
		return PLAT_FALSE;
	}
	
	for (uint8_t i=0; i<HAL_FLASH_PAGE_NUM; i++)
	{
		if (flash_addr_manage[i] != 0)
		{
			res = FlashCtl_programMemory((void *)flash_addr_manage[i],
									(void *)(HAL_FLASH_PAGE_SIZE*i+HAL_FLASH_BASE_ADDR),
									flash_size_manage[i]);
			if (res == PLAT_FALSE)
			{
				DBG_TRACE("res == PLAT_FALSE\r\n");
				return PLAT_FALSE;
			}
		}		
	}	
	
	return res;
}

void *hal_flash_read(uint8_t index)
{
	if (index < HAL_FLASH_PAGE_NUM)
	{	
		return (void *)(HAL_FLASH_PAGE_SIZE*index + HAL_FLASH_BASE_ADDR);
	}
	else
	{
		return PLAT_NULL;
	}
}

#define WREN	0x06
#define WRDI	0x04
#define RDSR	0x05
#define WRSR	0x01
#define READ	0x03
#define FSTRD	0x0B
#define WRITE	0x02
#define SLEEP	0xB9
#define RDID	0x9F

#define FRAM_SPI_CE		14
#define FRAM_SPI_SCK	15
#define FRAM_SPI_MOSI	16
#define FRAM_SPI_MISO	17

#define FRAM_SPI_CE_H()    (P1OUT |= BIT4)
#define FRAM_SPI_CE_L()    (P1OUT &= ~BIT4)

static uint8_t SPIx_ReadWriteByte(uint8_t data);

void hal_fram_init(void)
{
	GPIO_setAsOutputPin((FRAM_SPI_CE/10), (1u<<(FRAM_SPI_CE%10)));
	FRAM_SPI_CE_H();
    
	GPIO_setAsPeripheralModuleFunctionOutputPin((FRAM_SPI_SCK/10), 
													(1u<<(FRAM_SPI_SCK%10)), 
													GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionOutputPin((FRAM_SPI_MOSI/10), 
												(1u<<(FRAM_SPI_MOSI%10)), 
												GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionInputPin((FRAM_SPI_MISO/10), 
												(1u<<(FRAM_SPI_MISO%10)), 
												GPIO_PRIMARY_MODULE_FUNCTION);
	eUSCI_SPI_MasterConfig config;
	config.spiMode = EUSCI_B_SPI_3PIN;
	config.selectClockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK;
	config.msbFirst = EUSCI_B_SPI_MSB_FIRST;
	config.desiredSpiClock = 4000000;
	config.clockSourceFrequency = 12000000;
	config.clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
	config.clockPhase = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    
	SPI_initMaster(EUSCI_B0_SPI_BASE, &config);
    
	SPI_enableModule(EUSCI_B0_SPI_BASE);
    
	hal_fram_get_ver(); 
}

bool_t hal_fram_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
	uint8_t temH,temM,temL;
	uint32_t i;
	
	temH=(uint8_t)((addr&0xff0000)>>16);
	temM=(uint8_t)((addr&0x00ff00)>>8);
	temL=(uint8_t)(addr&0x0000ff);

	FRAM_SPI_CE_L();           		//片选CS=0
	SPIx_ReadWriteByte(READ);     	//读命令
	SPIx_ReadWriteByte(temH);      	//高位地址
	SPIx_ReadWriteByte(temM);      	//中位地址
	SPIx_ReadWriteByte(temL);      	//低位地址
	//读数据

	for(i=0;i<len;i++)
	{
		*buf++ = SPIx_ReadWriteByte(0xF0);
	}
    
	FRAM_SPI_CE_H();           //片选CS=1
    
    return PLAT_TRUE;
}

bool_t hal_fram_write(uint32_t addr, uint8_t *buf, uint32_t len)
{
	uint8_t temH,temM,temL;
	uint32_t i;

	temH=(uint8_t)((addr&0xff0000)>>16);
	temM=(uint8_t)((addr&0x00ff00)>>8);
	temL=(uint8_t)(addr&0x0000ff);
	
	FRAM_SPI_CE_L();
	SPIx_ReadWriteByte(WREN);		  //写使能
	FRAM_SPI_CE_H(); 
	
	delay_us(5);
	
	FRAM_SPI_CE_L();
	SPIx_ReadWriteByte(WRITE);		  //写命令
	SPIx_ReadWriteByte(temH);		  //高位地址
	SPIx_ReadWriteByte(temM);		  //中位地址
	SPIx_ReadWriteByte(temL);		  //低位地址
	//写入数据
	if (buf != PLAT_NULL)
	{
		for(i=0;i<len;i++)
		{
		  SPIx_ReadWriteByte(*buf++);
		}
	}
	else
	{
		for(i=0;i<len;i++)
		{
		 SPIx_ReadWriteByte(255-(uint8_t)i);
		}
	}
	FRAM_SPI_CE_H();
	
	delay_us(5);
	
	FRAM_SPI_CE_L();
	SPIx_ReadWriteByte(WRDI);		  //写禁能
	FRAM_SPI_CE_H();
    
    return PLAT_TRUE;
}

void hal_fram_get_ver(void)
{
	uint8_t value;

	DBG_PRINTF("FRAM Version");
	
	FRAM_SPI_CE_L(); 
    SPIx_ReadWriteByte(RDID);
    
    for(uint8_t i=0; i<9; i++)
    {
        value = SPIx_ReadWriteByte(0xFF);        
        DBG_PRINTF(":%x", value);
    }
	FRAM_SPI_CE_H();

	DBG_PRINTF("\r\n");
}

void hal_fram_get_spi_cs(bool_t state)
{
	if (state)
	{
		FRAM_SPI_CE_H();
	}
	else
	{
		FRAM_SPI_CE_L();
	}
}

void hal_fram_rdy_write(bool_t state)
{
	if (state == PLAT_TRUE)
	{
		FRAM_SPI_CE_L();
		SPIx_ReadWriteByte(WREN);		  //写使能
		FRAM_SPI_CE_H();		
		delay_us(5);		
		FRAM_SPI_CE_L();
	}
	else
	{
		FRAM_SPI_CE_H();	
		delay_us(5);		
		FRAM_SPI_CE_L();
		SPIx_ReadWriteByte(WRDI);		  //写禁能
		FRAM_SPI_CE_H();
	}
}


static uint8_t SPIx_ReadWriteByte(uint8_t data)
{
	while (UCB0STAT & UCBUSY);
	
	UCB0TXBUF = data;
	
	while (UCB0STAT & UCBUSY);

	return (uint8_t)UCB0RXBUF;	
}



