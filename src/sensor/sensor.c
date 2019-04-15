#include <platform.h>
#include <fxos8700cq.h>
#include <sensor.h>


#define SENSOR_TASK_STK_SIZE		256
#define SENSOR_TASK_PRIO			OSEL_TASK_PRIO(2)

OSEL_DECLARE_TASK(SENSOR_TASK, param);

osel_task_t *sensor_task_h;
osel_event_t *sensor_event_h;

static uint8_t sensor_timeout_id = 0;
static void sensor_timeout_callback(void);

void sensor_init(void)
{
  	/*创建 SENSOR 任务 */   
	sensor_task_h = osel_task_create(SENSOR_TASK, 
    								PLAT_NULL, 
    								SENSOR_TASK_STK_SIZE, 
    								SENSOR_TASK_PRIO);
    DBG_ASSERT(sensor_task_h != PLAT_NULL);
    
	sensor_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(sensor_event_h != PLAT_NULL);
	
	bool_t fxos;
	fxos = FXOS_Config();
	DBG_ASSERT(fxos != PLAT_FALSE);
}

OSEL_DECLARE_TASK(SENSOR_TASK, param)
{
	(void)param;
	osel_event_res_t res;
	
	DBG_TRACE("SENSOR_TASK!\r\n");
	
	sensor_timeout_id = hal_timer_alloc(SENSOR_TIMEOUT, sensor_timeout_callback);
	
	while(1)
	{
		res = osel_event_wait(sensor_event_h, OSEL_WAIT_FOREVER);
		
		if (res == OSEL_EVENT_NONE)
		{
			sensor_handler(OSEL_EVENT_GET(sensor_event_h, uint16_t));
		}
		else if (res == OSEL_EVENT_TIMEOUT)
		{
			//sensor_timeout_handler();
		}
	}
}

static void sensor_timeout_callback(void)
{
	sensor_timeout_id = hal_timer_free(sensor_timeout_id);
	
	uint16_t object = SENSOR_EVENT_FXOS;
	osel_event_set(sensor_event_h, &object);
	
	sensor_timeout_id = hal_timer_alloc(SENSOR_TIMEOUT, sensor_timeout_callback);
}

void sensor_handler(uint16_t event_type)
{
	uint16_t object = 0;	
	
	if (event_type & SENSOR_EVENT_FXOS)
	{
		object = SENSOR_EVENT_FXOS;
		osel_event_clear(sensor_event_h, &object);
		FXOS_Get_Data();//串口
	}
	else
	{
		//DBG_TRACE("no this event type!\r\n");
	}
}


