#ifndef __DEVICE_H
#define __DEVICE_H
#include <app_dbg.h>

#define SOFT_VERSION		0x1002
//此版本初步验证完成了网络的组网，以及设备工作时隙进行的数据相互收发的无误，还有设备功能的初级验证。。

#define DEVICE_STORE_INDEX  1

#define GET_DEV_MAC(mac)			(mac[5])
#define SET_DEV_MAC(mac,b)			{mac[5]=b;}

#define GET_DEV_ID(id)				(id[3])
#define GET_DEV_GROUP_ID(id)		(id[2])
#define GET_DEV_MODE_ID(id)			(id[1])
#define GET_DEV_TYPE_ID(id)			(id[0])

#define SET_DEV_ID(id,b)			{id[3]=b;}
#define SET_DEV_GROUP_ID(id,b)		{id[2]=b;}
#define SET_DEV_MODE_ID(id,b)		{id[1]=b;}
#define SET_DEV_TYPE_ID(id,b)		{id[0]=b;}

#define GPS_CALC (1000000.0)
#define COORD_CALC (100.0)

//设备行动指示
#define IDLE_MOVE			0x0
#define READY_MOVE			0x1
#define	START_MOVE			0x2
#define	MIDDLE_STOP_MOVE	0x3
#define	PASSIVITY_MOVE		0x4
#define	STOP_MOVE			0x5

/**
 * device LORA 信道设置范围
 */
#define  LORA_DEFAULT_CHAN 23 //0-36H
#define  LORA_MIN_CHAN 0
#define  LORA_MAX_CHAN 36


/**
 * device Lora TRS_PER 发射功率(大约值)
 */
#define PER30dBm	0x0//（默认）
#define	PER27dBm	0x1
#define	PER24dBm	0x2
#define	PER21dBm	0x3 


/**
 * device Lora PBT
 */
#define pbt8N1	 0x0//（默认）
#define	pbt8O1	 0x1
#define	pbt8E1	 0x2 

/**
 * device Lora PBT
 */
#define	TTL1200 	0x0
#define	TTL2400 	0x1
#define	TTL4800 	0x2
#define	TTL9600 	0x3//（默认）
#define	TTL19200 	0x4
#define	TTL38400 	0x5
#define	TTL57600 	0x6
#define	TTL115200 	0x7 

/**
 * device Lora SKY_BPS
 */	
#define	BPS0P3K 	0x0
#define	BPS0P6K 	0x1
#define	BPS1P2K 	0x2
#define	BPS2P4K 	0x3//（默认）
#define	BPS4P8K 	0x4
#define	BPS9P6K 	0x5

/**
 * device Lora option modbus
 */
#define	LUCY	0x0 //透传
#define	PTOP	0x1 //定点

/**
 * device Lora OPT_IO_DEVICE
 */	
#define	PushPull	0x1//推挽
#define	OpenDrain	0x0 //开漏
/**
 * device Lora awake time
 */
#define	TM250m	0x0//（默认）
#define	TM500m	0x1
#define	TM750m	0x2
#define	TM1000m	0x3
#define	TM1250m	0x4
#define	TM1500m	0x5
#define	TM1750m	0x6
#define	TM2000m	0x7
/**
 * device Lora FEC
 */
#define	REC_OFF		0x0//关闭 FEC 纠错
#define	REC_OPEN	0x1 //打开 FEC 纠错（默认）

/**
 * device Lora TRS_PER 发射功率(大约值)
 */
#define PER30dBm	0x0//（默认）
#define	PER27dBm	0x1
#define	PER24dBm	0x2
#define	PER21dBm	0x3 


#pragma pack(1)

/**
 * device Lora mode
 */
typedef enum
{	
	LORA_M0	 	= 1,
	LORA_M1		= 2,
	LORA_M2 	= 3,
	LORA_M3 	= 4,
	LORA_OTHER 	= 5
}dev_lora_mode_t;


/**
 * device lora cfg
 */
typedef struct
{
	uint8_t SKY_BPS		:3,//无线空中速率（bps）
			TTL_BPS		:3,//TTL 串口速率（bps）
			PBT			:2;//串口校验位
}dev_sped_t;

/**
 * device lora channel
 */

typedef struct
{
	uint8_t  CHAN_NUM		:6,//通信信道，默认 28H（170MHz）
			 CHAN_RES		:2;//保留
}dev_chan_t;

/**
 * device lora option
 */
typedef struct
{
	uint8_t  
			 OPT_RF_TRS_PER		:2,//发射功率(大约值)
			 OPT_FEC 			:1,//FEC 开关
			 OPT_RF_AWAKE_TIME	:3,//无线唤醒时间
			 OPT_IO_DEVICE		:1,//IO 驱动方式（默认 1）
			 OPT_MODBUS			:1;//定点发送使能位（类 MODBUS）
}dev_option_t;

/**
 * device lora cfg
 */

typedef struct
{
	uint8_t MODE;//模块配置模式
	uint8_t ADDH;//模块地址高字节（默认 00H）
	uint8_t ADDL;//模块地址低字节（默认 00H）
	dev_sped_t SPED;//速率参数，包括串口速率和空中速率
	dev_chan_t CHAN;//通信信道，默认 28H
	dev_option_t OPTION;
}dev_lora_t;

/**
 * device time
 */
 typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} dev_time_t;

/**
 * device position
 */
 typedef struct __s_dev_pos
{
	uint32_t latitude;   //纬度
	uint8_t sn;
	uint32_t longitude;  //经度
	uint8_t  we;
} dev_pos_t;

/**
 * device srawdata //六轴陀螺仪
 */
 typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
} dev_sraw_t; 

 typedef struct
{
	dev_sraw_t AccelData;
 	dev_sraw_t MagnData;
} dev_fxos_t; 

/**
 * device level hight
 */
typedef struct
{
  uint16_t altitude;  //海拔整数
  uint8_t point;       //小数位  
  uint8_t pm;	       //正负
}dev_alt_t;
/**
 * device power
 */
typedef struct
{
	uint8_t  lora		:1,
			 gps		:1,
               rf       :1,
			 reserve	:5;
}dev_power_t;

/**
 * device light
 */
typedef struct
{
	uint8_t  state		:7,
			 filcker	:1;
}dev_light_t;

/**
 * device getway order param
 */
#define  BeaconReserveLen 10
typedef struct
{
	uint8_t app_ctrl_order; //上位机控制命令
	uint8_t app_ctrl_flg; //上位机控制命令收到标志位
	uint8_t lora_ctrl_content[BeaconReserveLen];
}dev_gateway_order_t;

/**
 * device prama
 */

typedef struct
{
	uint16_t software;
	uint8_t lora_state;
	uint8_t gateway_ctrl_state; //网关串口数据交互方式
	
	dev_gateway_order_t geteway_data;
	dev_lora_t lora_cfg;
	dev_lora_mode_t lora_mode;
	
	uint8_t move_state; //物体移动状态
	dev_light_t state_light;
	dev_light_t move_light;
	uint8_t  buzzer;
	dev_fxos_t fxos_data;
}dev_param_t;

/**
 * device info
 */
typedef struct
{
    uint8_t id[4];
	dev_lora_t lora_cfg;
	//dont save
    dev_power_t pwr;
	dev_pos_t pos;
	dev_time_t time;
	dev_param_t param;
}device_info_t;


#pragma pack()

void device_info_init(void);

device_info_t *device_info_get(void);

bool_t device_info_set(device_info_t *device_info, bool_t force_update);

uint32_t device_lora_baudrate_info_get(uint8_t buadR);



#endif
