#include <platform.h>
#include <device.h>
#include <stack.h>
#include <stack_priv.h>
#include <mac.h>

extern time_t global_time;

static mac_slot_t node_slot_system[] =
{	
	{MAC_SLOT_NODE_RX_BEACON, 		T_BEACON_OFFSET,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX0,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX1,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX2,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX3,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX4,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX5,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX6,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX7,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX8,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX9,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX10,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX11,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX12,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX13,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_FIX14,	 	T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_SLOT_ORDER,		T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_RE_BEACON,		T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_RE_FIX,			T_FIX_SLOT_MS,				mac_idle},
	{MAC_SLOT_NODE_RE_ORDER,		T_FIX_SLOT_MS,				mac_idle},
	{MID_SLOT_NODE_SLEEP, 			T_FIX_SLOT_MS/*T_SUPER_FRM*/,mac_idle}
};

/*重要参数定时器句柄*/
htimer_t *m_timer_slot = PLAT_NULL;			//时隙定时器

/*定时器回调函数*/
static void timer_slot_cb(void);

/*重要参数运行时隙*/
uint8_t m_current_slot = 0;
mac_slot_t *p_mac_slot_describe = PLAT_NULL;

/*重要参数同步时间*/
mac_syn_time_t syn_time;

void node_slot_system_init(void)
{
	mac_slot_stop();
	for(uint8_t i=0;i<MAC_SLOT_NODE_SLOT_FIX7;i++)
	{
		node_slot_system[MAC_SLOT_NODE_SLOT_FIX0+i].duration = T_FIX_SLOT_MS;
		node_slot_system[MAC_SLOT_NODE_SLOT_FIX0+i].fn_cb = mac_idle;
	}
	p_mac_slot_describe = (mac_slot_t *)node_slot_system;
	mem_clr(&nwk_param,sizeof(nwk_param_t));
}

void mac_idle(void)
{
	DBG_LORA_PRINTF("idle\r\n");
}

//获取 间隙总时间
uint32_t sigma_all_slot(uint8_t slot_num)
{
	uint32_t time_long = 0;
	uint8_t i = 0;
	for (i=0; i<slot_num; i++)
	{
		//提取相应 数组要发送信标指令的 时间间隙长度 
		time_long += p_mac_slot_describe[i].duration;
	}
	return time_long;
}

//信标搜索停止
void mac_slot_stop(void)
{
	/*clear three timer handler*/     
	if (m_timer_slot != PLAT_NULL)
	{
		htimer_cancel(&m_timer_slot);
	}
	p_mac_slot_describe = PLAT_NULL;
	m_current_slot = 0;
	mem_clr(&syn_time, sizeof(mac_syn_time_t));
}

void set_fake_timer(void)
{
	uint16_t object = STACK_EVENT_TIME;
	m_current_slot++;//important	
	osel_event_set(stack_event_h, &object);	
}

//设置信标发送的最长时间
void set_slot_timer(uint32_t slot_long_ms)
{
	time_t fire_time;
	time_t now;
	uint8_t err = 0;
	uint32_t slot_long_tick = 0;
	
	//换算成 定时器 发送这些函数的时间
	slot_long_tick = TIME_TO_TICK(slot_long_ms);
	
	//同步时间的偏移
	fire_time.w = slot_long_tick + syn_time.mark_syn_time.w;
	
	//获取 定时器 现有的时间
	htimer_now(&now);
	
	if (fire_time.w < now.w)
	{//如果 小于 现有的 定时获取时间 表示信标帧同步出错
		DBG_TRACE("fire time < now + 2ms, err\r\n");
		DBG_TRACE("m_slot_current_slot is %d\r\n", m_current_slot);		
		hal_board_reset();
		return;
	}
	
	if ((fire_time.w > now.w) && ((fire_time.w - now.w)>TIME_TO_TICK(T_SUPER_FRM)))
	{//表示 信标函数时间分配 总和 已经超过 1s 同步时间了。
		DBG_TRACE("fire time - now > 1000ms, err\r\n");
		DBG_TRACE("m_slot_current_slot is %d\r\n", m_current_slot);		
		hal_board_reset();
		return;
	}
	
	if (m_timer_slot != PLAT_NULL)
	{
		//取消上一次的 信标定时
		htimer_cancel(&m_timer_slot);
	}
	
	//依次设置相应的下一个需发送信标函数所需的 定时时间
	m_timer_slot = htimer_at(fire_time, timer_slot_cb, (htimer_err_t *)&err);
	if (m_timer_slot == PLAT_NULL)
	{//设置失败
		DBG_TRACE("alloc m_timer_slot timer fail\r\n");
		DBG_TRACE("m_slot_current_slot is %d\r\n", m_current_slot);
		hal_board_reset();	
	}
}


/*
 * timer callback for slot timer
 * 依次执行下一个 发送信标函数
 */
void timer_slot_cb(void)
{
	uint16_t object = STACK_EVENT_TIME;
	m_current_slot++;
	osel_event_set(stack_event_h, &object);
}




