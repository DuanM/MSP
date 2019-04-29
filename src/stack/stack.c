#include <platform.h>
#include <device.h>
#include <app.h>
#include <stack.h>
#include <stack_priv.h>
#include <mac.h>

#define STACK_TASK_STK_SIZE		256
#define STACK_TASK_PRIO		OSEL_TASK_PRIO(1)

OSEL_DECLARE_TASK(STACK_TASK, param);

osel_task_t *stack_task_h;
osel_event_t *stack_event_h;

list_t stack_data0_tx_q;
list_t stack_data1_tx_q;
list_t stack_data_rx_q;

static fpv_t stack_indicate_func = PLAT_NULL;
static fpv_t stack_send_func = PLAT_NULL;
static fppcv_t stack_cfg_func = PLAT_NULL;

static list_t *stack_data_tx_q;

static void stack_gps_pulse_callback(void);
static void stack_lora_recv_callback(void);

void stack_init(void)
{
	/*创建 STACK 任务 */   
	stack_task_h = osel_task_create(STACK_TASK, 
									PLAT_NULL, 
									STACK_TASK_STK_SIZE, 
									STACK_TASK_PRIO);
	DBG_ASSERT(stack_task_h != PLAT_NULL);
	
	stack_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(stack_event_h != PLAT_NULL);
	
	//htimer_init();
	
	list_init(&stack_data0_tx_q);
	list_init(&stack_data1_tx_q);
	list_init(&stack_data_rx_q);
}

OSEL_DECLARE_TASK(STACK_TASK, param)
{
    (void)param;
	osel_event_res_t res;	
	
    DBG_PRINTF("STACK_TASK!\r\n");
	
 	mac_pib_init();
 	
	ControlIO_GPS_Pulse_Callback(stack_gps_pulse_callback);//GPS 秒脉冲
	
	while(1)
	{
		res = osel_event_wait(stack_event_h, OSEL_WAIT_FOREVER);
		
		if (res == OSEL_EVENT_NONE)
		{
			mac_handler(OSEL_EVENT_GET(stack_event_h, uint16_t));            
		}
		else if (res == OSEL_EVENT_TIMEOUT)
		{
			//mac_timeout_handler();
		}
	}
}

void stack_uart_lora_irq_enable_callback(void)
{
	hal_uart_rx_irq_enable(UART_LORA,1,stack_lora_recv_callback);
}

//GPS 脉冲回调
static void stack_gps_pulse_callback(void)
{
	uint16_t object = STACK_EVENT_PULSE;
	osel_event_set(stack_event_h, &object);
}

void stack_tx_event(void)
{
	uint16_t object = STACK_EVENT_TX;
	osel_event_set(stack_event_h, &object);
}

//串口lora回调
static void stack_lora_recv_callback(void)
{
	uint16_t object = STACK_EVENT_RX;
	osel_event_set(stack_event_h, &object);
}

void stack_indicate(fpv_t func)
{
	if(func != PLAT_NULL)
	stack_indicate_func = func;
}

void stack_indicate_handle(void)
{
	if(stack_indicate_func != PLAT_NULL)
	(*stack_indicate_func)();
}

void stack_config(fppcv_t func)
{
	if(func != PLAT_NULL)
	stack_cfg_func = func;
}

void stack_config_handle(uint8_t *cfg_buf,device_info_t *device_info)
{
	if(stack_cfg_func != PLAT_NULL)
	(*stack_cfg_func)(cfg_buf,device_info);
}

void stack_priv_list_cfg(list_t *list)
{
	if(list != PLAT_NULL)
	stack_data_tx_q = list;
}

list_t * stack_priv_list_get_handle(void)
{
	return stack_data_tx_q;
}

void stack_priv_list_send_cfg(fpv_t func)
{
	if(func != PLAT_NULL)
	stack_send_func = func;
}

void stack_priv_list_send_handle(void)
{
	if(stack_send_func != PLAT_NULL)
	(*stack_send_func)();
}
