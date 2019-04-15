#ifndef __APP_DBG_H
#define __APP_DBG_H

#define TEST_PRINTF_LEVEL		0
#define LORA_PRINTF_LEVEL 		1
#define GPS_PRINTF_LEVEL		2
#define FXOS_PRINTF_LEVEL		3
#define INTELL_PRINTF_LEVEL		4
#define STACK_PRINTF_LEVEL		5

#pragma pack(1)

typedef struct
{
	uint8_t test:	1,
			lora:	1,
			gps:	1,
			fxos:	1,
			intell:	1,
			stack:	1,
			reserve:2;
}app_dbg_flag_t;

#pragma pack()

extern app_dbg_flag_t app_dbg_flag;

//调试专用dbg
#define DBG_LORA_PRINTF(...) 	APP_DBG_PRINT(LORA_PRINTF_LEVEL, __VA_ARGS__)
#define DBG_GPS_PRINTF(...) 	APP_DBG_PRINT(GPS_PRINTF_LEVEL, __VA_ARGS__)
#define DBG_FXOS_PRINTF(...) 	APP_DBG_PRINT(FXOS_PRINTF_LEVEL, __VA_ARGS__)

#define DBG_INTELL_PRINTF(...) 	APP_DBG_PRINT(INTELL_PRINTF_LEVEL, __VA_ARGS__)
#define DBG_STACK_PRINTF(...) 	APP_DBG_PRINT(STACK_PRINTF_LEVEL, __VA_ARGS__)

#define DBG_LORA_FLG 	(app_dbg_flag.lora)
#define DBG_GPS_FLG 	(app_dbg_flag.gps)
#define DBG_FXOS_FLG 	(app_dbg_flag.fxos)
#define DBG_INTELL_FLG 	(app_dbg_flag.intell)
#define DBG_STACK_FLG 	(app_dbg_flag.stack) 


#define APP_DBG_INIT()									\
	do													\
	{													\
		app_dbg_init();									\
	} while (__LINE__ == -1)

void app_dbg_init(void);

void app_dbg_set(uint8_t level, uint8_t flag);

#define APP_DBG_SET(level, flag)						\
	do													\
	{													\
		app_dbg_set(level, flag);						\
	} while (__LINE__ == -1)
	



#include "platform.h"

#define APP_DBG_PRINT(level, ...)									\
	do																\
	{																\
		if (level == LORA_PRINTF_LEVEL && app_dbg_flag.lora)			\
		{															\
			plat_dbg_print(DBG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__);	\
		}																	\
		else if (level == GPS_PRINTF_LEVEL && app_dbg_flag.gps)		\
		{																	\
			plat_dbg_print(DBG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__);	\
		}																	\
		else if (level == FXOS_PRINTF_LEVEL && app_dbg_flag.fxos)			\
		{																	\
			plat_dbg_print(DBG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__);	\
		}																	\
		else if (level == INTELL_PRINTF_LEVEL && app_dbg_flag.intell)	\
		{																	\
			plat_dbg_print(DBG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__);	\
		}																	\
		else if (level == TEST_PRINTF_LEVEL && app_dbg_flag.test)		\
		{																	\
			plat_dbg_print(DBG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__);	\
		}																	\
		else if (level == STACK_PRINTF_LEVEL && app_dbg_flag.stack)		\
		{																	\
			plat_dbg_print(DBG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__);	\
		}																	\
	} while (__LINE__ == -1)							
      

      
      
#endif


