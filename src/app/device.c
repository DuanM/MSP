#include <platform.h>
#include <device.h>

#define SAVE_SIZE	(sizeof(device_info_t)-sizeof(dev_power_t)-sizeof(dev_time_t)-sizeof(dev_pos_t)-sizeof(dev_param_t))

static device_info_t *pg_device_info = PLAT_NULL;

void device_info_init(void)
{
    OSEL_DECL_CRITICAL();
    bool_t res;
	device_info_t *p_flash_device_info = PLAT_NULL;
    
    DBG_ASSERT(SAVE_SIZE <= HAL_FLASH_PAGE_SIZE);	
    
	if (pg_device_info == PLAT_NULL)
	{
		pg_device_info = heap_alloc(sizeof(device_info_t), PLAT_TRUE);
		DBG_ASSERT(pg_device_info != PLAT_NULL);
	}
    
    res = hal_flash_alloc(DEVICE_STORE_INDEX, (uint8_t *)pg_device_info, SAVE_SIZE);
    if (res == PLAT_FALSE)
    {
        DBG_TRACE("res == PLAT_FALSE\r\n");
        return;
    }
    
    p_flash_device_info = (device_info_t *)hal_flash_read(DEVICE_STORE_INDEX);
    
	if (GET_DEV_TYPE_ID(p_flash_device_info->id) != 0xA4)
	{
		//默认为DEV_NULL
		SET_DEV_ID(pg_device_info->id, 0x1);
		SET_DEV_GROUP_ID(pg_device_info->id, 0x1);
		SET_DEV_MODE_ID(pg_device_info->id, 0x0);
		SET_DEV_TYPE_ID(pg_device_info->id, 0xA4);
		
		pg_device_info->lora_cfg.MODE = LORA_CFG_MODE;
		pg_device_info->lora_cfg.ADDH = 0x0;
		pg_device_info->lora_cfg.ADDL = 0x0;
		
		pg_device_info->lora_cfg.SPED.SKY_BPS = BPS2P4K;
		pg_device_info->lora_cfg.SPED.TTL_BPS = TTL9600;
		pg_device_info->lora_cfg.SPED.PBT = pbt8N1;
		
		pg_device_info->lora_cfg.CHAN.CHAN_NUM = LORA_DEFAULT_CHAN;
		pg_device_info->lora_cfg.CHAN.CHAN_RES = LORA_MIN_CHAN;
		
		pg_device_info->lora_cfg.OPTION.OPT_MODBUS = PTOP;
		pg_device_info->lora_cfg.OPTION.OPT_IO_DEVICE = PushPull;
		pg_device_info->lora_cfg.OPTION.OPT_RF_AWAKE_TIME = TM250m;
		pg_device_info->lora_cfg.OPTION.OPT_FEC = REC_OPEN;
		pg_device_info->lora_cfg.OPTION.OPT_RF_TRS_PER = PER30dBm;
		
		OSEL_ENTER_CRITICAL();
		hal_flash_write(DEVICE_STORE_INDEX);
		OSEL_EXIT_CRITICAL();
	}
	else
	{
		mem_cpy(pg_device_info, p_flash_device_info, SAVE_SIZE);
	}
	
	pg_device_info->pwr.lora = 0;
	pg_device_info->pwr.gps = 0;
	
	pg_device_info->param.software = SOFT_VERSION;
	
	//定位
 	pg_device_info->pos.sn = 'V';
	pg_device_info->pos.we = 'V';
	
	if(pg_device_info->lora_cfg.ADDL != GET_DEV_ID(pg_device_info->id))
	{
		pg_device_info->lora_cfg.ADDH = 0x00;
		pg_device_info->lora_cfg.ADDL = GET_DEV_ID(pg_device_info->id);
		OSEL_ENTER_CRITICAL();
		hal_flash_write(DEVICE_STORE_INDEX);
		OSEL_EXIT_CRITICAL();
	}
	
	mem_cpy(&pg_device_info->param.lora_cfg,&pg_device_info->lora_cfg,sizeof(dev_lora_t));
	
	//罗盘
	mem_set(&pg_device_info->param.fxos_data,0,sizeof(dev_fxos_t));
	
	
	srand(GET_DEV_ID(pg_device_info->id));
	
	DBG_PRINTF("\r\nSoftware version=0x%X\r\n",pg_device_info->param.software);
	
	DBG_PRINTF("Device_ID(hex)=%x:%x:%x:%x\r\n",
				pg_device_info->id[0],
				pg_device_info->id[1],
				pg_device_info->id[2],
				pg_device_info->id[3]);
}

device_info_t *device_info_get(void)
{	
	return pg_device_info;
}

bool_t device_info_set(device_info_t *device_info, bool_t force_update)
{
	OSEL_DECL_CRITICAL();

	if (pg_device_info == device_info)
	{
		if (force_update == PLAT_TRUE)
		{
			OSEL_ENTER_CRITICAL();
			hal_flash_write(DEVICE_STORE_INDEX);
			OSEL_EXIT_CRITICAL();
		}		
		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}

uint32_t device_lora_baudrate_info_get(uint8_t buadR)
{
	uint32_t baudrate = 0;
	switch(buadR)
	{
	case TTL1200:baudrate = 1200;
		break;
	case TTL2400:baudrate = 2400;
		break;
	case TTL4800:baudrate = 4800;
		break;
	case TTL9600:baudrate = 9600;
		break;
	case TTL19200:baudrate = 19200;
		break;
	case TTL38400:baudrate = 38400;
		break;
	case TTL57600:baudrate = 57600;
		break;
	case TTL115200:baudrate = 115200;
		break;
	default :baudrate = 9600;
		break;
	}
	return baudrate;
}






