#ifndef __MAC_H
#define __MAC_H

//////////////////MAC_SLOT/////////////////////////////////////////////
#define TIME_TO_TICK(time_ms)		((uint32_t)((time_ms) * 32.768 + 0.5))

//the following define , all in ms unit
#define T_SUPER_FRM				1000
#define T_CAPTURE_TIME			1666

#define T_FIX_SLOT_MS			200 //时隙时间
#define T_FIX_SLOT_DATA_MS		5 //起始点延时
#define T_BEACON_OFFSET			1

//#if (T_SUPER_FRM <= T_GROUP_MS*MAX_GROUP_NUM)
//#error ("T_SUPER_FRM define time must be more than total used time")
//#endif

typedef enum
{
	MAC_SLOT_NODE_RX_BEACON = 0,	
	MAC_SLOT_NODE_SLOT_FIX0,
	MAC_SLOT_NODE_SLOT_FIX1,
	MAC_SLOT_NODE_SLOT_FIX2,
	MAC_SLOT_NODE_SLOT_FIX3,
	MAC_SLOT_NODE_SLOT_FIX4,
	MAC_SLOT_NODE_SLOT_FIX5,
	MAC_SLOT_NODE_SLOT_FIX6,
	MAC_SLOT_NODE_SLOT_FIX7,
	MAC_SLOT_NODE_SLOT_FIX8,
	MAC_SLOT_NODE_SLOT_FIX9,
	MAC_SLOT_NODE_SLOT_FIX10,
	MAC_SLOT_NODE_SLOT_FIX11,
	MAC_SLOT_NODE_SLOT_FIX12,
	MAC_SLOT_NODE_SLOT_FIX13,
	MAC_SLOT_NODE_SLOT_FIX14,
	MAC_SLOT_NODE_SLOT_ORDER,
	MAC_SLOT_NODE_RE_BEACON,
	MAC_SLOT_NODE_RE_FIX,
	MAC_SLOT_NODE_RE_ORDER,
	MID_SLOT_NODE_SLEEP,
	MAC_SLOT_NODE_MAX	
} mac_slot_node_t;

typedef void(* fn_cb_t)(void);

typedef struct __mac_slot_descr_t
{
	uint8_t type;
	uint32_t duration;
	fn_cb_t fn_cb;	
} mac_slot_t;

typedef struct
{
	bool_t update;
	time_t mark_syn_time;
	time_t attempt_syn_time;
}mac_syn_time_t;

void mac_idle(void);

void set_slot_timer(uint32_t slot_long_ms);
void set_fake_timer(void);
uint32_t sigma_all_slot(uint8_t slot_num);

extern uint8_t m_current_slot;
extern mac_syn_time_t syn_time;
extern htimer_t *m_timer_slot;			//时隙定时器
extern mac_slot_t *p_mac_slot_describe;

/////////////////////////MAC_PIB/////////////////////////////////
#pragma pack(1)


typedef struct
{
	uint8_t id;
	uint8_t group_id;
	uint8_t mode_id;
	uint8_t type_id;
	
	int8_t centre_id;
}mac_pib_t;

#pragma pack()

extern mac_pib_t mac_pib;

void mac_slot_stop(void);

void mac_pib_init(void);

void node_slot_system_init(void);


#endif
