#ifndef __APP_H
#define __APP_H
#include <control_io.h>

#define APP_EVENT_DBG_RX		(1u<<1)
#define APP_EVENT_DBG_TX		(1u<<2)
#define APP_EVENT_GPS			(1u<<3)
#define APP_EVENT_LORA_RX		(1u<<4)
#define APP_EVENT_CFG			(1u<<5)
#define APP_EVENT_INDICATE		(1u<<6)

#define  APP_STATUS	
#define  APP_AUX
#define  APP_HW
//#define  APP_IMG
#define MINUTE 60  //一分钟 60s

//****************************************************
//dsss 通信数据协议头
#define APP_SNR_FRM_HEAD0               0xB3
#define APP_SNR_FRM_HEAD1               0xE4

//协议主类型
#define APP_SNR_FRM_SERV_TYPE			0x01//业务
#define APP_SNR_FRM_MSG_TYPE			0x02//报文
#define APP_SNR_FRM_ORDER_TYPE			0x03//命令
#define APP_SNR_FRM_CTRL_TYPE			0x04//控制
#define APP_SNR_FRM_PC_TYPE				0xFE//串口调试输出


//业务子类型
#define APP_SNR_FRM_SERV_STATUS_STYPE		        0x01//状态
#define APP_SNR_FRM_SERV_INTELL_STYPE		    0x02//情报（原始）
#define APP_SNR_FRM_SERV_APART_STYPE		0x03//区域图像
#define APP_SNR_FRM_SERV_BKGD_STYPE			0x04//背景图像
#define APP_SNR_FRM_SERV_INTELLFS_STYPE			0x05//情报（融合后）

//报文子类型
#define APP_SNR_FRM_MSG_SHR_STYPE		        0x01//短消息

//命令  子类型
#define APP_SNR_FRM_ORDER_CFG_STYPE		        0x01//参数配置
#define APP_SNR_FRM_ORDER_BKGD_STYPE		    0x02//拍摄背景图像
#define APP_SNR_FRM_ORDER_APART_STYPE		    0x03//拍摄区域图像
#define APP_SNR_FRM_ORDER_LENS_STYPE		    0x04//拍摄镜头号设置

//控制帧  子类型
#define APP_SNR_FRM_CTRL_INFO_STYPE		        0x01//业务通知
#define APP_SNR_FRM_CTRL_GET_STYPE		    0x02    //业务获取

//****************************************************
//uart 通信数据协议头
#define APP_SNR_UART_FRM_HEAD0               0xD9
#define APP_SNR_UART_FRM_HEAD1               0xF4

#define APP_SNR_UART_FRM_IMG_TYPE			0x01
#define APP_SNR_UART_FRM_ZDSX_TYPE			0x02
#define APP_SNR_UART_FRM_AUX_TYPE			0x03
#define APP_SNR_UART_FRM_HW_TYPE			0x04


//Image
#define APP_SNR_UART_FRM_BKGD_IMG_STYPE			0x01
#define APP_SNR_UART_FRM_PART_IMG_STYPE			0x02
#define APP_SNR_UART_FRM_CTRL_IMG_STYPE			0x03
#define APP_SNR_UART_FRM_STATUS_IMG_STYPE	    0x04
#define APP_SNR_UART_FRM_TEST_IMG_STYPE			0x05

//ZDSX
#define APP_SNR_UART_FRM_INTELL_DATA_ZDSX_STYPE		0x01
#define APP_SNR_UART_FRM_SZ_TRIGGER_ZDSX_STYPE		0x02
#define APP_SNR_UART_FRM_CTRL_PRAMA_ZDSX_STYPE		0x03
#define APP_SNR_UART_FRM_STATUS_PRAMA_ZDSX_STYPE    0x04


//AUX
#define APP_SNR_UART_FRM_COMPASS_AUX_STYPE		0x81//罗盘信息
#define APP_SNR_UART_FRM_GPS_AUX_STYPE		    0x02//定位
#define APP_SNR_UART_FRM_ALTITUDE_AUX_STYPE	0x03//海拔
#define APP_SNR_UART_FRM_LIGHT_AUX_STYPE	    0x04//光照

#define APP_SNR_UART_FRM_CALIBRATION_AUX_STYPE	    0x10//罗盘校准
#define APP_SNR_UART_FRM_SOFT_MAGNETIC_AUX_STYPE	0x11//罗盘软磁采集

#define APP_SNR_UART_FRM_GPS_PASS_THROUGH_AUX_STYPE	        0x20//GPS uart 透传
#define APP_SNR_UART_FRM_MODULE_CONFIG_INFO_QUERY_AUX_STYPE	    0x23//模块配置状态


//******************************************************************
//上位机应用软件串口交互定义
#define  APP_SOFT_TOOL_APART_IMG 0x03
#define  APP_SOFT_TOOL_BKGRD_IMG 0x04
#define  APP_SOFT_TOOL_POWER_IMG 0x05

#define  APP_SOFT_TOOL_INTELL_ZDSX 0x06
#define  APP_SOFT_TOOL_TRIGGER_ZDSX 0x07
#define  APP_SOFT_TOOL_STATUS_ZDSX 0x08

#define  APP_SOFT_TOOL_ORIEN_AUX 0x09
#define  APP_SOFT_TOOL_POS_AUX   0x0A
#define  APP_SOFT_TOOL_HIGHT_AUX 0x0B
#define  APP_SOFT_TOOL_LIGHT_AUX 0x0C
#define  APP_SOFT_TOOL_DATA_HW   0x0D

#pragma pack(1)

typedef struct
{
	uint8_t mod_id;
	uint8_t mid_id;
	uint8_t sml_id;
	uint8_t dev_id;
}dev_id;

//*****************低功耗状态*******************************
typedef struct
{
	uint16_t	img_ctrl:1,
				zdsx_ctrl:1,
				hw_ctrl:1,
				stack_ctrl:1,
				laser_ctrl:1,
				other_ctrl:11;
}sensor_low_power_ctrl_t;


//低功耗配置
typedef struct
{
	uint8_t low_power_get_ctrl_flg;	//低功耗设备获取指令标志
    sensor_low_power_ctrl_t  sensor_low_power;	//设备灵敏度
	uint32_t low_power_time;//低功耗时间
}app_sensor_low_power_config_t;

//************************************************
//DSSS发送数据帧帧头
//串口数据帧头
typedef struct
{
	struct
	{
		uint8_t first;
		uint8_t second;
	}head;
	
	uint8_t  type;
	uint8_t  stype;
	uint16_t frm_len;
	uint8_t  id[6];
	uint8_t  check;
}app_lora_recv_frm_head_t;


#define LITTLE2BIG_ENDIAN_16B(a) ((a>>8)&0xFF)|((a&0xFF)<<8);
#define BIG2LITTLE_ENDIAN_16B(b) ((b>>8)&0xFF)|((b&0xFF)<<8);

#define APP_TIMEOUT  500000
#define APP_EVENT_TIMEOUT  2000000

extern list_t app_data_tx_q;
//*****************情报*******************************

 
 
//红外数据输出
typedef struct
{
	uint8_t hw_num;
	uint8_t hw_buff[10];
	uint16_t bkgd_noise;
    uint8_t  remain[11];
}app_sensor_hw_frm_t;


//传感器控制帧结构体
typedef struct
{
  uint8_t    Sensitive; //灵敏度
  uint16_t   SetAngle;  //设置角度
  uint16_t   CompassYaw;//姿态角
}app_sensor_control_t;

//****************数据网络dsss发送处理*******************************
typedef struct
{ 
	uint8_t 	data_type; //数据类型
	uint16_t	data_bitch;//批次	
	uint16_t	data_frmlen;//帧长度	
	uint32_t	data_size; //剩余大小  
	uint32_t	data_store;//存储 FRAM 的位置
	uint16_t	data_packets;//数据分的包数
    uint16_t 	data_index;  //分包序号
}app_sensor_data_record_t;

#pragma pack()
//******************************************************


///////////////////////////////////////////////////////////
#define SENSOR_INSIDE_FRM_HEAD	0xF4D9
#define SENSOR_INSIDE_GRAPH		0x01
#define SENSOR_INSIDE_ZDSX		0x02
#define SENSOR_INSIDE_AUX		0x03
#define AUX_REQ_COMPASS_A		0x01
#define AUX_REQ_COMPASS_B		0x81
#define AUX_REQ_POSITION		0x02
#define AUX_REQ_ALTITUDE		0x03
#define AUX_REQ_VERSION			0x23
#define AUX_REQ_ILLUM			0x04
#define AUX_CAL_COMPASS			0x10
#define AUX_CAL_SOFT_COMPASS	0x11
#define AUX_CAL_POSITION		0x20

#pragma pack(1)

typedef struct
{
	uint16_t head;
	uint8_t type;
	uint8_t sub_type;
	uint16_t payload_len;
}app_sensor_inside_frm_head;

typedef struct
{
	uint8_t type;
	uint16_t dir_int;
	uint8_t dir_dec;
	uint8_t pitch_pm;
	uint8_t pitch_int;
    uint8_t pitch_dec;
	uint8_t roll_pm;
	uint8_t roll_int;
	uint8_t roll_dec;
}app_aux_compass_t;

typedef struct
{
	uint32_t lat;
	uint8_t lat_type;
	
	uint32_t lngt;
	uint8_t lngt_type;
}app_aux_postion_t;

typedef struct
{
	uint16_t alt_int;
	uint8_t alt_dec;
	uint8_t alt_pm;
}app_aux_altitude_t;

typedef struct
{
	uint16_t illum;
}app_aux_illumination_t;

typedef struct
{
	uint8_t rate;
}app_aux_cal_compass_t;

typedef struct
{
	uint16_t version;
}app_aux_version_t;

//用以开机延迟
typedef struct
{
uint16_t  app_delay_cnt  :3,
          app_status_cnt :5,
          app_aux_cnt    :2,
          app_hw_cnt     :4,
          idle_other     :2;
}dev_idle_delay_t;

#pragma pack()

///////////////////////////////////////////////////////////
extern osel_task_t *app_task_h;
extern osel_event_t *app_event_h;
extern osel_task_t *init_task_h;

void app_init(void);

void app_lora_recv_callback(void);

void app_indicate_event(void);

void app_image_uart_init(void);

void app_handler(uint16_t event_type);

void laser_power_up(void);//激光上电
void laser_power_down(void);

void app_string_send(kbuf_t *kbuf,uint8_t tybe,uint8_t *buf,uint32_t addr,uint32_t size,uint8_t flag);

void app_gps_poll_callback(void);
void app_gps_poll_handler(void);
void app_gps_proc(char_t *p_data, uint16_t size);

void app_status_begin(void);

void app_sensor_dsss_frm_send_handler(kbuf_t *kbuf,uint8_t frm_mtype,uint8_t frm_stype);

void app_sensor_uart_frm_send_handler(uint8_t frm_mtype,uint8_t frm_stype,uint8_t *frm_content,uint16_t frm_content_len);

#endif



