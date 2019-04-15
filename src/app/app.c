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
	
	list_init(&app_data_tx_q);
	stack_priv_list_cfg(&app_data_tx_q);
	
	stack_indicate(app_indicate_event);
	stack_config(app_lora_cfg_event);
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

//dbg
static void app_dbg_recv_callback(void)
{
	uint16_t object = APP_EVENT_DBG_RX;
	osel_event_set(app_event_h, &object);
}

void app_lora_recv_callback(void)
{
	uint16_t object = APP_EVENT_LORA_RX;
	osel_event_set(app_event_h, &object);
}

void app_indicate_event(void)
{
	uint16_t object = APP_EVENT_INDICATE;
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



