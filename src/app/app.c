#include <platform.h>
#include <device.h>
#include <app.h>
#include <stack.h>

#define APP_TASK_STK_SIZE			256
#define APP_TASK_PRIO				OSEL_TASK_PRIO(3)

OSEL_DECLARE_TASK(APP_TASK, param);

osel_task_t *app_task_h;
osel_event_t *app_event_h;

list_t app_data_tx_q;

static void app_dbg_recv_callback(void);
static void app_lora_cfg_event(void);
static void app_tx_event(void);
static void app_key_callback(void);
static void app_aux_event(void);

static uint8_t lora_cfg_timeout_id = 0;

void app_init(void)
{
	app_task_h = osel_task_create(APP_TASK, 
									NULL, 
									APP_TASK_STK_SIZE, 
									APP_TASK_PRIO);
	DBG_ASSERT(app_task_h != PLAT_NULL);
	
	app_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(app_event_h != PLAT_NULL);
	
	//DBG
	hal_uart_rx_irq_enable(UART_DEBUG,1,app_dbg_recv_callback);
	
	hal_uart_init(UART_GPS, 9600);//GPS
	
	ControlIO_Key_Callback(app_key_callback);
	
	list_init(&app_data_tx_q);
	stack_priv_list_cfg(&app_data_tx_q);
	ControlIO_Aux_Callback(app_aux_event);
	stack_indicate(app_indicate_event);
	stack_config(app_lora_cfg_handler);
	stack_priv_list_send_cfg(app_tx_event);
}

OSEL_DECLARE_TASK(APP_TASK, param)
{
	osel_event_res_t res;
	(void)param;	
	device_info_t *device_info = device_info_get();
	device_info->param.lora_state = PLAT_TRUE;
	lora_cfg_timeout_id = hal_timer_alloc(APP_EVENT_TIMEOUT, app_lora_cfg_event);
	DBG_PRINTF("APP_TASK!\r\n");
	
	while (1)
	{
		res = osel_event_wait(app_event_h, OSEL_WAIT_FOREVER);
		if (res == OSEL_EVENT_NONE)
		{
			app_handler(OSEL_EVENT_GET(app_event_h, uint16_t));
		}
		else if (res == OSEL_EVENT_TIMEOUT)
		{
			
		}
	}
}

void app_indicate_event(void)
{
	uint16_t object = APP_EVENT_INDICATE;
	osel_event_set(app_event_h, &object);
}

static void app_aux_event(void)
{
	uint16_t object = APP_EVENT_AUX;
	osel_event_set(app_event_h, &object);
}


void app_lora_recv_callback(void)
{
	uint16_t object = APP_EVENT_LORA_RX;
	osel_event_set(app_event_h, &object);
}

//dbg
static void app_dbg_recv_callback(void)
{
	uint16_t object = APP_EVENT_DBG_RX;
	osel_event_set(app_event_h, &object);
}

static void app_lora_cfg_event(void)
{
	device_info_t *device_info = device_info_get();
	
	lora_cfg_timeout_id = hal_timer_free(lora_cfg_timeout_id);
	
	if(device_info->param.lora_state)
	{
		uint16_t object = APP_EVENT_CFG;
		osel_event_set(app_event_h, &object);
		lora_cfg_timeout_id = hal_timer_alloc(APP_EVENT_TIMEOUT, app_lora_cfg_event);
	}
}

static void app_tx_event(void)
{
	uint16_t object = APP_EVENT_DBG_TX;
	osel_event_set(app_event_h, &object);
}

static void app_key_callback(void)
{
	uint16_t object = APP_EVENT_KEY;
	osel_event_set(app_event_h, &object);
}

//APP
void app_lora_cfg_handler(uint8_t *cfg_buf)
{
	device_info_t *device_info = device_info_get();
	dev_lora_t *dev_lora = (dev_lora_t *)cfg_buf;
	
	nwk_param.ctrl_type = PLAT_TRUE;
	
	if(device_info->param.lora_cfg.CHAN.CHAN_NUM != dev_lora->CHAN.CHAN_NUM ||  device_info->param.lora_cfg.OPTION.OPT_FEC != dev_lora->OPTION.OPT_FEC ||
	device_info->param.lora_cfg.OPTION.OPT_IO_DEVICE != dev_lora->OPTION.OPT_IO_DEVICE ||  device_info->param.lora_cfg.OPTION.OPT_MODBUS != dev_lora->OPTION.OPT_MODBUS || 
	device_info->param.lora_cfg.OPTION.OPT_RF_AWAKE_TIME != dev_lora->OPTION.OPT_RF_AWAKE_TIME || device_info->param.lora_cfg.OPTION.OPT_RF_TRS_PER != dev_lora->OPTION.OPT_RF_TRS_PER ||
	device_info->param.lora_cfg.SPED.PBT != dev_lora->SPED.PBT ||  device_info->param.lora_cfg.SPED.SKY_BPS != dev_lora->SPED.SKY_BPS || device_info->param.lora_cfg.SPED.TTL_BPS != dev_lora->SPED.TTL_BPS)
	{
		device_info->param.lora_state = PLAT_TRUE;
		
		while(!DEV_AUX_PIN_VALUE)
		{//等 AUX 为高电平 即LORA 空闲状态
			delay_ms(10);
		}
		
		ControlIO_LoraMode(LORA_M3);
		hal_uart_init(UART_LORA, 9600); //lora
		delay_ms(10);
		
		hal_uart_send_string(UART_LORA, (uint8_t *)dev_lora,sizeof(dev_lora_t));
		
		mem_cpy(&device_info->param.lora_cfg,dev_lora,sizeof(dev_lora_t));
		if(device_info->param.lora_cfg.MODE == LORA_CFG_MODE)
		{//保存flash
			mem_cpy(&device_info->lora_cfg,dev_lora,sizeof(dev_lora_t));
			device_info_set(device_info,PLAT_TRUE);
		}
		
		hal_uart_init(UART_LORA, device_lora_baudrate_info_get(device_info->param.lora_cfg.SPED.TTL_BPS)); //主控对lora串口波特率
		
		while(!DEV_AUX_PIN_VALUE)
		{//等 AUX 为高电平 即LORA 配置完成
			delay_ms(10);
		}
		
		ControlIO_LoraMode(LORA_M0);
		
		delay_ms(10);
		
		device_info->param.lora_state = PLAT_FALSE;
    }
}



