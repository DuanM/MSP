#include <platform.h>
#include <device.h>
#include <ti\devices\msp432p4xx\inc\msp.h>
#include <ti\devices\msp432p4xx\inc\system_msp432p401r.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>
#include <ti\devices\msp432p4xx\driverlib\interrupt.h>

#include <control_io.h>


static void Key_Interrupt_Callback(void);
static void GPS_Interrupt_Callback(void);

void ControlIO_Init(void)
{
	device_info_t *p_device_info = device_info_get();
	
	GPIO_setAsOutputPin((ACS_LORA_M0_PIN/10), (1u<<(ACS_LORA_M0_PIN%10)));
	GPIO_setAsOutputPin((ACS_LORA_M1_PIN/10), (1u<<(ACS_LORA_M1_PIN%10)));
	GPIO_setAsInputPin((ACS_LORA_AUX_PIN/10), (1u<<(ACS_LORA_AUX_PIN%10)));
	
	GPIO_setAsOutputPin((DEV_STATE_LIGHT_RED_PIN/10), (1u<<(DEV_STATE_LIGHT_RED_PIN%10)));
	GPIO_setAsOutputPin((DEV_STATE_LIGHT_GREEN_PIN/10), (1u<<(DEV_STATE_LIGHT_GREEN_PIN%10)));
	GPIO_setAsOutputPin((DEV_STATE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_STATE_LIGHT_YELLOW_PIN%10)));
	
    GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_RED_PIN/10), (1u<<(DEV_STATE_LIGHT_RED_PIN%10)));
    GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_GREEN_PIN/10), (1u<<(DEV_STATE_LIGHT_GREEN_PIN%10)));
    GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_STATE_LIGHT_YELLOW_PIN%10)));
	
	GPIO_setAsOutputPin((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)));
	GPIO_setAsOutputPin((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)));
	GPIO_setAsOutputPin((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)));
	
    GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)));
    GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)));
    GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)));
	
	GPIO_setAsOutputPin((DEV_BUZZER_PIN/10), (1u<<(DEV_BUZZER_PIN%10)));
	GPIO_setOutputLowOnPin((DEV_BUZZER_PIN/10), (1u<<(DEV_BUZZER_PIN%10)));
	
	//sensor
	GPIO_setAsOutputPin((ACS_LORA_PWR_PIN/10), (1u<<(ACS_LORA_PWR_PIN%10)));
	GPIO_setOutputLowOnPin((ACS_LORA_PWR_PIN/10), (1u<<(ACS_LORA_PWR_PIN%10)));
	
	GPIO_setAsOutputPin((SEN_GPS_PWR_PIN/10), (1u<<(SEN_GPS_PWR_PIN%10)));
	GPIO_setOutputLowOnPin((SEN_GPS_PWR_PIN/10), (1u<<(SEN_GPS_PWR_PIN%10)));
    
	ControlIO_DevPower(CTL_LORA_PWR, PLAT_TRUE, PLAT_FALSE);
    ControlIO_DevPower(CTL_GPS_PWR, PLAT_TRUE, PLAT_FALSE);
	
    ControlIO_StatusLight(RED);
	
    DBG_PRINTF("LORA power=%d\r\n", p_device_info->pwr.lora);
	DBG_PRINTF("GPS power=%d\r\n", p_device_info->pwr.gps);
}

bool_t ControlIO_DevPower(uint8_t type, bool_t state, bool_t save)
{
	device_info_t *p_device_info = device_info_get();
	switch(type)
	{
		case CTL_LORA_PWR:
			if (p_device_info->pwr.lora != state)
			{
				if(state)
				{
					GPIO_setOutputHighOnPin((ACS_LORA_PWR_PIN/10), (1u<<(ACS_LORA_PWR_PIN%10)));
				}
				else
				{
					GPIO_setOutputLowOnPin((ACS_LORA_PWR_PIN/10), (1u<<(ACS_LORA_PWR_PIN%10)));
				}
				p_device_info->pwr.lora = state;	
			}
			break;
		case CTL_GPS_PWR:
			if (p_device_info->pwr.gps != state)
			{
				if(state)
				{
					GPIO_setOutputHighOnPin((SEN_GPS_PWR_PIN/10), (1u<<(SEN_GPS_PWR_PIN%10)));
				}
				else
				{
					GPIO_setOutputLowOnPin((SEN_GPS_PWR_PIN/10), (1u<<(SEN_GPS_PWR_PIN%10)));
				}
				p_device_info->pwr.gps = state;
			}
			break;
		default: break;
	}
	
	if (save)
	{
		device_info_set(p_device_info, PLAT_TRUE);
	}
	
	return PLAT_FALSE;
}


static fpv_t func;

static void GPS_Interrupt_Callback(void)
{
	uint_fast16_t status; 
	
	status = GPIO_getInterruptStatus((SEN_GPS_PULSE_PIN/10),(1u<<(SEN_GPS_PULSE_PIN%10)));
	GPIO_clearInterruptFlag((SEN_GPS_PULSE_PIN/10), status);
	(*func)();
}

void ControlIO_GPS_Pulse_Callback(fpv_t callback)
{
	func = callback;
	GPIO_setAsInputPin((SEN_GPS_PULSE_PIN/10), (1u<<(SEN_GPS_PULSE_PIN%10)));
	GPIO_interruptEdgeSelect((SEN_GPS_PULSE_PIN/10), (1u<<(SEN_GPS_PULSE_PIN%10)), GPIO_LOW_TO_HIGH_TRANSITION);
	GPIO_registerInterrupt((SEN_GPS_PULSE_PIN/10), GPS_Interrupt_Callback);
	GPIO_clearInterruptFlag((SEN_GPS_PULSE_PIN/10), (1u<<(SEN_GPS_PULSE_PIN%10)));
	GPIO_enableInterrupt((SEN_GPS_PULSE_PIN/10), (1u<<(SEN_GPS_PULSE_PIN%10)));
}

static fpv_t auxfunc;

static void Aux_Interrupt_Callback(void)
{
	uint_fast16_t status; 
	status = GPIO_getInterruptStatus((ACS_LORA_AUX_PIN/10),(1u<<(ACS_LORA_AUX_PIN%10)));
	
	if(DEV_AUX_PIN_VALUE)
	{
		GPIO_interruptEdgeSelect((ACS_LORA_AUX_PIN/10), (1u<<(ACS_LORA_AUX_PIN%10)), GPIO_HIGH_TO_LOW_TRANSITION);
	}
	else
	{
		GPIO_interruptEdgeSelect((ACS_LORA_AUX_PIN/10), (1u<<(ACS_LORA_AUX_PIN%10)), GPIO_LOW_TO_HIGH_TRANSITION);
	}
	
	GPIO_clearInterruptFlag((ACS_LORA_AUX_PIN/10), status);
	
	(*auxfunc)();
}

void ControlIO_Aux_Callback(fpv_t callback)
{
	auxfunc = callback;
	GPIO_setAsInputPin((ACS_LORA_AUX_PIN/10), (1u<<(ACS_LORA_AUX_PIN%10)));
	GPIO_interruptEdgeSelect((ACS_LORA_AUX_PIN/10), (1u<<(ACS_LORA_AUX_PIN%10)), GPIO_HIGH_TO_LOW_TRANSITION);
	GPIO_registerInterrupt((ACS_LORA_AUX_PIN/10), Aux_Interrupt_Callback);
	GPIO_clearInterruptFlag((ACS_LORA_AUX_PIN/10), (1u<<(ACS_LORA_AUX_PIN%10)));
	GPIO_enableInterrupt((ACS_LORA_AUX_PIN/10), (1u<<(ACS_LORA_AUX_PIN%10)));
}

bool_t ControlIO_LoraMode(uint8_t type)
{
	device_info_t *p_device_info = device_info_get();
	switch(type)
	{
		case LORA_M0://一般模式//M0=0;M1=0;
			if(p_device_info->param.lora_mode != LORA_M0)
			{
				GPIO_setOutputLowOnPin((ACS_LORA_M1_PIN/10), (1u<<(ACS_LORA_M1_PIN%10)));
				GPIO_setOutputLowOnPin((ACS_LORA_M0_PIN/10), (1u<<(ACS_LORA_M0_PIN%10)));
				p_device_info->param.lora_mode = LORA_M0;
			}
		break;
		case LORA_M1://唤醒模式//M0=1;M1=0;
			if(p_device_info->param.lora_mode != LORA_M1)
			{
				GPIO_setOutputHighOnPin((ACS_LORA_M0_PIN/10), (1u<<(ACS_LORA_M0_PIN%10)));
				GPIO_setOutputLowOnPin((ACS_LORA_M1_PIN/10), (1u<<(ACS_LORA_M1_PIN%10)));
				p_device_info->param.lora_mode = LORA_M1;
			}
		break;
		case LORA_M2://省电模式//M0=0;M1=1;
			if(p_device_info->param.lora_mode != LORA_M2)
			{
				GPIO_setOutputLowOnPin((ACS_LORA_M0_PIN/10), (1u<<(ACS_LORA_M0_PIN%10)));
				GPIO_setOutputHighOnPin((ACS_LORA_M1_PIN/10), (1u<<(ACS_LORA_M1_PIN%10)));
				p_device_info->param.lora_mode = LORA_M2;
			}	
		break;
		case LORA_M3://休眠模式//M0=1;M1=1;
			if(p_device_info->param.lora_mode != LORA_M3)
			{
				GPIO_setOutputHighOnPin((ACS_LORA_M0_PIN/10), (1u<<(ACS_LORA_M0_PIN%10)));
				GPIO_setOutputHighOnPin((ACS_LORA_M1_PIN/10), (1u<<(ACS_LORA_M1_PIN%10)));
				p_device_info->param.lora_mode = LORA_M3;
			}
		break;
		default:
			p_device_info->param.lora_mode = LORA_OTHER;
            return PLAT_FALSE;
		break;
	}
	delay_ms(10);
	return PLAT_TRUE;
}

static fpv_t keyfunc;
static void Key_Interrupt_Callback(void)
{
	uint_fast16_t status; 
	status = GPIO_getInterruptStatus((DEV_SW1_PIN/10),(1u<<(DEV_SW1_PIN%10)));
	GPIO_clearInterruptFlag((DEV_SW1_PIN/10), status);
	(*keyfunc)();
}

void ControlIO_Key_Callback(fpv_t callback)
{
	keyfunc = callback;
	GPIO_setAsInputPin((DEV_SW1_PIN/10), (1u<<(DEV_SW1_PIN%10))); //设置为输入
	GPIO_interruptEdgeSelect((DEV_SW1_PIN/10), (1u<<(DEV_SW1_PIN%10)), GPIO_HIGH_TO_LOW_TRANSITION);
	GPIO_registerInterrupt((DEV_SW1_PIN/10), Key_Interrupt_Callback);
	GPIO_clearInterruptFlag((DEV_SW1_PIN/10), (1u<<(DEV_SW1_PIN%10)));
	GPIO_enableInterrupt((DEV_SW1_PIN/10), (1u<<(DEV_SW1_PIN%10)));
}

void ControlIO_StatusLight(uint8_t type)
{
	device_info_t *p_device_info = device_info_get();
	switch(type)
	{
		case OFF://状态灯 全灭
			if(p_device_info->param.state_light.state != OFF)
			{
				GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_RED_PIN/10), (1u<<(DEV_STATE_LIGHT_RED_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_GREEN_PIN/10), (1u<<(DEV_STATE_LIGHT_GREEN_PIN%10)));
				GPIO_setOutputHighOnPin((DEV_STATE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_STATE_LIGHT_YELLOW_PIN%10)));
				p_device_info->param.state_light.state  = OFF;
			}
		break;
		case RED://状态灯 红
			if(p_device_info->param.state_light.state  != RED)
			{
				GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_GREEN_PIN/10), (1u<<(DEV_STATE_LIGHT_GREEN_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_STATE_LIGHT_YELLOW_PIN%10)));
				GPIO_setOutputHighOnPin((DEV_STATE_LIGHT_RED_PIN/10), (1u<<(DEV_STATE_LIGHT_RED_PIN%10)));
				p_device_info->param.state_light.state  = RED;
			}
			else if(p_device_info->param.state_light.filcker)
			{
				GPIO_toggleOutputOnPin((DEV_STATE_LIGHT_RED_PIN/10), (1u<<(DEV_STATE_LIGHT_RED_PIN%10)));
			}
		break;
		case GREEN://状态灯 绿
			if(p_device_info->param.state_light.state  != GREEN)
			{
				GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_STATE_LIGHT_YELLOW_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_RED_PIN/10), (1u<<(DEV_STATE_LIGHT_RED_PIN%10)));
				GPIO_setOutputHighOnPin((DEV_STATE_LIGHT_GREEN_PIN/10), (1u<<(DEV_STATE_LIGHT_GREEN_PIN%10)));
				p_device_info->param.state_light.state  = GREEN;
			}
			else if(p_device_info->param.state_light.filcker)
			{
				GPIO_toggleOutputOnPin((DEV_STATE_LIGHT_GREEN_PIN/10), (1u<<(DEV_STATE_LIGHT_GREEN_PIN%10)));
			}
		break;
		case YELLOW://状态灯 黄
			if(p_device_info->param.state_light.state  != YELLOW)
			{
				GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_RED_PIN/10), (1u<<(DEV_STATE_LIGHT_RED_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_STATE_LIGHT_GREEN_PIN/10), (1u<<(DEV_STATE_LIGHT_GREEN_PIN%10)));
				GPIO_setOutputHighOnPin((DEV_STATE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_STATE_LIGHT_YELLOW_PIN%10)));
				p_device_info->param.state_light.state  = YELLOW;
			}
			else if(p_device_info->param.state_light.filcker)
			{
				GPIO_toggleOutputOnPin((DEV_STATE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_STATE_LIGHT_YELLOW_PIN%10)));
			}
		break;
	}
}


void ControlIO_MoveLight_State(void)
{
	device_info_t *p_device_info = device_info_get();
	switch(p_device_info->param.move_light.state)
	{
		case RED://红灯;亮
			if(!DEV_MOVE_LIGHT_RED_PIN_VALUE)
			{
				GPIO_setOutputHighOnPin((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)));
			}
		break;
		case GREEN://绿灯;亮
			if(!DEV_MOVE_LIGHT_GREEN_PIN_VALUE)
			{
				GPIO_setOutputHighOnPin((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)));
			}
		break;
		case YELLOW://黄灯;亮
			if(!DEV_MOVE_LIGHT_YELLOW_PIN_VALUE)
			{
				GPIO_setOutputHighOnPin((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)));
			}
		break;
	}
}


void ControlIO_MoveLight(void)
{
	device_info_t *p_device_info = device_info_get();
	switch(p_device_info->param.move_light.state)
	{
		case OFF://全灭
			if(!DEV_MOVE_LIGHT_RED_PIN_VALUE || !DEV_MOVE_LIGHT_GREEN_PIN_VALUE || !DEV_MOVE_LIGHT_YELLOW_PIN_VALUE)
			{
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)));
			}
		break;
		case RED://红灯亮
			if(!DEV_MOVE_LIGHT_RED_PIN_VALUE)
			{
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)));
				GPIO_setOutputHighOnPin((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)));
			}
			else if(p_device_info->param.move_light.filcker)
			{
				GPIO_toggleOutputOnPin((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)));
			}
			
		break;
		case GREEN://绿灯;亮
			if(!DEV_MOVE_LIGHT_GREEN_PIN_VALUE)
			{
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)));
				GPIO_setOutputHighOnPin((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)));
			}
			else if(p_device_info->param.move_light.filcker)
			{
				GPIO_toggleOutputOnPin((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)));
			}
		break;
		case YELLOW://黄灯;亮
			if(!DEV_MOVE_LIGHT_YELLOW_PIN_VALUE)
			{
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)));
				GPIO_setOutputLowOnPin((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)));
				GPIO_setOutputHighOnPin((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)));
			}
			else if(p_device_info->param.move_light.filcker)
			{
				GPIO_toggleOutputOnPin((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)));
			}
		break;
	}
}

void ControlIO_Buzzer_State(void)
{
	device_info_t *p_device_info = device_info_get();
	switch(p_device_info->param.buzzer)
	{
	case BUZZER_OFF://关掉蜂鸣器
		if(DEV_BUZZER_PIN_VALUE) GPIO_setOutputLowOnPin((DEV_BUZZER_PIN/10), (1u<<(DEV_BUZZER_PIN%10)));
		break;
	case BUZZER_OPEN://打开蜂鸣器
 		if(!DEV_BUZZER_PIN_VALUE) GPIO_setOutputHighOnPin((DEV_BUZZER_PIN/10), (1u<<(DEV_BUZZER_PIN%10)));
		break;
	}
}

void ControlIO_Buzzer(void)
{
	device_info_t *p_device_info = device_info_get();
	switch(p_device_info->param.buzzer)
	{
	case BUZZER_OFF://关掉蜂鸣器
		if(DEV_BUZZER_PIN_VALUE) GPIO_setOutputLowOnPin((DEV_BUZZER_PIN/10), (1u<<(DEV_BUZZER_PIN%10)));
		break;
	case BUZZER_OPEN://打开蜂鸣器
		if(!DEV_BUZZER_PIN_VALUE) GPIO_setOutputHighOnPin((DEV_BUZZER_PIN/10), (1u<<(DEV_BUZZER_PIN%10)));
		break;
	case BUZZER_FLICKER://蜂鸣器闪鸣
		GPIO_toggleOutputOnPin((DEV_BUZZER_PIN/10), (1u<<(DEV_BUZZER_PIN%10)));
		break;
	}
}



