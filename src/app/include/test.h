#ifndef __TSET_H
#define __TSET_H

#define TEST_UART_MAX_LEN		128
#define MAX_FRAME_SEQ_INSIDE	5u


#define UART_MENU_STATE			0
#define UART_CMD_STATE			1

#define STEP_LEVEL0				0x00
#define STEP_LEVEL1				0x10
#define STEP_LEVEL2				0x20
#define STEP_LEVEL2_1			0x21
#define STEP_LEVEL2_2			0x22
#define STEP_LEVEL2_3			0x23
#define STEP_LEVEL2_4			0x24
#define STEP_LEVEL2_5			0x25
#define STEP_LEVEL2_6			0x26
#define STEP_LEVEL2_7			0x27
#define STEP_LEVEL2_8			0x28
#define STEP_LEVEL3				0x30
#define STEP_LEVEL3_1			0x31
#define STEP_LEVEL3_2			0x32
#define STEP_LEVEL3_3			0x33
#define STEP_LEVEL3_4			0x34
#define STEP_LEVEL3_5			0x35
#define STEP_LEVEL3_6			0x36
#define STEP_LEVEL3_7			0x37
#define STEP_LEVEL4				0x40
#define STEP_LEVEL4_1			0x41
#define STEP_LEVEL4_2			0x42
#define STEP_LEVEL4_3			0x43

#define MAX_FRAME_SEQ			5
#define FRAME_REPORT_NUM		100
#define FRAME_MAX_LEN 			126u
#define INSIDE_FRAME_LEN		FRAME_MAX_LEN

#define TSET_DSSS_SCRIPT_LEN				8
#define TEST_DSSS_CMD_MAX					5
#define TEST_DSSS_CMD_TX					0
#define TEST_DSSS_CMD_RX					1
#define TEST_DSSS_CMD_CCA					2
#define TEST_DSSS_CMD_EXIT					3
#define TEST_DSSS_CMD_HELP					4

#define TEST_DSSS_PARA_MAX					10
#define TEST_DSSS_PARA_MODE					0
#define TEST_DSSS_PARA_DISPLAY				1
#define TEST_DSSS_PARA_LEN					2
#define TEST_DSSS_PARA_NUM					3
#define TEST_DSSS_PARA_INTEV				4
#define TEST_DSSS_PARA_SRC					5
#define TEST_DSSS_PARA_DST					6
#define TEST_DSSS_PARA_VALUE				7
#define TEST_DSSS_PARA_RADIO_NUM			8
#define TEST_DSSS_PARA_SYNC_SEND			9


#define TEST_DSSS_RECV_SIG_DEV				0
#define TEST_DSSS_RECV_P2P					1
#define TEST_DSSS_RECV_DISP					2

#define TEST_DSSS_SEND_NORMAL				0
#define TEST_DSSS_SEND_CW					1
#define TEST_DSSS_SEND_FULL					2

#define TEST_DSSS_SEND_TO_IDLE				0
#define TEST_DSSS_SEND_TO_RECV				1


#pragma pack(1)

typedef struct
{
	uint32_t machine_state:			8,
			 uart_recv_date:		1,//获得串口数据
			 uart_state:			1,//串口状态
			 rf_inited:				1,//初始化射频接口
			 rf_state:				2,//射频状态
			 rf_recv_data:			1,//射频收到帧
			 rf_recv_inside_first:	1,//射频收到第一帧
			 rf_send_data:  		1,//射频发送数据
     		 rf_time_out:      		1,//射频超时
     		 rf_index:				1,//射频哪块
			 dummy:					14;
}test_cb_t;

typedef struct
{
	uint8_t cmd;
	uint8_t mode;	
	uint16_t frm_disp;
	uint16_t frm_len;
	uint16_t frm_num;
	uint16_t frm_interv_ms;
	uint8_t src_addr;
	uint8_t dst_addr;
	uint8_t value;
	uint8_t radio_num;
	uint8_t sync_send;
}test_dsss_t;

#define PHY_SIZE			1
#define PAYLOAD_HEAD_SIZE	6

typedef struct
{
	struct
	{
		uint8_t len:		5,
				crc_need:	1,
						:	2;
	}phr;
	struct
	{
		uint8_t dst;
		uint8_t src;
		uint16_t seq;
		uint16_t seq_tot;
		uint8_t data[FRAME_MAX_LEN-PAYLOAD_HEAD_SIZE-PHY_SIZE];
		uint8_t crc_offset;		
	}payload;
	
	uint8_t frm_len;
	uint16_t rssi_lqi;
    
}test_dsss_frame_t;

#pragma pack()

void test_init(void);
void test_trigger(void);
void test_handler(void);


#endif

