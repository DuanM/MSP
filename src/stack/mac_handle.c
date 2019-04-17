#include <platform.h>
#include <stack.h>
#include <stack_priv.h>
#include <mac.h>


static void mac_tx_handler(void);
static void mac_tx_fix(void);

static void mac_beacon_handler(void);

static void mac_rx_handler(void);
static bool_t mac_parse(nwk_frm_head_t *frm);
static void mac_beacon_frame_proc(uint8_t *frm_buff);
static void mac_ctrl_frame_proc(uint8_t *frm_buff);
static void mac_query_frame_proc(uint8_t *frm_buff);
static void mac_cfg_frame_proc(uint8_t *frm_buff);
static void mac_status_info_send(uint8_t *frm_buff,uint16_t frm_len);
static void mac_time_handler(void);
static void mac_syn_time_init(void);
static void mac_pulse_handle(void);

nwk_param_t nwk_param;

void mac_handler(uint16_t event_type)
{
	uint16_t object;
	
	if (event_type & STACK_EVENT_TX) 
	{
		object = STACK_EVENT_TX;
		osel_event_clear(stack_event_h, &object);
		mac_tx_handler();
	}
	else if (event_type & STACK_EVENT_RX)
	{
		object = STACK_EVENT_RX;
		osel_event_clear(stack_event_h, &object);
		mac_rx_handler();
	}
	else if (event_type & STACK_EVENT_PULSE) 
	{
		object = STACK_EVENT_PULSE;
		osel_event_clear(stack_event_h, &object);
		mac_pulse_handle();
	}
	else if (event_type & STACK_EVENT_TIME) 
	{
		object = STACK_EVENT_TIME;
		osel_event_clear(stack_event_h, &object);
		mac_time_handler();
	}
}

static uint8_t mac_timeout_id = 0;
static void mac_pulse_handle(void)
{
	device_info_t *p_device_info = device_info_get();
    
	if( mac_pib.mode_id == DEV_MODE_CENTRE)
	{
		mac_beacon_handler();
	}
	else if(mac_pib.centre_id != DEV_NET_DISADD)
	{
		for(uint8_t i=1;i<DEV_NET_MAX_NUM;i++)
		{
			if((mac_pib.centre_id+i)%DEV_NET_MAX_NUM == (mac_pib.id%DEV_NET_MAX_NUM))
			{
				mac_timeout_id=hal_timer_free(mac_timeout_id);
				if(i==1)
				{
					mac_timeout_id=hal_timer_alloc(MAC_SLOT0_TIMEOUT*i,mac_tx_fix);
				}
				else
				{
					mac_timeout_id=hal_timer_alloc(MAC_SLOT0_TIMEOUT+MAC_SLOT1_TIMEOUT*(i-1),mac_tx_fix);
				}
			}
		}
	}
	DBG_LORA_PRINTF("G");
}

static void mac_beacon_handler(void)
{
	OSEL_DECL_CRITICAL();
	device_info_t *p_device_info = device_info_get();
	//信标帧时间 间隙
	kbuf_t *kbuf = kbuf_alloc(KBUF_SMALL_TYPE);
	if(kbuf==PLAT_NULL) return;
	nwk_frm_head_t *nwk_frm_head = PLAT_NULL;
	
	kbuf->valid_len = sizeof(nwk_frm_head_t);
	if(p_device_info->param.lora_cfg.OPTION.OPT_MODBUS == PTOP)
	{//点对点模式
		nwk_ptop_head_t *nwk_ptop_head =(nwk_ptop_head_t *)kbuf->base;
		nwk_ptop_head->ADDH = 0xFF;
		nwk_ptop_head->ADDL = 0xFF;
		nwk_ptop_head->CHAN = p_device_info->param.lora_cfg.CHAN.CHAN_NUM;
		kbuf->valid_len += sizeof(nwk_ptop_head_t);
		nwk_frm_head = (nwk_frm_head_t *)(kbuf->base+sizeof(nwk_ptop_head_t));
	}
	else
	{
		nwk_frm_head = (nwk_frm_head_t *)kbuf->base;
	}
	
	nwk_frm_head->head.first = NWK_FRM_HEAD0;
	nwk_frm_head->head.second = NWK_FRM_HEAD1;
	nwk_frm_head->type = NWK_FRM_DOWN_MTYPE;
	
	nwk_frm_head->frm_len = sizeof(nwk_beacon_frm_t);
	mem_cpy(&nwk_frm_head->id,&p_device_info->id,sizeof(nwk_id_t));
	nwk_frm_head->check = 0x0;
	
	kbuf->valid_len += sizeof(nwk_beacon_frm_t);
	nwk_beacon_frm_t *beacon_frm = (nwk_beacon_frm_t *)(((uint8_t *)nwk_frm_head)+sizeof(nwk_frm_head_t));
	beacon_frm->InNetNum = nwk_param.InNetNum;
	
	for(uint8_t i=0;i<INNETMAXNUM;i++)
	{
		if(nwk_param.InNetDev[i].InNetState > 0)
		{
			beacon_frm->InNetId[i] = nwk_param.InNetDev[i].InNetId;
			nwk_param.InNetDev[i].InNetState -= 1;
			if(nwk_param.InNetDev[i].InNetState == 0)
			{
				nwk_param.InNetNum -= 1;
				nwk_param.InNetDev[i].InNetId = 0;
			}
		}
		else
		{
			beacon_frm->InNetId[i] = 0;
		}
	}
	
	if(p_device_info->param.geteway_data.app_ctrl_flg)
	{
		p_device_info->param.geteway_data.app_ctrl_flg = PLAT_FALSE;
		nwk_frm_head->stype = p_device_info->param.geteway_data.app_ctrl_order;
		mem_cpy(beacon_frm->reserve,p_device_info->param.geteway_data.lora_ctrl_content,BeaconReserveLen);
	}
	else
	{
		nwk_frm_head->stype = NWK_FRM_DOWN_BEACON_STYPE;
	}
	
	OSEL_ENTER_CRITICAL();
	list_behind_put(&kbuf->list, &stack_data1_tx_q);
	OSEL_EXIT_CRITICAL();
	
	stack_tx_event();
}

//表示 调用发送信标 处理函数 仅用作信标发起模式下的传输板上。
static void mac_time_handler(void)
{
	uint32_t time_ms = 0;
	static uint32_t pre_time_ms = 0;
	
	//到相应的信标内函数 的 相应的 时间
	time_ms = sigma_all_slot(m_current_slot + 1);
	
	if((pre_time_ms != time_ms) && (time_ms !=0))
	{//时间 
		//if the time of timer change,record the time;
		pre_time_ms = time_ms;
	}
	else
	{
		//next slot during time is zero
		set_fake_timer();//信标 头指针 前移，表示有误，再次处理信标 数组
		return;
	}
	
	if(m_current_slot < MAC_SLOT_NODE_MAX-1)
	{
		set_slot_timer(time_ms);
	}
	
	//处理当前函数
	if(p_mac_slot_describe[m_current_slot].fn_cb != NULL)
	{
		//处理当前的信标函数
		p_mac_slot_describe[m_current_slot].fn_cb();
	}
}

static void mac_tx_handler(void)
{
	OSEL_DECL_CRITICAL();
	device_info_t *p_device_info = device_info_get();
	
	while(p_device_info->param.lora_state)
	{//等待 LORA 为正常工作状态
		delay_ms(20);
	}
	
	kbuf_t *kbuf = PLAT_NULL;
	while(1)
	{
		OSEL_ENTER_CRITICAL();
		kbuf = (kbuf_t *)list_front_get(&stack_data1_tx_q);
		OSEL_EXIT_CRITICAL();
		if (kbuf == PLAT_NULL) return;
		
		hal_uart_send_string(UART_LORA, kbuf ->base, kbuf->valid_len);
		
		kbuf = kbuf_free(kbuf);
		
		DBG_LORA_PRINTF("t");
	}
}

//数据接收处理函数
static void mac_rx_handler(void)
{
#define MAC_SIZE  36
	static uint8_t frm_buff[KBUF_SMALL_TYPE];
	static uint8_t frm_len = 0;
	
	static uint8_t frm_flag = NWK_FRM_DEFAULT_FLG;
	static uint8_t fram_all_len = 0; 
	static uint8_t fram_head_len = sizeof(nwk_frm_head_t);
	
	uint8_t buf[MAC_SIZE];
	uint8_t size = 0;
	
	size = hal_uart_read(UART_LORA, buf, MAC_SIZE);
	if(size)
	{
		for(uint8_t i=0; i< size; i++)
		{
			frm_buff[frm_len] = buf[i];
			frm_len += 1;
			if(2 <= frm_len)
			{
				//协议数据起始头  
				if((frm_buff[frm_len-2] == NWK_FRM_HEAD0) && (frm_buff[frm_len-1] == NWK_FRM_HEAD1))
				{
					mem_clr(frm_buff,KBUF_SMALL_SIZE);
					frm_buff[0] = NWK_FRM_HEAD0;
					frm_buff[1] = NWK_FRM_HEAD1;
					frm_flag = NWK_FRM_HEAD_START_FLG;
					frm_len = 2;
					fram_all_len = 0;
				}//找到协议头的数据
				else if((frm_len >= fram_head_len) && (frm_flag == NWK_FRM_HEAD_START_FLG))
				{
					nwk_frm_head_t *frm_head = (nwk_frm_head_t *)frm_buff;
					fram_all_len = frm_head->frm_len+fram_head_len;//一帧数据总长度
					frm_flag = NWK_FRM_HEAD_LENGTH_FLG;//帧数据帧头接收完成标志
					
					if(fram_all_len > KBUF_SMALL_SIZE)
					{	//帧头数据错误
						frm_len = 0;
						mem_clr(frm_buff,KBUF_SMALL_SIZE);
						
						fram_all_len = 0;
						frm_flag = NWK_FRM_DEFAULT_FLG;
					}
				}
				else if((frm_flag == NWK_FRM_HEAD_LENGTH_FLG) && (frm_len >= fram_all_len) && (fram_all_len != 0))
				{
					nwk_frm_head_t *frm_head = (nwk_frm_head_t *)frm_buff;
					if(mac_parse(frm_head))
					{
						if(frm_head->type == NWK_FRM_DOWN_MTYPE)
						{//自身为 普通模式
							switch(frm_head->stype)
							{
							case NWK_FRM_DOWN_BEACON_STYPE://下发 信标
								mac_beacon_frame_proc(frm_buff);
								break;
							case NWK_FRM_DOWN_CTRL_STYPE://下发 控制
								mac_beacon_frame_proc(frm_buff);
								mac_ctrl_frame_proc(frm_buff);
								break;
							case NWK_FRM_DOWN_QUERY_STYPE://下发 查询
								mac_beacon_frame_proc(frm_buff);
								mac_query_frame_proc(frm_buff);
								break;
							case NWK_FRM_DOWN_CFG_STYPE://下发 配置
								mac_beacon_frame_proc(frm_buff);
								mac_cfg_frame_proc(frm_buff);
								break;
							}
						}
						else if(frm_head->type == NWK_FRM_UP_MTYPE)
						{//自身为 中心节点模式
							switch(frm_head->stype)
							{
							case NWK_FRM_UP_STATE_STYPE://上传 状态
							case NWK_FRM_UP_CFGINFO_STYPE://上传 配置
								mac_status_info_send(frm_buff,frm_len);
								break;
							}
						}
					}
					frm_len = 0;
					mem_clr(frm_buff,KBUF_SMALL_SIZE);
					
					fram_all_len = 0;
					frm_flag = NWK_FRM_DEFAULT_FLG;
				}
				else if(frm_len >= KBUF_SMALL_SIZE)
				{
					frm_buff[0] = frm_buff[frm_len-1];
					frm_len = 1;
					
					mem_clr(frm_buff+frm_len,KBUF_SMALL_SIZE-frm_len);
					
					fram_all_len = 0;
					frm_flag = NWK_FRM_DEFAULT_FLG;
				}
			}
		}
	}
}

//接收数据 处理
static bool_t mac_parse(nwk_frm_head_t *frm)
{
	if(frm->id.type_id != mac_pib.type_id || frm->id.group_id != mac_pib.group_id || frm->id.src_id == mac_pib.id)
	{
		return PLAT_FALSE;
	}
	return PLAT_TRUE;
}

//普通节点 接收beacon 同步时间获取
static void mac_syn_time_init(void)
{
	//获取 接收数据到信标帧时，系统的时间
	htimer_now(&syn_time.attempt_syn_time);
	syn_time.mark_syn_time.w = syn_time.attempt_syn_time.w-TIME_TO_TICK(T_BEACON_OFFSET);
}

//普通节点 接收beacon 入网处理
static void mac_beacon_frame_proc(uint8_t *frm_buff)
{
	nwk_frm_head_t *nwk_frm_head = (nwk_frm_head_t *)frm_buff;
	nwk_beacon_frm_t *nwk_beacon_frm = (nwk_beacon_frm_t *)(frm_buff+sizeof(nwk_frm_head_t));
	uint8_t InNetCnt=0;
	for(InNetCnt=0;InNetCnt<INNETMAXNUM;InNetCnt++)
	{
		if(nwk_beacon_frm->InNetId[InNetCnt] == mac_pib.id)
		{
			mac_pib.centre_id = nwk_frm_head->id.src_id;
			break;
		}
	}
	
	if(InNetCnt==INNETMAXNUM && nwk_beacon_frm->InNetNum < INNETMAXNUM)
	{
		if(mac_pib.id%DEV_NET_MAX_NUM != nwk_frm_head->id.src_id%DEV_NET_MAX_NUM)
		{
			mac_pib.centre_id = nwk_frm_head->id.src_id;
			DBG_LORA_PRINTF("M[%X]",mac_pib.centre_id);
		}
		else
		{
			DBG_LORA_PRINTF("slot conflict\r\n");
		}
	}
}

//普通节点 控制数据函数
static void mac_ctrl_frame_proc(uint8_t *frm_buff)
{
	device_info_t *p_device_info = device_info_get();
	nwk_beacon_frm_t *beacon_frm = (nwk_beacon_frm_t *)(frm_buff+sizeof(nwk_frm_head_t));
	nwk_beacon_ctrl_frm_t *beacon_ctrl_frm = (nwk_beacon_ctrl_frm_t *)beacon_frm->reserve;
	if(beacon_ctrl_frm->dst_id != mac_pib.id && beacon_ctrl_frm->dst_id != 0xff) return;
	mem_cpy(&p_device_info->param.move_light,&beacon_ctrl_frm->light,sizeof(dev_light_t));
	p_device_info->param.buzzer = beacon_ctrl_frm->buzzer;
	stack_indicate_handle();
}

//普通节点 查询数据函数
static void mac_query_frame_proc(uint8_t *frm_buff)
{
	device_info_t *p_device_info = device_info_get();
	nwk_beacon_frm_t *beacon_frm = (nwk_beacon_frm_t *)(frm_buff+sizeof(nwk_frm_head_t));
	nwk_beacon_query_frm_t *beacon_query_frm = (nwk_beacon_query_frm_t *)beacon_frm->reserve;
	if(beacon_query_frm->dst_id != mac_pib.id && beacon_query_frm->dst_id != 0xff) return;
	nwk_param.ctrl_type = PLAT_TRUE;
}

//普通节点 配置数据函数
static void mac_cfg_frame_proc(uint8_t *frm_buff)
{
	device_info_t *device_info = device_info_get();
	
	nwk_beacon_frm_t *beacon_frm = (nwk_beacon_frm_t *)(frm_buff+sizeof(nwk_frm_head_t));
	nwk_beacon_cfg_frm_t *beacon_cfg_frm = (nwk_beacon_cfg_frm_t *)beacon_frm->reserve;
	if(beacon_cfg_frm->dst_id != mac_pib.id && beacon_cfg_frm->dst_id != 0xff) return;
	nwk_param.ctrl_type = PLAT_TRUE;
	dev_lora_t *dev_lora = (dev_lora_t *)beacon_cfg_frm->cfg;
	if(device_info->param.lora_cfg.ADDH != dev_lora->ADDH || dev_lora->ADDL != device_info->param.lora_cfg.ADDL ||
		device_info->param.lora_cfg.CHAN.CHAN_NUM != dev_lora->CHAN.CHAN_NUM ||  device_info->param.lora_cfg.OPTION.OPT_FEC != dev_lora->OPTION.OPT_FEC ||
		device_info->param.lora_cfg.OPTION.OPT_IO_DEVICE != dev_lora->OPTION.OPT_IO_DEVICE ||  device_info->param.lora_cfg.OPTION.OPT_MODBUS != dev_lora->OPTION.OPT_MODBUS || 
		device_info->param.lora_cfg.OPTION.OPT_RF_AWAKE_TIME != dev_lora->OPTION.OPT_RF_AWAKE_TIME || device_info->param.lora_cfg.OPTION.OPT_RF_TRS_PER != dev_lora->OPTION.OPT_RF_TRS_PER ||
		device_info->param.lora_cfg.SPED.PBT != dev_lora->SPED.PBT ||  device_info->param.lora_cfg.SPED.SKY_BPS != dev_lora->SPED.SKY_BPS || device_info->param.lora_cfg.SPED.TTL_BPS != dev_lora->SPED.TTL_BPS)
	{
		mem_cpy(&device_info->param.lora_cfg,beacon_cfg_frm->cfg,sizeof(dev_lora_t));
		if(device_info->param.lora_cfg.MODE == LORA_CFG_MODE)
		{//保存flash
			mem_cpy(&device_info->lora_cfg,beacon_cfg_frm->cfg,sizeof(dev_lora_t));
			device_info_set(device_info,PLAT_TRUE);
		}
		device_info->param.lora_state = PLAT_TRUE;
		stack_config_handle();
	}
}


//中心节点 串口透传 PC
static void mac_status_info_send(uint8_t *frm_buff,uint16_t frm_len)
{//上传接收 入网和接收普通节点的状态和配置数据信息支，仅持点对点
	OSEL_DECL_CRITICAL();
	nwk_frm_head_t *nwk_frm_head = (nwk_frm_head_t *)frm_buff;
	nwk_status_frm_t *status_frm = (nwk_status_frm_t *)(frm_buff+sizeof(nwk_frm_head_t));
	if(status_frm->dst_id != mac_pib.id) return;
	uint8_t i=0;
	for(i=0;i<INNETMAXNUM;i++)
	{
		if(nwk_param.InNetDev[i].InNetId == nwk_frm_head->id.src_id)
		{
			nwk_param.InNetDev[i].InNetState = INNETALIVETIME;
			break;
		}
	}
	//代写
	if(i == INNETMAXNUM && nwk_param.InNetNum < INNETMAXNUM)
	{
		for(i=0;i<INNETMAXNUM;i++)
		{
			if(nwk_param.InNetDev[i].InNetId == 0)
			{
				nwk_param.InNetDev[i].InNetId = nwk_frm_head->id.src_id;
				nwk_param.InNetDev[i].InNetState = INNETALIVETIME;
				nwk_param.InNetNum+=1;
				break;
			}
		}
	}
	DBG_LORA_PRINTF("C[%X]",nwk_frm_head->id.src_id);
	
	kbuf_t *kbuf = kbuf_alloc(KBUF_SMALL_TYPE);
	if(kbuf==PLAT_NULL) return;
	list_t *stack_priv_list = stack_priv_list_get_handle();
	
	//数据发送到APP，Uart 发送给 PC
	mem_cpy(kbuf->base,frm_buff,frm_len);
	kbuf->valid_len = frm_len;
	
	OSEL_ENTER_CRITICAL();
	list_behind_put(&kbuf->list, stack_priv_list);
	OSEL_EXIT_CRITICAL();
	
	stack_priv_list_send_handle();
}

//普通节点 时隙发送
static void mac_tx_fix(void)
{
	OSEL_DECL_CRITICAL();
	
	kbuf_t *kbuf = kbuf_alloc(KBUF_SMALL_TYPE);
	if(kbuf==PLAT_NULL) return;
	nwk_frm_head_t *nwk_frm_head = PLAT_NULL;
	device_info_t *p_device_info = device_info_get();
	kbuf->valid_len = sizeof(nwk_frm_head_t);
	if(p_device_info->lora_cfg.OPTION.OPT_MODBUS == PTOP)
	{//点对点模式
		nwk_ptop_head_t *nwk_ptop_head =(nwk_ptop_head_t *)kbuf->base;
		nwk_ptop_head->ADDH = 0x00;
		nwk_ptop_head->ADDL = mac_pib.centre_id;
		nwk_ptop_head->CHAN = p_device_info->lora_cfg.CHAN.CHAN_NUM;
		kbuf->valid_len += sizeof(nwk_ptop_head_t);
		nwk_frm_head = (nwk_frm_head_t *)(kbuf->base+sizeof(nwk_ptop_head_t));
	}
	else
	{
		nwk_frm_head = (nwk_frm_head_t *)kbuf->base;
	}
	
	nwk_frm_head->head.first = NWK_FRM_HEAD0;
	nwk_frm_head->head.second = NWK_FRM_HEAD1;
	nwk_frm_head->type = NWK_FRM_UP_MTYPE;
	nwk_frm_head->frm_len = sizeof(nwk_status_frm_t);
	mem_cpy(&nwk_frm_head->id,&p_device_info->id,sizeof(nwk_id_t));
	nwk_frm_head->check = 0x0;
	
	kbuf->valid_len += sizeof(nwk_status_frm_t);
	nwk_status_frm_t *nwk_frm = (nwk_status_frm_t *)((uint8_t *)nwk_frm_head+sizeof(nwk_frm_head_t));
	nwk_frm->dst_id = mac_pib.centre_id;
	mem_cpy(&nwk_frm->fxos_data,&p_device_info->param.fxos_data,sizeof(dev_fxos_t));
	mem_cpy(&nwk_frm->pos,&p_device_info->pos,sizeof(dev_pos_t));
	mem_cpy(&nwk_frm->light,&p_device_info->param.move_light,sizeof(dev_light_t));
	nwk_frm->buzzer = p_device_info->param.buzzer;
	
	if(nwk_param.ctrl_type)
	{
		nwk_param.ctrl_type = PLAT_FALSE;//携带自身的配置信息
		nwk_frm_head->stype = NWK_FRM_UP_CFGINFO_STYPE;
		nwk_cfg_frm_t *nwk_cfg_frm = (nwk_cfg_frm_t *)nwk_frm->reserve;
		mem_cpy(&nwk_cfg_frm->lora_cfg,&p_device_info->lora_cfg,sizeof(dev_lora_t));
	}
	else
	{
		nwk_frm_head->stype = NWK_FRM_UP_STATE_STYPE;
	}
	
	//hal_uart_send_string(UART_LORA, kbuf ->base, kbuf->valid_len);
	
	OSEL_ENTER_CRITICAL();
	list_behind_put(&kbuf->list, &stack_data1_tx_q);
	OSEL_EXIT_CRITICAL();
	
	stack_tx_event();
}



