#include <platform.h>
#include <device.h>
#include <app.h>

app_dbg_flag_t app_dbg_flag;

void app_dbg_init(void)
{
	mem_clr(&app_dbg_flag, sizeof(app_dbg_flag_t));
}

void app_dbg_set(uint8_t level, uint8_t flag)
{
	if (level == LORA_PRINTF_LEVEL)
	{
		app_dbg_flag.lora = flag;
	}
	else if (level == GPS_PRINTF_LEVEL)
	{
		app_dbg_flag.gps = flag;
	}
	else if (level == FXOS_PRINTF_LEVEL)
	{
		app_dbg_flag.fxos = flag;
	}
	else if (level == TEST_PRINTF_LEVEL)
	{
		app_dbg_flag.test = flag;
	}
	else if (level == INTELL_PRINTF_LEVEL)
	{
		app_dbg_flag.intell = flag;
	}
	else if (level == STACK_PRINTF_LEVEL)
	{
		app_dbg_flag.stack = flag;
	}
}

