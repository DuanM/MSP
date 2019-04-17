#include <platform.h>
#include <test.h>
#include <device.h>
#include <app.h>
#include <stack.h>
#include <sensor.h>

static void app_tx_handle(void);
static void app_dbg_handler(void);
static void app_lora_handler(void);
static void app_cfg_handler(void);
static void app_indicate_callback(void);
static void app_indicate_handler(void);
static void app_key_handler(void);
static void app_aux_handler(void);

static uint8_t app_indicate_id = 0;

void app_handler(uint16_t event_type)
{
	uint16_t object = 0;	
	
	if (event_type & APP_EVENT_DBG_RX)
	{
		object = APP_EVENT_DBG_RX;
		osel_event_clear(app_event_h, &object);
		app_dbg_handler();//串口接收处理
	}
	else if (event_type & APP_EVENT_DBG_TX)
	{
		object = APP_EVENT_DBG_TX;
		osel_event_clear(app_event_h, &object);
		app_tx_handle();
		//串口发送处理
	}
	else if (event_type & APP_EVENT_GPS)
	{
		object = APP_EVENT_GPS;
		osel_event_clear(app_event_h, &object);
		app_gps_poll_handler();//GPS
	}
	else if (event_type & APP_EVENT_LORA_RX)
	{
		object = APP_EVENT_LORA_RX;
		osel_event_clear(app_event_h, &object);
		app_lora_handler();//LORA
	}
    else if (event_type & APP_EVENT_CFG)
	{
		object = APP_EVENT_CFG;
		osel_event_clear(app_event_h, &object);
		app_cfg_handler();//
	}
	else if (event_type & APP_EVENT_INDICATE)
	{
		object = APP_EVENT_INDICATE;
		osel_event_clear(app_event_h, &object);
		app_indicate_handler();		
	}
	else if (event_type & APP_EVENT_KEY)
	{
		object = APP_EVENT_KEY;
		osel_event_clear(app_event_h, &object);
		app_key_handler();		
	}
	else if (event_type & APP_EVENT_AUX)
	{
		object = APP_EVENT_AUX;
		osel_event_clear(app_event_h, &object);
		app_aux_handler();		
	}
	else
	{
		//DBG_TRACE("no this event type!\r\n");
	}
}
static void app_aux_handler(void)
{
	if(DEV_AUX_PIN_VALUE)
	{
		DBG_LORA_PRINTF("P");
	}
	else
	{
		DBG_LORA_PRINTF("D");
	}
}

static void app_tx_handle(void)
{
	OSEL_DECL_CRITICAL();
	device_info_t *device_info = device_info_get();
	kbuf_t *kbuf = PLAT_NULL;
	while(1)
	{
		OSEL_ENTER_CRITICAL();
		kbuf = (kbuf_t *)list_front_get(&app_data_tx_q);
		OSEL_EXIT_CRITICAL();
		
		if (kbuf == PLAT_NULL) return;
		
		if(device_info->param.gateway_ctrl_state)
		{//串口协议数据可以上传网关发送
			hal_uart_send_string(UART_DEBUG, kbuf ->base, kbuf->valid_len);
		}
		
		kbuf = kbuf_free(kbuf);
	}
}

static void app_key_handler(void)
{
	OSEL_DECL_CRITICAL();
	delay_ms(10);//按键消抖时间
	if(!DEV_KEY_PIN_VALUE)
	{
		delay_ms(30);//按键消抖时间
		if(!DEV_KEY_PIN_VALUE)
		{
			device_info_t *device_info = device_info_get();
			uint8_t move_flg = PLAT_FALSE;
			
			OSEL_ENTER_CRITICAL();
			if(device_info->param.move_state == READY_MOVE)
			{//开始移动
				device_info->param.move_state = START_MOVE;
				//行动灯绿，不闪烁,蜂鸣器 停止鸣叫
				device_info->param.move_light.state = GREEN;
				device_info->param.move_light.filcker = PLAT_FALSE;
				device_info->param.buzzer = BUZZER_OFF;
				move_flg = PLAT_TRUE;
			}
			else if(device_info->param.move_state == START_MOVE)
			{//中途停止
				device_info->param.move_state = MIDDLE_STOP_MOVE;
				//行动灯红，不闪烁,蜂鸣器 不鸣叫
				device_info->param.move_light.state = RED;
				device_info->param.move_light.filcker = PLAT_FALSE;
				device_info->param.buzzer = BUZZER_OFF;
				move_flg = PLAT_TRUE;
			}
			else if(device_info->param.move_state == PASSIVITY_MOVE)
			{//目标停止
				device_info->param.move_state = STOP_MOVE;
				//行动灯红，不闪烁,蜂鸣器 停止鸣叫
				device_info->param.move_light.state = RED;
				device_info->param.move_light.filcker = PLAT_FALSE;
				device_info->param.buzzer = BUZZER_OFF;
				move_flg = PLAT_TRUE;
			}
			OSEL_EXIT_CRITICAL();
			
			if(move_flg)
			{
				app_indicate_event();
			}
		}
	}
}

static void app_lora_handler(void)
{
#define APP_SIZE  10
	static uint8_t frm_buff[APP_SIZE];
	static uint8_t frm_len = 0;
	
	static uint8_t frm_flag = NWK_FRM_DEFAULT_FLG;
	static uint8_t fram_head_len = sizeof(dev_lora_t);
	
	uint8_t buf[APP_SIZE];
	uint8_t size = 0;
	size = hal_uart_read(UART_LORA, buf, APP_SIZE);
	if(size)
	{
		for(uint8_t i=0;i<size;i++)
		{
			frm_buff[frm_len] = buf[i];
			frm_len++;
			if(buf[i] == LORA_CFG_MODE)
			{
				mem_clr(frm_buff,APP_SIZE);
				frm_buff[0] = LORA_CFG_MODE;
				frm_flag = NWK_FRM_HEAD_START_FLG;
				frm_len = 1;
			}
			else if((frm_len >= fram_head_len) && (frm_flag == NWK_FRM_HEAD_START_FLG))
			{
				device_info_t *device_info = device_info_get();
				dev_lora_t *dev_lora = (dev_lora_t *)frm_buff;
				DBG_LORA_PRINTF("LR ");
				for(uint8_t i=0;i<frm_len;i++)
				{
					DBG_LORA_PRINTF("%X ",frm_buff[i]);
				}
				DBG_LORA_PRINTF("\r\n");
				
				DBG_LORA_PRINTF("LL ");
				for(uint8_t i=0;i<sizeof(dev_lora_t);i++)
				{
					DBG_LORA_PRINTF("%X ", *(((uint8_t *)&device_info->param.lora_cfg)+i));
				}
				DBG_LORA_PRINTF("\r\n");
				
				if(dev_lora->MODE != device_info->param.lora_cfg.MODE || device_info->param.lora_cfg.ADDH != dev_lora->ADDH || dev_lora->ADDL != device_info->param.lora_cfg.ADDL ||
					device_info->param.lora_cfg.CHAN.CHAN_NUM != dev_lora->CHAN.CHAN_NUM ||  device_info->param.lora_cfg.OPTION.OPT_FEC != dev_lora->OPTION.OPT_FEC ||
					device_info->param.lora_cfg.OPTION.OPT_IO_DEVICE != dev_lora->OPTION.OPT_IO_DEVICE ||  device_info->param.lora_cfg.OPTION.OPT_MODBUS != dev_lora->OPTION.OPT_MODBUS || 
					device_info->param.lora_cfg.OPTION.OPT_RF_AWAKE_TIME != dev_lora->OPTION.OPT_RF_AWAKE_TIME || device_info->param.lora_cfg.OPTION.OPT_RF_TRS_PER != dev_lora->OPTION.OPT_RF_TRS_PER ||
					device_info->param.lora_cfg.SPED.PBT != dev_lora->SPED.PBT ||  device_info->param.lora_cfg.SPED.SKY_BPS != dev_lora->SPED.SKY_BPS || device_info->param.lora_cfg.SPED.TTL_BPS != dev_lora->SPED.TTL_BPS)
				{
					ControlIO_LoraMode(LORA_M0);
					delay_ms(10);
					ControlIO_LoraMode(LORA_M3);
					delay_ms(10);
					hal_uart_send_string(UART_LORA, (uint8_t *)&device_info->param.lora_cfg,sizeof(dev_lora_t));
					DBG_LORA_PRINTF("lora new cfg.\r\n");
				}
				else
				{
					DBG_LORA_PRINTF("lora old cfg.\r\n");
				}
				delay_ms(10); 
				uint8_t buf[3]={0xC4,0xC4,0xC4};
				hal_uart_send_string(UART_LORA, buf,3);
				delay_ms(10); 
				ControlIO_LoraMode(LORA_M0);
				delay_ms(10); 
				uint32_t baudrate = device_lora_baudrate_info_get(device_info->param.lora_cfg.SPED.TTL_BPS);
				hal_uart_init(UART_LORA, baudrate); //lora
				stack_uart_lora_irq_enable_callback();
				
				DBG_LORA_PRINTF("Lora baudrate = %d\r\n",baudrate);
				
				device_info->param.lora_state = PLAT_FALSE;
				
				frm_len = 0;
				mem_clr(frm_buff,APP_SIZE);
				frm_flag = NWK_FRM_DEFAULT_FLG;
			}
			else  if(frm_len >= APP_SIZE)
			{
				frm_len = 0;
				mem_clr(frm_buff,APP_SIZE);
				frm_flag = NWK_FRM_DEFAULT_FLG;
			}
		}
	}
}


//接收数据 处理
static bool_t app_parse(nwk_frm_head_t *frm)
{
	device_info_t *device_info = device_info_get();
	if(frm->id.type_id != GET_DEV_TYPE_ID(device_info->id) || frm->id.group_id != GET_DEV_GROUP_ID(device_info->id) || frm->id.src_id == GET_DEV_ID(device_info->id)|| frm->id.mode_id != DEV_MODE_TERMINAL)
	{
		return PLAT_FALSE;
	}
	return PLAT_TRUE;
}

//DBG
#define CMD_OK		"OK\r\n"
#define CMD_ERROR	"Error\r\n"

//DBG
static void app_dbg_handler(void)
{
	static bool_t global_dbg_flag = PLAT_FALSE;
#define BUF_SIZE    40
	static uint8_t temp_buf[BUF_SIZE];	
	static uint8_t temp_len = 0;
	
	static uint8_t frm_flag = NWK_FRM_DEFAULT_FLG;
	static uint8_t fram_all_len = 0; 
	static uint8_t fram_head_len = sizeof(nwk_frm_head_t);
	
	uint8_t uart_buf[BUF_SIZE];
	uint8_t size = 0;
	uint32_t value = 0;
	
	char_t *p_start = PLAT_NULL;	
	char_t *p_stop = PLAT_NULL; 
	device_info_t *p_device_info = device_info_get();
	
	size = hal_uart_read(UART_DEBUG, uart_buf, BUF_SIZE);
	if(size)
	{
		if(!p_device_info->param.gateway_ctrl_state)
		{//串口 发送 指令模式
			hal_uart_send_string(UART_DEBUG,uart_buf,size);
			
			if(temp_len+size >= BUF_SIZE)
			{
				temp_len = 0;
				mem_clr(temp_buf,BUF_SIZE);
			}
			
			for(uint8_t i=0;i<size;i++)
			{
				temp_buf[temp_len++] = uart_buf[i];
				
				if(uart_buf[i] == '\r')
				{
					goto DBG_PROC;
				}
			}
		}
		else
		{//上位机发送 协议指令模式
			for(uint8_t i=0; i< size; i++)
			{
				temp_buf[temp_len++] = uart_buf[i];
				if(temp_len >= 2)
				{
					//协议数据起始头  
					if((temp_buf[temp_len-2] == NWK_FRM_HEAD0) && (temp_buf[temp_len-1] == NWK_FRM_HEAD1))
					{
						mem_clr(temp_buf,KBUF_SMALL_SIZE);
						temp_buf[0] = NWK_FRM_HEAD0;
						temp_buf[1] = NWK_FRM_HEAD1;
						temp_len = 2;
						frm_flag = NWK_FRM_HEAD_START_FLG;
						fram_all_len = 0;
					}//找到协议头的数据
					else if((temp_len >= fram_head_len) && (frm_flag == NWK_FRM_HEAD_START_FLG))
					{
						nwk_frm_head_t *frm_head = (nwk_frm_head_t *)temp_buf;
						fram_all_len = frm_head->frm_len+fram_head_len;//一帧数据总长度
						frm_flag = NWK_FRM_HEAD_LENGTH_FLG;//帧数据帧头接收完成标志
						
						if(fram_all_len > BUF_SIZE)
						{	//帧头数据错误
							temp_len = 0;
							mem_clr(temp_buf,BUF_SIZE);
							
							fram_all_len = 0;
							frm_flag = NWK_FRM_DEFAULT_FLG;
						}
					}
					else if((frm_flag == NWK_FRM_HEAD_LENGTH_FLG) && (temp_len >= fram_all_len) && (fram_all_len != 0))
					{
						nwk_frm_head_t *frm_head = (nwk_frm_head_t *)temp_buf;
						if(app_parse(frm_head))
						{
							if(frm_head->type == NWK_FRM_DOWN_MTYPE)
							{//自身为 普通模式
								switch(frm_head->stype)
								{
								case NWK_FRM_DOWN_CTRL_STYPE://下发 控制
								case NWK_FRM_DOWN_QUERY_STYPE://下发 查询
								case NWK_FRM_DOWN_CFG_STYPE://下发 配置
									p_device_info->param.geteway_data.app_ctrl_flg = PLAT_TRUE;
									p_device_info->param.geteway_data.app_ctrl_order = NWK_FRM_DOWN_CTRL_STYPE;
									mem_cpy(p_device_info->param.geteway_data.lora_ctrl_content,temp_buf+sizeof(nwk_frm_head_t),BeaconReserveLen);
									break;
								case NWK_FRM_DOWN_GATEWAY_STYPE://下发 释放串口协议控制
									p_device_info->param.gateway_ctrl_state = PLAT_FALSE;
									break;
								}
							}
						}
						temp_len = 0;
						fram_all_len = 0;
						frm_flag = NWK_FRM_DEFAULT_FLG;
						mem_clr(temp_buf,BUF_SIZE);
					}
					else if(temp_len >= BUF_SIZE)
					{
						temp_buf[0] = temp_buf[temp_len-1];
						temp_len = 1;
						fram_all_len = 0;
						frm_flag = NWK_FRM_DEFAULT_FLG;
						mem_clr(temp_buf+temp_len,BUF_SIZE-temp_len);
					}
				}
			}
		}
		return;
	//////////////////////////////////////////////////////////////////////////////////
	DBG_PROC:    
		DBG_SET_LEVEL(DBG_LEVEL_PRINTF);
		DBG_PRINTF("\r\n");
		hal_rtc_block_t  rtc_block;
		p_start =  strstr((char_t*)temp_buf, "test");
		if (p_start)
		{
			//Enter Test
			DBG_PRINTF(CMD_OK);	
			DBG_SET_LEVEL(DBG_LEVEL_PRINTF);
			test_trigger();
			goto QUIT;
		}
		
		p_start =  strstr((char_t*)temp_buf, "reset");
		if (p_start)
		{
			//Enter Reset
			DBG_PRINTF(CMD_OK);	
			delay_ms(100);
			hal_board_reset();
			while(1);
		}
		
		p_start =  strstr((char_t*)temp_buf, "gateway_order_ctrl");
		if (p_start)
		{
			//Enter 网关模式
			p_device_info->param.gateway_ctrl_state = PLAT_TRUE;
			DBG_PRINTF(CMD_OK);
		}
		
	   p_start =  strstr((char_t*)temp_buf, "dbg_app_close");
	   if (p_start)
	   {
		   //特殊 dbg 关闭 
		   DBG_PRINTF(CMD_OK);	
	       APP_DBG_INIT();
		   goto QUIT;
	   }
	   
	   p_start =  strstr((char_t*)temp_buf, "dbg_printf_open");
	   if (p_start)
	   {
		   //全局 dbg 打开
		   global_dbg_flag = PLAT_TRUE;		   
		   DBG_PRINTF(CMD_OK);
		   goto QUIT;
	   }
	   
	   p_start =  strstr((char_t*)temp_buf, "dbg_printf_close");
	   if (p_start)
	   {
		   //全局 dbg 关闭
	       global_dbg_flag = PLAT_FALSE;
		   DBG_PRINTF(CMD_OK);
		   goto QUIT;
	   }
	   p_start =  strstr((char_t*)temp_buf, "gps_dbg_open");
	   if (p_start)
	   {
		   //cancel software tool mode
	       APP_DBG_SET(GPS_PRINTF_LEVEL, PLAT_TRUE);           
		   DBG_PRINTF(CMD_OK);
		   goto QUIT;
	   }
	   p_start =  strstr((char_t*)temp_buf, "lora_dbg_open");
	   if (p_start)
	   {
		   //cancel software tool mode
	       APP_DBG_SET(LORA_PRINTF_LEVEL, PLAT_TRUE);           
		   DBG_PRINTF(CMD_OK);
		   goto QUIT;
	   }
	   p_start =  strstr((char_t*)temp_buf, "fxos_dbg_open");
	   if (p_start)
	   {
		   //cancel software tool mode
	       APP_DBG_SET(FXOS_PRINTF_LEVEL, PLAT_TRUE);           
		   DBG_PRINTF(CMD_OK);
		   goto QUIT;
	   }
	   
		p_start =  strstr((char_t*)temp_buf, "query");
		if (p_start)
		{
			DBG_PRINTF("<**************device inform*************>\r\n");
			DBG_PRINTF("Software version=0x%X\r\n", p_device_info->param.software);
			
			DBG_PRINTF("Device_ID(hex)=%x:%x:%x:%x\r\n", 
					p_device_info->id[0],
					p_device_info->id[1],
					p_device_info->id[2],
					p_device_info->id[3]);
			
			DBG_PRINTF("LORA power=%d\r\n", p_device_info->pwr.lora);
			DBG_PRINTF("GPS power=%d\r\n", p_device_info->pwr.gps);
			
			DBG_PRINTF("LORA Addr = %x%x\r\n",p_device_info->lora_cfg.ADDH,p_device_info->lora_cfg.ADDL);
			
			DBG_PRINTF("<****************************************>\r\n");
			goto QUIT;
		}
		
		p_start =  strstr((char_t*)temp_buf, "ready_move");
		if (p_start)
		{
			//准备移动
			p_device_info->param.move_state = READY_MOVE;
			//行动灯绿，不闪烁,蜂鸣器 停止鸣叫
			p_device_info->param.move_light.state = GREEN;
			p_device_info->param.move_light.filcker = PLAT_TRUE;
			p_device_info->param.buzzer = BUZZER_FLICKER;////BUZZER_OPEN;
			app_indicate_event();
			DBG_PRINTF(CMD_OK);
			goto QUIT;
		}
		
		p_start =  strstr((char_t*)temp_buf, "passivity_move");
		if (p_start)
		{
			//准备移动
			p_device_info->param.move_state = PASSIVITY_MOVE;
			//行动灯绿，不闪烁,蜂鸣器 停止鸣叫
			p_device_info->param.move_light.state = RED;
			p_device_info->param.move_light.filcker = PLAT_TRUE;
			p_device_info->param.buzzer = BUZZER_OPEN;
			app_indicate_event();
			
			DBG_PRINTF(CMD_OK);
			goto QUIT;
		}
		
		p_start =  strstr((char_t*)temp_buf, "dev_cfg");
		if (p_start)
		{
			ControlIO_LoraMode(LORA_M3);
			hal_uart_init(UART_LORA, 9600); //lora
			delay_ms(10);
			p_device_info->lora_cfg.MODE = 0xC0;
			p_device_info->lora_cfg.ADDH = 0x00;
			p_device_info->lora_cfg.ADDL = GET_DEV_ID(p_device_info->id);
			
			p_device_info->lora_cfg.SPED.SKY_BPS = BPS2P4K;
			p_device_info->lora_cfg.SPED.TTL_BPS = TTL9600;
			p_device_info->lora_cfg.SPED.PBT = pbt8N1;
			
			p_device_info->lora_cfg.CHAN.CHAN_NUM = LORA_DEFAULT_CHAN;
			p_device_info->lora_cfg.CHAN.CHAN_RES = LORA_MIN_CHAN;
			
			p_device_info->lora_cfg.OPTION.OPT_MODBUS = PTOP;
			p_device_info->lora_cfg.OPTION.OPT_IO_DEVICE = PushPull;
			p_device_info->lora_cfg.OPTION.OPT_RF_AWAKE_TIME = TM250m;
			p_device_info->lora_cfg.OPTION.OPT_FEC = REC_OPEN;
			p_device_info->lora_cfg.OPTION.OPT_RF_TRS_PER = PER30dBm;
			
			mem_cpy(&p_device_info->param.lora_cfg,&p_device_info->lora_cfg,sizeof(dev_lora_t));
			
			hal_uart_send_string(UART_LORA, (uint8_t *)&p_device_info->lora_cfg,sizeof(dev_lora_t));
			
			//保存flash
			device_info_set(p_device_info,PLAT_TRUE);
			
			hal_uart_init(UART_LORA, device_lora_baudrate_info_get(p_device_info->lora_cfg.SPED.TTL_BPS)); //lora
			ControlIO_LoraMode(LORA_M0);
			delay_ms(10);
			DBG_PRINTF(CMD_OK);
			goto QUIT;
		}
		p_start = strstr((char_t *)temp_buf, "id=");
		if (p_start)
		{   //config device id
			p_start = p_start + strlen("id=");	
			for (uint8_t i=0; i<4; i++)
			{
				p_stop = strstr((char_t *)p_start, ":");
				*p_stop = '\0';
				sscanf(p_start, "%x", &value);
				p_device_info->id[i] = value;
				*p_stop = ' ';
				p_start = p_stop+1;
			}
			DBG_PRINTF("id=%02X:%02X:%02X:%02X\r\n", p_device_info->id[0],p_device_info->id[1],
			p_device_info->id[2],p_device_info->id[3]);
			
			p_device_info->lora_cfg.ADDL = GET_DEV_ID(p_device_info->id);
			
			//保存flash
			device_info_set(p_device_info,PLAT_TRUE);
            
			delay_ms(100);
			hal_board_reset();
			goto QUIT;
		}
		
		p_start =  strstr((char_t*)temp_buf, "time=");
		if (p_start)
		{
			uint8_t times[3];
			p_start = p_start + strlen("time=");  
			for (uint8_t i=0; i<3; i++)
			{
				p_stop = strstr((char_t *)p_start, ":");
				*p_stop = '\0';
				sscanf(p_start, "%d", &value);
				times[i] = value;
				*p_stop = ' ';
				p_start = p_stop+1;
			}

			hal_rtc_get(&rtc_block);
			rtc_block.hour = times[0];
			rtc_block.minute = times[1];
			rtc_block.second = times[2];
			hal_rtc_set(&rtc_block);

			DBG_PRINTF("rtc set time = %d:%d:%d\r\n", rtc_block.hour,rtc_block.minute,rtc_block.second);
			goto QUIT;
		}

		p_start =  strstr((char_t*)temp_buf, "stack_dbg_open");
		if (p_start)
		{
			//cancel software tool mode
			APP_DBG_SET(STACK_PRINTF_LEVEL, PLAT_TRUE);
			DBG_PRINTF(CMD_OK);
			goto QUIT;
		}

		if (p_start == PLAT_NULL)
		{
			DBG_PRINTF(CMD_ERROR);
		}
		QUIT:
		if (global_dbg_flag != PLAT_TRUE)
		{
			DBG_SET_LEVEL(DBG_LEVEL_INFO);
		}
		mem_clr(temp_buf, BUF_SIZE);
		temp_len = 0;
	}
}

static void app_indicate_handler(void)
{
	device_info_t *p_device_info = device_info_get();
	
	ControlIO_MoveLight();
	ControlIO_Buzzer();
	
	if(p_device_info->param.buzzer == BUZZER_FLICKER || p_device_info->param.move_light.filcker == PLAT_TRUE)
	{
		app_indicate_id = hal_timer_free(app_indicate_id);
		app_indicate_id = hal_timer_alloc(APP_TIMEOUT, app_indicate_callback);
	}
	else
	{
		ControlIO_Buzzer_State();
		ControlIO_MoveLight_State();
		app_indicate_id = hal_timer_free(app_indicate_id);
	}
}

static void app_indicate_callback(void)
{
	app_indicate_id = hal_timer_free(app_indicate_id);
	app_indicate_event();
	app_indicate_id = hal_timer_alloc(APP_TIMEOUT, app_indicate_callback);
}

//APP_STACK
void app_cfg_handler(void)
{
	device_info_t *p_device_info = device_info_get();
	p_device_info->param.lora_state = PLAT_TRUE;
	ControlIO_LoraMode(LORA_M3);
	hal_uart_init(UART_LORA, 9600); //lora
	delay_ms(10);
	hal_uart_rx_irq_enable(UART_LORA,1,app_lora_recv_callback);
	
	uint8_t buf[3]={0xC1,0xC1,0xC1};
	hal_uart_send_string(UART_LORA, buf,3);
	
	DBG_LORA_PRINTF("Cfg Sd \r\n");
}












