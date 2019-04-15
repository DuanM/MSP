#ifndef __STACK_PRIV_H
#define __STACK_PRIV_H

#define MAX_PKT_LEN			128
/*最大物理帧长度*/
#define MAX_PHY_FRM_SIZE    126
#define MAX_PHY_HEAD_SIZE   (MAX_PHY_FRM_SIZE/9)      	//MAX_PHY_FRM_SIZE除以9
#define PHY_HEAD_SIZE		sizeof(phy_frm_head_t)		//		
#define PHY_TAIL_SIZE		1							//CRC8字段

#define PHY_ID_0			RF_RADIO_0
#define PHY_ID_1			RF_RADIO_1
#define PHY_ID_MAX			RF_RADIO_MAX

#define STACK_EVENT_TX			(1u << 0)
#define STACK_EVENT_RX			(1u << 1)
#define STACK_EVENT_BEACON		(1u << 2)
#define STACK_EVENT_TIME		(1u << 3)

#define STACK_EVENT_TRANS_TX	(1u << 4)
#define STACK_EVENT_TRANS_RX	(1u << 5)

#pragma pack(1)
//phy 头数据格式
typedef struct
{
	uint8_t len:		5,
			need_crc:	1,
			dummy:		2;
}phy_frm_head_t;  

#define MAC_BEACON_FRAME	0 //信标帧
#define MAC_DATA_FRAME		1 //数据帧
#define MAC_CTRL_FRAME		2 //控制帧

//mac 帧格式
typedef struct
{
	struct
	{
		uint8_t type:		2, //帧类型
				attr:		3, //帧标识不同帧类型下各种子类型的属性
				res:		1, //子域标识,当前保留
				hop_no:		2; //转发次数
	}ctrl;
    
	uint16_t seq_no; //发送次数
	
	uint8_t mid_id;
	uint8_t sml_id;
	uint8_t dst_id;	
	uint8_t src_id;
}mac_frm_head_t;

typedef struct
{
	uint16_t head;
	uint16_t payload_len;
	uint8_t type_main;
	uint8_t type_sub;
	uint8_t sender_id[4];
	uint8_t target_id[4];

	uint16_t check_sum;
}s_app_frm_head_t;

#pragma pack()

/////////////mac control////////////////////////
#define REG_DEV_MAX_NUM			16
#define FIX_SLOT_MAX_NUM		8

#pragma pack(1)

//记录超帧指示相关数据
//typedef struct
//{
//	uint8_t superfrm_nbr:	3,//传输设备1s的分为的块
//			superfrm_no:	3,//传输设备工作的间隙
//			slot_cur_max:	2,//小系统内竞争时隙个数
//}mac_superfrm_instr_t;

//信标帧数据格式
typedef struct
{
	mac_frm_head_t head;
	
	uint8_t superfrm_nbr:	2,//传输设备1s的分为的块
			superfrm_no:	3,//传输设备工作的间隙
			slot_cur_max:	2,//小系统内竞争时隙个数
			slot_point_en:	1;
	
	uint8_t cs_bitmap;
	uint8_t point_dev_id;
	
	struct
	{
		uint8_t id;//子级设备id号
		uint8_t state;//子级设备加入网络状态
	} reg_dev[REG_DEV_MAX_NUM];//一个传输设备最多可携带16台设备
	
	uint8_t ack_cs[FIX_SLOT_MAX_NUM];//竞争时隙确认指示,主要是确认收到的信标帧的设备id号
	
	struct
	{
		uint8_t hour;//时
		uint8_t min;//分
		uint8_t sec;//秒
	} rtc_time;
}mac_beacon_frm_t;

#pragma pack()

#define MAC_CTRL_JOINT_REQ		0
#define MAC_CTRL_SLOT_REQ		1
#define MAC_CTRL_HEART_REQ		2
#define MAC_CTRL_ASSIG_REPEAT	3

/////////////stack_analysis/////////////////////
bool_t stack_address_analysis(kbuf_t * kbuf);
/////////////phy////////////////////////////////
void phy_init(void);
kbuf_t *phy_recv_frame(uint8_t phy_id);
bool_t phy_send_frame(uint8_t phy_id, kbuf_t *pbuf, bool_t crc_need);
void phy_rx_state(uint8_t phy_id, bool_t sfd_flag);
void phy_idle_state(uint8_t phy_id);
void phy_tx_state(uint8_t phy_id);
void phy_close(uint8_t phy_id);
void phy_sleep_state(uint8_t phy_id);
////////////mac/////////////////////////////////
void mac_handler(uint16_t event_type);
///////////mac_trans////////////////////////////
void mac_trans_start(void);
void mac_trans_stop(void);
void mac_trans_rx_handler(void);
void mac_trans_tx_handler(void);

#endif