#include <platform.h>
#include <device.h>
#include <test.h>
#include <sensor.h>
#include <app.h>
#include <stack.h>
#include <control_io.h>

#define INIT_TASK_STK_SIZE			256
#define INIT_TASK_PRIO				OSEL_TASK_PRIO(0)

OSEL_DECLARE_TASK(INIT_TASK, param);
osel_task_t *init_task_h;

void main(void)
{
    DBG_SET_LEVEL(DBG_LEVEL_PRINTF);
    
    hal_board_init();
    
	memory_init(PLAT_NULL);
    
    hal_uart_init(UART_DEBUG, 115200);
    
	hal_flash_init();    
    
    hal_timer_init();
	
	device_info_init();
    
	ControlIO_Init();
    
	//hal_fram_init();
    
	hal_adc14_init();
    
	hal_rtc_init();
    
	osel_init();
    
	init_task_h = osel_task_create(INIT_TASK, 
    								PLAT_NULL, 
    								INIT_TASK_STK_SIZE, 
    								INIT_TASK_PRIO);
    DBG_ASSERT(init_task_h != PLAT_NULL);
    
	osel_start();
    
    while(1);
}

void app_idle_hook(void)
{
	//轮询方式读取gps
	app_gps_poll_callback();
}


OSEL_DECLARE_TASK(INIT_TASK, param)
{
	(void)param;
	bool_t res;
	device_info_t *p_device_info = device_info_get();

	DBG_PRINTF("%s\r\n", __TIMESTAMP__);

	res = kbuf_init();
	DBG_ASSERT(res == PLAT_TRUE);	
	
	osel_systick_init();
	
	test_init();

	app_init();

	sensor_init();
	
	stack_init();
	
	//DBG_SET_LEVEL(DBG_LEVEL_CRITICAL);
	APP_DBG_SET(LORA_PRINTF_LEVEL, PLAT_TRUE);
	
	osel_task_idle_hook_reg(0, app_idle_hook);//nwk
	
	while(1)
	{
		test_handler();
	}
}   



