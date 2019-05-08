#ifndef __NWK_H
#define __NWK_H

#include <device.h>

#define LORA_QUERY_MODE 0xC3
#define LORA_CFG_MODE 0xC0
#define NET_ALIVE_CNT 10

#define	DEV_MODE_GENERAL  0x0 //普通节点
#define	DEV_MODE_CENTRE   0x01 //中心汇聚节点
#define	DEV_MODE_TERMINAL 0x02  //上位机设备

#define DEV_NET_DISADD   -1 //无效地址
#define DEV_NET_MAX_NUM   0x04 //网络最多支持设备数
#define INNETMAXNUM DEV_NET_MAX_NUM-1

#define MAC_SLOT_TIMEOUT  20000 //时隙时间
#define MAC_SLOT0_TIMEOUT 300000 //时隙时间
#define MAC_SLOT1_TIMEOUT 200000 //接收时隙

//****************************************************
//uart 通信数据协议头
#define NWK_FRM_HEAD0               0xD6
#define NWK_FRM_HEAD1               0xA4

#define NWK_FRM_DOWN_MTYPE			0x01//下传c
#define NWK_FRM_UP_MTYPE			0x02//上传

#define NWK_FRM_DOWN_BEACON_STYPE	0x00//下发 信标
#define NWK_FRM_DOWN_CTRL_STYPE		0x01//下发 控制
#define NWK_FRM_DOWN_QUERY_STYPE	0x02//下发 查询
#define NWK_FRM_DOWN_CFG_STYPE		0x03//下发 配置

#define NWK_FRM_DOWN_LOCAL_QUERY_STYPE	0x04//下发 查询本地
#define NWK_FRM_DOWN_GATEWAY_STYPE	0xFF//下发 释放串口协议控制


#define NWK_FRM_UP_STATE_STYPE		0x00//上传 状态
#define NWK_FRM_UP_CFGINFO_STYPE	0x01//上传 配置


//****************************************************
//定义串口接收数据帧标志位数据
#define NWK_FRM_DEFAULT_FLG 0x00
#define NWK_FRM_HEAD_START_FLG 0x01
#define NWK_FRM_HEAD_LENGTH_FLG 0x02

#define	NWK_NET_NONE 0
#define	NWK_NET_WORK 1

//灯
#define	OFF 	0
#define	RED 	1
#define	GREEN	2
#define	YELLOW	3
#define	FLICKER	4

//蜂鸣器
#define	BUZZER_OFF	0
#define	BUZZER_OPEN	1
#define	BUZZER_FLICKER	2

#pragma pack(1)


#define INNETALIVETIME 0xf

typedef struct
 {
 	uint8_t InNetState; //在网状态
 	uint8_t InNetId;	//在网ID
}nwk_innetid_t;


typedef struct
 {
 	uint8_t ctrl_type; //是否获取 上位机 的获取配置指令
 	
 	uint8_t InNetNum;
	nwk_innetid_t InNetDev[INNETMAXNUM];
}nwk_param_t;

extern nwk_param_t nwk_param;

#pragma pack()


#pragma pack(1)

typedef enum
{
	NWK_STATUS_START = 0,
	NWK_STATUS_CAPTURE,
	NWK_STATUS_STOP,
	NWK_STATUS_WORK
}nwk_status_t;
	

//协议帧头 ID
typedef struct
{
	uint8_t type_id;
	uint8_t mode_id;
	uint8_t group_id;
	uint8_t src_id;
}nwk_id_t;

//要发送的目的地址
typedef struct
{
	uint8_t ADDH;
	uint8_t ADDL;
	uint8_t CHAN;
}nwk_ptop_head_t;

//协议帧头
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
	nwk_id_t id;
	uint8_t  check;
}nwk_frm_head_t;



//beacon帧
typedef struct
{
	uint8_t InNetNum;
	uint8_t InNetId[INNETMAXNUM];
	uint8_t reserve[BeaconReserveLen];//保留字节
}nwk_beacon_frm_t;

//beacon ctrl 帧
typedef struct
{
	uint8_t dst_id;//目的id
	dev_light_t light;//行动指示灯状态
	uint8_t buzzer;//蜂鸣器状态
	uint8_t reserve[7];//保留字节
}nwk_beacon_ctrl_frm_t;

//beacon query 帧
typedef struct
{
	uint8_t dst_id;//目的id
	uint8_t reserve[9];//保留字节
}nwk_beacon_query_frm_t;

#define LORACDFLEN sizeof(dev_lora_t)

//beacon cfg 帧
typedef struct
{
	uint8_t dst_id;//目的id
	uint8_t cfg[LORACDFLEN];//lora 配置
	uint8_t reserve[3];//保留字节
}nwk_beacon_cfg_frm_t;


typedef struct
{
	dev_lora_t lora_cfg;
	uint8_t lora_mode_num; //lora型号
	uint8_t lora_version_num;//lora版本号
	//uint8_t reserve[2];//保留字节
}nwk_cfg_frm_t;


#define STATERESERVELEN sizeof(nwk_cfg_frm_t)


//上行 状态帧
typedef struct
{
	uint8_t dst_id;//	1	目标ID，0xFF广播
	dev_fxos_t fxos_data;//	12	6轴磁力X/Y/Z
	dev_pos_t pos;// 10	节点ID按入网顺序排列
	dev_light_t light;//	1	行动指示灯状态
	uint8_t buzzer;//	1	蜂鸣器状态
	uint8_t battery;// 1	节点电量（0-9）
	uint8_t reserve[STATERESERVELEN];// 8	保留字节
}nwk_status_frm_t;



#pragma pack()











#endif































