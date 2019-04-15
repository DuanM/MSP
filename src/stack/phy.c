#include <platform.h>
#include <device.h>
#include <stack.h>
#include <stack_priv.h>
#include <mac.h>

static void phy_0_rxsfd_int_cb(uint16_t cap_time);
static void phy_0_rxok_int_cb(uint16_t reserve);
static void phy_0_txok_int_cb(uint16_t reserve);

static void phy_1_rxsfd_int_cb(uint16_t cap_time);
static void phy_1_rxok_int_cb(uint16_t reserve);
static void phy_1_txok_int_cb(uint16_t reserve);

static volatile bool_t phy_send_ok_flag[PHY_ID_MAX];

static volatile uint16_t phy_rssi_lqi[PHY_ID_MAX];

void phy_init(void)
{
	device_info_t *p_device_info = device_info_get();	
	//device_type_t dev_type = (device_type_t)GET_DEV_MAC(p_device_info->mac);
	//uint8_t id = GET_SML_SYS_ID(p_device_info->id);

	//according to the id
	
	if (p_device_info->pwr.rf)
	{
		if (!hal_rf_init(RF_RADIO_0, DSSS_PN64, 0))
		{
			DBG_TRACE("BB0 failed\r\n");
			hal_rf_deinit(RF_RADIO_0);
			p_device_info->pwr.rf = PLAT_FALSE;
			return;
		}
	}
	
	if (p_device_info->pwr.rf)
	{
        //sleep state
        phy_sleep_state(PHY_ID_0);
		
        //register rf callback function//配置中断待回调函数
        //信标发送和接收中断回调
        hal_rf_reg_int(RF_RADIO_0, HAL_RF_RXSFD_INT, phy_0_rxsfd_int_cb);
		//数据接收中断回调
		hal_rf_reg_int(RF_RADIO_0, HAL_RF_RXOK_INT, phy_0_rxok_int_cb);
		//数据发送中断回调
        hal_rf_reg_int(RF_RADIO_0, HAL_RF_TXOK_INT, phy_0_txok_int_cb);
		
        DBG_TRACE("phy0 init ok\r\n");
        phy_send_ok_flag[PHY_ID_0] = PLAT_FALSE;
	}
}

kbuf_t *phy_recv_frame(uint8_t phy_id)
{
	kbuf_t *kbuf = kbuf_alloc(KBUF_SMALL_TYPE);
	phy_frm_head_t *p_phy_frm_head = PLAT_NULL;

	if(kbuf == PLAT_NULL)
	{
		DBG_TRACE("kbuf alloc failed \r\n");
		hal_rf_flush(phy_id);
		return PLAT_NULL;
	}

	p_phy_frm_head = (phy_frm_head_t *)kbuf->base;	

	phy_rssi_lqi[phy_id] = hal_rf_read_reg(phy_id, BB_CCA);

	hal_rf_read(phy_id, (uint8_t *)p_phy_frm_head, 1);	
	
    if((p_phy_frm_head->len > 0) && (p_phy_frm_head->len <= MAX_PHY_HEAD_SIZE))
    {
    	kbuf->valid_len = p_phy_frm_head->len * 9;

		kbuf->offset = kbuf->base+sizeof(phy_frm_head_t);
		
    	hal_rf_read(phy_id, (uint8_t *)kbuf->offset, kbuf->valid_len-PHY_HEAD_SIZE);
		
		kbuf->priv = (void *)&phy_rssi_lqi[phy_id];		
        
        return kbuf;
	}
    else
    {
    	hal_rf_flush(phy_id);
		kbuf = kbuf_free(kbuf);
        return PLAT_NULL;
    }       
}

bool_t phy_send_frame(uint8_t phy_id, kbuf_t *kbuf, bool_t crc_need)
{
	uint8_t frame_len;
	uint16_t delay_10ms_cnt = 0;
	phy_frm_head_t *p_phy_frm_head = PLAT_NULL;	
	
	if (kbuf == PLAT_NULL)
	{
		return PLAT_FALSE;
	}
	if (kbuf->valid_len > MAX_PHY_FRM_SIZE)
    {
        return PLAT_FALSE;
    }

	hal_rf_set_state(phy_id, HAL_RF_IDLE_STATE);
	p_phy_frm_head = (phy_frm_head_t *)kbuf->base;
	
	//set phy head
    p_phy_frm_head->len = kbuf->valid_len/9;
	if(kbuf->valid_len%9)
	{
		p_phy_frm_head->len += 1;
	}
    
	p_phy_frm_head->need_crc = crc_need;
	//cal frame length	
	frame_len = p_phy_frm_head->len * 9;
    if(crc_need)
    {
        kbuf->base[frame_len-1] = 0x00;
    }
	//send to baseband
	phy_send_ok_flag[phy_id] = PLAT_FALSE;
	hal_rf_cfg_int(phy_id, HAL_RF_TXOK_INT, PLAT_TRUE);
	hal_rf_write(phy_id, kbuf->base, frame_len);
	hal_rf_set_state(phy_id, HAL_RF_TX_STATE);
    
	while(!phy_send_ok_flag[phy_id] && delay_10ms_cnt<16)//增加大约80ms自旋锁保护
    {
        osel_systick_delay(1);//5ms
        delay_10ms_cnt++;
    }	
    
	return phy_send_ok_flag[phy_id];   
}

void phy_rx_state(uint8_t phy_id, bool_t sfd_flag)
{
	hal_rf_set_state(phy_id, HAL_RF_RX_STATE);

	if (sfd_flag == PLAT_TRUE)
	{	//信标捕获中断开启
		hal_rf_cfg_int(phy_id, HAL_RF_RXSFD_INT, PLAT_TRUE);	
	}
	hal_rf_cfg_int(phy_id, HAL_RF_RXOK_INT, PLAT_TRUE);   
}

void phy_idle_state(uint8_t phy_id)
{
	if(hal_rf_get_state(phy_id) != HAL_RF_IDLE_STATE)
    {
		hal_rf_set_state(phy_id, HAL_RF_IDLE_STATE);
		
		hal_rf_cfg_int(phy_id, HAL_RF_RXSFD_INT, PLAT_FALSE);
		hal_rf_cfg_int(phy_id, HAL_RF_TXSFD_INT, PLAT_FALSE);
		hal_rf_cfg_int(phy_id, HAL_RF_RXOK_INT, PLAT_FALSE);	
		hal_rf_cfg_int(phy_id, HAL_RF_TXOK_INT, PLAT_FALSE);
	}
}

void phy_close(uint8_t phy_id)
{
	phy_idle_state(phy_id);
	hal_rf_deinit(phy_id);
}

void phy_sleep_state(uint8_t phy_id)
{
	if(hal_rf_get_state(phy_id) != HAL_RF_OFF_STATE)
	{
		hal_rf_set_state(phy_id, HAL_RF_OFF_STATE);  
	}
}

void phy_0_rxsfd_int_cb(uint16_t cap_time)
{//信标帧 接收和发送成功则会调用次函数

	htimer_hw_t sfd_time = cap_time;
    
	//获取 接收数据到信标帧时，系统的时间
	htimer_now(&syn_time.attempt_syn_time);   
	
    //硬定时超限，会使软定时数据加1，此段为了使时间一定正确
	if (syn_time.attempt_syn_time.s.hw < sfd_time)
	{//防止 后获取时间超时
		syn_time.attempt_syn_time.s.sw--;
	}
	syn_time.attempt_syn_time.s.hw = sfd_time;
	
	//信标帧同步标志
	syn_time.update = PLAT_TRUE;
     
}

void phy_0_rxok_int_cb(uint16_t cap_time)
{

}

void phy_0_txok_int_cb(uint16_t reserve)
{
	phy_send_ok_flag[PHY_ID_0] = PLAT_TRUE; 
}

void phy_1_rxsfd_int_cb(uint16_t reserve)
{
	
}

void phy_1_rxok_int_cb(uint16_t reserve)
{
	
}

void phy_1_txok_int_cb(uint16_t reserve)
{
	phy_send_ok_flag[PHY_ID_1] = PLAT_TRUE;
}




