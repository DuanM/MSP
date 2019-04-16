#include <platform.h>
#include <test.h>
#include <device.h>
#include <control_io.h>
#include <app.h>
#include <sensor.h>
#include <stack.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>

#define PRINTF_STR(x)				hal_uart_printf(UART_DEBUG, x)
#define PRINTF_CHAR(x)				hal_uart_send_char(UART_DEBUG, x)
#define PRINTF_REGISTER_CB(x)		hal_uart_rx_irq_enable(UART_DEBUG, 1, x)

const uint8_t TEST_INTERFACE[] = 
"\r\n\
<***********************************************>\r\n\
Test Software Version 2.2.1(MSP432)\r\n\
1.DSSS Test\r\n\
2.Misc Test\r\n\
3.Configuration\r\n\
4.Reset\r\n\
Please input NUM to select function!\r\n\
<***********************************************>\r\n";

const uint8_t DSSS_INTERFACE[] = 
"\r\nRF DSSS Test\r\n";

const uint8_t MISC_TEST_INTERFACE[] = 
"\r\n\
Misc Test \r\n\
1.Enter UARTA0 Test(aux) \r\n\
2.Enter UARTA1 Test(graph/ip)\r\n\
3.Enter UARTA2 Test(zdsx/bd)\r\n\
4.Enter HW Test(only sensor)\r\n\
5.Enter FRAM Test(only sensor)\r\n\
6.Enter Power Manage Test\r\n\
7.Enter GPIO Test\r\n\
0.Back\r\n\
Please input NUM to select function!\r\n\
<***********************************************>\r\n";

const uint8_t CONFIGURATION_INTERFACE[] = 
"\r\n\
Configuration \r\n\
1.DSSS RF0 Parameter\r\n\
2.Not support\r\n\
3.Set Device Parameter\r\n\
0.Back\r\n\
Please input NUM to select function!\r\n\
<***********************************************>\r\n";

const char_t RF_DSSS_CMD[TEST_DSSS_CMD_MAX][TSET_DSSS_SCRIPT_LEN] =
{
	"tx",
	"rx",
	"cca",
	"exit",
	"?"
};

const char_t RF_DSSS_PARA[TEST_DSSS_PARA_MAX][TSET_DSSS_SCRIPT_LEN] =
{
	"-m=",
	"-p=",
	"-l=",
	"-n=",
	"-t=",
	"-src=",
	"-dst=",
	"-v=",
	"-r=",
	"-s="
};

static osel_event_t *test_event_h = PLAT_NULL;
static uint8_t *uart_buf;
static test_dsss_frame_t *p_rf_frm, *p_rf_frm1, *p_rf_frm2;
static uint16_t test_rf_timeout_id = 0;
static void test_rf_timeout_cb(void);
static volatile test_cb_t test_cb;
static void test_uart_rx_cb(void);
static void test_gpio_handler(void);
static void test_power_manage_handler(void);
static void test_fram_handler(void);
static void test_graph_ip_handler(void);
static void test_zdsx_bd_handler(void);
static void test_hw_handler(void);
static void test_aux_handler(void);
static test_dsss_t *test_dsss_script(void);
static void test_dsss_send(test_dsss_t *p_test_dsss);
static void test_dsss_recv(test_dsss_t *p_test_dsss);

static void test_dsss_handler(void)
{
	uint8_t i;
	test_dsss_t *p_test_dsss = PLAT_NULL;
	
    do
    {
        p_test_dsss = test_dsss_script();
		if (p_test_dsss == PLAT_NULL) DBG_PRINTF("\r\nInput Invalid CMD!\r\n");	
    }while(p_test_dsss == PLAT_NULL);

	switch(p_test_dsss->cmd)
	{
	case TEST_DSSS_CMD_EXIT:
		test_cb.machine_state = STEP_LEVEL0;		
		break;
	case TEST_DSSS_CMD_TX:
		DBG_PRINTF("TX Mode\r\n");		
		test_dsss_send(p_test_dsss);
		DBG_PRINTF("Send Over! -m=%d;-l=%d;-n=%d; -t=%d;-r=%d\r\n", 
					p_test_dsss->mode,
					p_test_dsss->frm_len,
					p_test_dsss->frm_num,
					p_test_dsss->frm_interv_ms,
					p_test_dsss->radio_num);		
		break;
	case TEST_DSSS_CMD_RX:
		DBG_PRINTF("RX Mode\r\n");
		DBG_PRINTF("Receiving frame,press key 'Q' to quit!\r\n");
		test_dsss_recv(p_test_dsss); 
		DBG_PRINTF("Recv Over!\r\n");
		break;
	case TEST_DSSS_CMD_CCA:
		DBG_PRINTF("CCA Mode\r\n");
		DBG_PRINTF("Press key 'Q' to quit!\r\n");
		DBG_PRINTF("CCA Over!\r\n");
		break;
	case TEST_DSSS_CMD_HELP:	
		DBG_PRINTF("\r\nCMD:");
		for (i=0; i<TEST_DSSS_CMD_MAX; i++)
		{			
			DBG_PRINTF("%s ", RF_DSSS_CMD[i]);
		}
		DBG_PRINTF("\r\nParameter:");
		for (i=0; i<TEST_DSSS_PARA_MAX; i++)
		{			
			DBG_PRINTF("%s xxxxx;", RF_DSSS_PARA[i]);
		}
		DBG_PRINTF("\r\n(m):0(sig&send)1(dev&cw)2(disp&full send)");
		DBG_PRINTF("\r\n(p):disp");
		DBG_PRINTF("\r\n(l):packet len");
		DBG_PRINTF("\r\n(n):packet num");
		DBG_PRINTF("\r\n(t):packet intervel");
		DBG_PRINTF("\r\n(v):packet payload");
		DBG_PRINTF("\r\n(r):radio num");
		DBG_PRINTF("\r\n(s):dual radio sync send");
	default:break;
	}
}

static void test_misc_handler(void)
{
    switch(test_cb.machine_state)
	{
	case STEP_LEVEL2_1:
		PRINTF_STR("UARTA0 Test(aux)\r\n");
		test_cb.uart_state = UART_CMD_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_aux_handler();	
		break;
	case STEP_LEVEL2_2:
		PRINTF_STR("UARTA1 Test(graph/ip)\r\n");
		test_cb.uart_state = UART_CMD_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_graph_ip_handler();					
		break;
	case STEP_LEVEL2_3:
		PRINTF_STR("UARTA2 Test(zdsx/bd)\r\n");	
		test_cb.uart_state = UART_CMD_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_zdsx_bd_handler();					
		break;
	case STEP_LEVEL2_4:
		PRINTF_STR("HW Test(only sensor)\r\n");		
		test_cb.uart_state = UART_CMD_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_hw_handler();	
		break;		
	case STEP_LEVEL2_5:
		PRINTF_STR("FRAM Test(only sensor)\r\n");		
		test_cb.uart_state = UART_CMD_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_fram_handler();		
		break;
	case STEP_LEVEL2_6:
		PRINTF_STR("Power Manage Test\r\n");	
		test_cb.uart_state = UART_CMD_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_power_manage_handler();
		break;
	case STEP_LEVEL2_7:
		PRINTF_STR("GPIO Test\r\n");		
		test_cb.uart_state = UART_CMD_STATE;
		test_cb.uart_recv_date = PLAT_FALSE;
		test_gpio_handler();
		break;	
	}
	PRINTF_STR("Test Over\r\n");
}

////////////////////Config//////////////////////////////////
static void config_set_register(uint8_t radio_id)
{
	uint32_t addr, value;
	char_t *str_start = PLAT_NULL;
	char_t *str_stop = PLAT_NULL;
	bool_t res;
	
READ_REG_SCRIPT:
	DBG_PRINTF("Input Register\r\n");
	DBG_PRINTF("Example:[0x00]=0x1234;\r\n");

	mem_set(uart_buf, 0, TEST_UART_MAX_LEN);
	test_cb.uart_recv_date = PLAT_FALSE;
	while(!test_cb.uart_recv_date);
	str_start = (char_t *)uart_buf;
	str_stop = (char_t *)uart_buf;

	str_start = strstr((char_t *)str_start, "exit");
	if (str_start) return;
    else str_start = (char_t *)uart_buf;
    
	while(1)
	{			
		str_start = strstr((char_t *)str_start, "[0x");
		str_stop = strstr((char_t *)str_start, "]=");
		if (str_start && str_stop)
		{
			addr = 0;

			str_start = str_start + strlen("[0x");
			*str_stop = '\0';			
			sscanf(str_start, "%x", &addr);
            *str_stop = ' ';
			str_start = str_stop;
		}
		else
		{
			goto READ_REG_SCRIPT;
		}

		str_start = strstr((char_t *)str_start, "=0x");
		str_stop = strstr((char_t *)str_start, ";");

		if (str_start && str_stop)
		{
			value = 0;
			str_start = str_start + strlen("=0x");
			*str_stop = '\0';
			sscanf(str_start, "%x", &value);
            *str_stop = ' ';
			str_start = str_stop;
		}
		else
		{
			goto READ_REG_SCRIPT;
		}

		res = hal_rf_set_register(radio_id, (uint8_t)addr, (uint16_t *)&value);

		if (res)
		{
			test_cb.rf_inited = PLAT_FALSE;
			DBG_PRINTF("\r\nSet OK-[0x%X]=0x%X!\r\n", addr, value);
		}
	}
}

static void config_set_device(void)
{
	uint32_t value;
	char_t *str_start = PLAT_NULL;
	char_t *str_stop = PLAT_NULL;
	device_info_t *p_device_info = device_info_get();
	
READ_DEVICE_SCRIPT:
	DBG_PRINTF("Input device param [save]\r\n");

	DBG_PRINTF("Ex:id=%02x;%02x;%02x;%02x;\r\n",
		p_device_info->id[0],
		p_device_info->id[1],
		p_device_info->id[2],
		p_device_info->id[3]);

	mem_set(uart_buf, 0, TEST_UART_MAX_LEN);
	test_cb.uart_recv_date = PLAT_FALSE;
	while(!test_cb.uart_recv_date);
	str_start = (char_t *)uart_buf;
	str_stop = (char_t *)uart_buf;

	str_start = strstr((char_t *)str_start, "exit");
	if (str_start) return;
    else str_start = (char_t *)uart_buf;

	str_start = strstr((char_t *)str_start, "mac");
	if (str_start)
	{
		DBG_PRINTF("\r\nmac=");
		str_start = str_start + strlen("mac=");		
		DBG_PRINTF("\r\n");
	}

	str_start = (char_t *)uart_buf;
	str_start = strstr((char_t *)str_start, "id");
	if (str_start)
	{
		DBG_PRINTF("\r\nid=");
		str_start = str_start + strlen("id=");		
		
		for (uint8_t i=0; i<4; i++)
		{
			str_stop = strstr((char_t *)str_start, ";");
			*str_stop = '\0';
			sscanf(str_start, "%x", &value);
			p_device_info->id[i] = value;
        	*str_stop = ' ';
			str_start = str_stop+1;

			DBG_PRINTF("%02X:", p_device_info->id[i]);
		}
		DBG_PRINTF("\r\n");
	}

	str_start = (char_t *)uart_buf;
	str_start = strstr((char_t *)str_start, "save");
	if (str_start)
	{
		device_info_set(p_device_info, PLAT_TRUE);
		DBG_PRINTF("OK\r\n");
	}
	
	goto READ_DEVICE_SCRIPT;
}

static void test_config_handler(void)
{
    uint16_t index, value;
	
	switch (test_cb.machine_state)
	{	
	case STEP_LEVEL3_1://operation RF0 register
		DBG_PRINTF("RF0 Register!\r\n");
		
		for(index = 0; index<REG_NUM; index++)
		{
			hal_rf_get_register(RF_RADIO_0, rf_default_reg_array[index].addr, &value);
			DBG_PRINTF("0-%02d.[0x%02X]=0x%04X;\t[%s]\r\n",
						index,
						rf_default_reg_array[index].addr,
						value,
						rf_default_reg_array[index].name);
		}

		config_set_register(RF_RADIO_0);
		break;
	case STEP_LEVEL3_2://operation RF1 register
		DBG_PRINTF("Not support!\r\n");			
		break;
	case STEP_LEVEL3_3:
		DBG_PRINTF("Set Device Param!\r\n");
		config_set_device();
		break;
	default:break;
	}//case STEP_LEVEL4  end
}

void test_init(void)
{	
	/*create TEST EVENT */	
	test_event_h = osel_event_create(OSEL_EVENT_TYPE_SEM, 0);
	DBG_ASSERT(test_event_h != PLAT_NULL);
}

void test_trigger(void)
{
	if (test_event_h)
	{
		osel_event_set(test_event_h, PLAT_NULL);
	}
}

void test_handler(void)
{
	device_info_t *p_device_info = device_info_get();
	//device_type_t dev_type = (device_type_t)GET_DEV_MAC(p_device_info->mac);
	
	osel_event_wait(test_event_h, OSEL_WAIT_FOREVER);

	DBG_SET_LEVEL(DBG_LEVEL_PRINTF);

	uart_buf = heap_alloc(TEST_UART_MAX_LEN, PLAT_TRUE);
	DBG_ASSERT(uart_buf != PLAT_NULL);

	p_rf_frm1 = heap_alloc(sizeof(test_dsss_frame_t), PLAT_TRUE);
	DBG_ASSERT(p_rf_frm1 != PLAT_NULL);
	p_rf_frm2 = heap_alloc(sizeof(test_dsss_frame_t), PLAT_TRUE);
	DBG_ASSERT(p_rf_frm2 != PLAT_NULL);

	PRINTF_REGISTER_CB(test_uart_rx_cb);
	
	mem_clr((void *)&test_cb, sizeof(test_cb_t));
	
	test_cb.machine_state = STEP_LEVEL0;
	
	test_cb.rf_inited = PLAT_TRUE;
	
	//stack_deinit();
	
	while(1)
	{
		switch(test_cb.machine_state&0xF0)
		{
			case STEP_LEVEL0://Total Application Interface!
				PRINTF_STR((uint8_t *)TEST_INTERFACE);
	        	test_cb.machine_state = STEP_LEVEL0;
				test_cb.uart_recv_date = PLAT_FALSE;
				test_cb.uart_state = UART_MENU_STATE;
				while(!test_cb.uart_recv_date);
	            break;
			case STEP_LEVEL1://RF DSSS TEST Interface!
				if (test_cb.rf_inited == PLAT_FALSE)
				{
					
					
					test_cb.rf_inited = PLAT_TRUE;					
				}				
				PRINTF_STR((uint8_t *)DSSS_INTERFACE);
				test_cb.machine_state = STEP_LEVEL1;
				test_dsss_handler();
				break;
			case STEP_LEVEL2://Misc Test Interface!
				PRINTF_STR((uint8_t *)MISC_TEST_INTERFACE);
	        	test_cb.machine_state = STEP_LEVEL2;
				test_cb.uart_state = UART_MENU_STATE;
				test_cb.uart_recv_date = PLAT_FALSE;					
				while(!test_cb.uart_recv_date);
				test_misc_handler();
				break;
			case STEP_LEVEL3://Test Config Parameter!
				if (test_cb.rf_inited == PLAT_FALSE)
				{
					test_cb.rf_inited = PLAT_TRUE;
				}
				PRINTF_STR((uint8_t *)CONFIGURATION_INTERFACE);	
				test_cb.machine_state = STEP_LEVEL3;
				test_cb.uart_state = UART_MENU_STATE;
				test_cb.uart_recv_date = PLAT_FALSE;					
				while(!test_cb.uart_recv_date);
				test_config_handler();
				break;
			case STEP_LEVEL4://Back to Reset!					
				hal_board_reset();
				break;				
			default:
				break;
		}
	};
}

static void test_uart_rx_cb(void)
{	
	static uint16_t i = 0;
	uint8_t temp;

	while(hal_uart_read(UART_DEBUG, &temp, 1))
	{
		if(test_cb.uart_state == UART_MENU_STATE)
		{
			switch(temp)
			{
				case '0':
					test_cb.machine_state = STEP_LEVEL0;
					break;
				case '1':
				  	if(test_cb.machine_state == STEP_LEVEL0)
					{
					  	test_cb.machine_state = STEP_LEVEL1;
					}
					else if(test_cb.machine_state == STEP_LEVEL2)
					{
						test_cb.machine_state = STEP_LEVEL2_1;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_1;
					}
					break;
				case '2':
					if(test_cb.machine_state == STEP_LEVEL0)
					{
						test_cb.machine_state = STEP_LEVEL2;
					}					
					else if (test_cb.machine_state == STEP_LEVEL2)
					{
						test_cb.machine_state = STEP_LEVEL2_2;
					}
					else if (test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_2;
					}
					break;
				case '3':
					if(test_cb.machine_state == STEP_LEVEL0)
					{
						test_cb.machine_state = STEP_LEVEL3;
					}
					else if(test_cb.machine_state == STEP_LEVEL2)
					{
						test_cb.machine_state = STEP_LEVEL2_3;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_3;
					}
					break;
				case '4':
					if(test_cb.machine_state == STEP_LEVEL0)
					{
						test_cb.machine_state = STEP_LEVEL4;
					}
					else if(test_cb.machine_state == STEP_LEVEL2)
					{
						test_cb.machine_state = STEP_LEVEL2_4;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_4;
					}
					break;
				case '5':
					if(test_cb.machine_state == STEP_LEVEL0)
					{
						//test_cb.machine_state = STEP_LEVEL5;
					}
					else if(test_cb.machine_state == STEP_LEVEL2)
					{
						test_cb.machine_state = STEP_LEVEL2_5;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_5;
					}
					break;
				case '6':
					if(test_cb.machine_state == STEP_LEVEL2)
					{
						test_cb.machine_state = STEP_LEVEL2_6;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_6;
					}
					break;
				case '7':
					if(test_cb.machine_state == STEP_LEVEL2)
					{
						test_cb.machine_state = STEP_LEVEL2_7;
					}
					else if(test_cb.machine_state == STEP_LEVEL3)
					{
						test_cb.machine_state = STEP_LEVEL3_7;
					}
					break;
				case 'q':
				case 'Q':					
					test_cb.rf_state = RF_STATE_SHUTDOWN;
					test_cb.rf_recv_data = PLAT_TRUE;
					test_cb.rf_send_data = PLAT_FALSE;
					break;
				default:
					continue;
			}
			test_cb.uart_recv_date = PLAT_TRUE;
			test_cb.uart_state = UART_CMD_STATE;
		}
		else if (test_cb.uart_state == UART_CMD_STATE)
		{
			if (temp == '\b' && i)
			{
				i--;
			}
			else
			{
				uart_buf[i]=temp;
				if (temp != 'q')
				{
					i++;
				}
			}

			if (temp == 'q')
			{
				osel_task_resume(init_task_h);
			}
			
			PRINTF_CHAR(temp);

			if(uart_buf[i-1]=='\r'||(i>TEST_UART_MAX_LEN))
			{					
				test_cb.uart_recv_date = PLAT_TRUE;
				uart_buf[i]='\0';
				i=0;
			}
		}
	}
}

static test_dsss_t *test_dsss_script(void)
{
	static test_dsss_t test_dsss;
	char_t *start_str = PLAT_NULL;
	char_t *stop_str = PLAT_NULL;
	uint32_t value;	
	uint8_t i;

	test_cb.uart_state = UART_CMD_STATE;
	test_cb.uart_recv_date = PLAT_FALSE;
	while(!test_cb.uart_recv_date);
	test_cb.uart_state = UART_MENU_STATE;
    
    mem_clr(&test_dsss, sizeof(test_dsss_t));

	for (i=0; i<TEST_DSSS_CMD_MAX; i++)
	{
		start_str = (char_t *)uart_buf;
		start_str = strstr(start_str, RF_DSSS_CMD[i]);
		if (start_str)
		{
			test_dsss.cmd = i;
			break;
		}		
	}

	if (start_str == PLAT_NULL) return PLAT_NULL;	
	
	for (i=0; i<TEST_DSSS_PARA_MAX; i++)
	{
		start_str = (char_t *)uart_buf;
		start_str = strstr(start_str, RF_DSSS_PARA[i]);
        if (start_str) stop_str = strstr(start_str, ";");
		if (start_str && stop_str)
		{
			start_str = start_str + strlen(RF_DSSS_PARA[i]);
			*stop_str = '\0';
			sscanf(start_str, "%d", &value);
            *stop_str = ' ';
			switch(i)
			{
				case TEST_DSSS_PARA_MODE:
					if ((value&0xf)<=2) test_dsss.mode = value;
					else test_dsss.mode = 0;
					break;
				case TEST_DSSS_PARA_DISPLAY:
					test_dsss.frm_disp = value;
					break;
				case TEST_DSSS_PARA_LEN:
					if (value <= FRAME_MAX_LEN) 
					{
						if (value%9)
						{
							value = value/9+1;
							value = value*9;
						}						
						test_dsss.frm_len = value;
					}
					else 
					{
						test_dsss.frm_len = FRAME_MAX_LEN;
					}
					break;
				case TEST_DSSS_PARA_NUM:
					test_dsss.frm_num = value;
					break;
				case TEST_DSSS_PARA_INTEV:
					test_dsss.frm_interv_ms = value;
					break;
				case TEST_DSSS_PARA_SRC:
					test_dsss.src_addr = value;
					break;
				case TEST_DSSS_PARA_DST:
					test_dsss.dst_addr = value;
					break;
				case TEST_DSSS_PARA_VALUE:
					test_dsss.value = value;
					break;
				case TEST_DSSS_PARA_RADIO_NUM:
					if (value < RF_RADIO_MAX)
					{
						test_dsss.radio_num = value;
					}
					else
					{
						test_dsss.radio_num = 0;
					}
					break;
				case TEST_DSSS_PARA_SYNC_SEND:
					test_dsss.sync_send = value;
					break;
			}
		}
		else
		{
			switch(i)
			{
				case TEST_DSSS_PARA_MODE:
					test_dsss.mode = 0;
					break;
				case TEST_DSSS_PARA_DISPLAY:
					test_dsss.frm_disp = 36;
					break;
				case TEST_DSSS_PARA_LEN:
					test_dsss.frm_len = FRAME_MAX_LEN;
					break;
				case TEST_DSSS_PARA_NUM:
					test_dsss.frm_num = 1500;
					break;
				case TEST_DSSS_PARA_INTEV:
					test_dsss.frm_interv_ms = 5;
					break;
				case TEST_DSSS_PARA_SRC:
					test_dsss.src_addr = 0xff;
					break;
				case TEST_DSSS_PARA_DST:
					test_dsss.dst_addr = 0xff;
					break;
				case TEST_DSSS_PARA_VALUE:
					test_dsss.value = 0xa5;
					break;
				case TEST_DSSS_PARA_RADIO_NUM:
					test_dsss.radio_num = 0;
				case TEST_DSSS_PARA_SYNC_SEND:
					test_dsss.sync_send = 0;
					break;
			}
		}
	}
	return &test_dsss;
}

static void test_dsss_send_cb(uint16_t cap_time)
{
	test_cb.rf_send_data = PLAT_FALSE;
}

static void test_dsss_send(test_dsss_t *p_test_dsss)
{
	uint16_t i=0;
	uint8_t radio_id = p_test_dsss->radio_num;
	uint8_t radio_sync_id;
	
	hal_rf_set_state(radio_id, HAL_RF_IDLE_STATE);

	hal_rf_unreg_int(radio_id, HAL_RF_TXOK_INT);
	hal_rf_reg_int(radio_id, HAL_RF_TXOK_INT, test_dsss_send_cb);

	if (p_test_dsss->sync_send)
	{
		if (radio_id == RF_RADIO_0) radio_sync_id = RF_RADIO_1;
		else radio_sync_id = RF_RADIO_0;

		hal_rf_set_state(radio_sync_id, HAL_RF_CON_TX_STATE);

		DBG_PRINTF("Continue Send by RF:%d\r\n", radio_sync_id);
	}

	switch(p_test_dsss->mode&0xf)//
	{
		case TEST_DSSS_SEND_NORMAL:
			//p_test_dsss->frm_len is already for 9 times
			p_rf_frm1->phr.len = p_test_dsss->frm_len/9;
			p_rf_frm1->phr.crc_need = 0;
			p_rf_frm1->payload.dst = p_test_dsss->dst_addr;
			p_rf_frm1->payload.src = p_test_dsss->src_addr;
			p_rf_frm1->payload.seq = 0;
			p_rf_frm1->payload.seq_tot = p_test_dsss->frm_num - 1;//0开始
			for(i=0; i<p_test_dsss->frm_len - PAYLOAD_HEAD_SIZE - PHY_SIZE - 1; i++)
			{
				p_rf_frm1->payload.data[i] = p_test_dsss->value;
			}
			p_rf_frm1->payload.crc_offset = p_test_dsss->frm_len-PHY_SIZE-PAYLOAD_HEAD_SIZE-1;

			test_cb.rf_state = RF_STATE_TX;

			for (i=0; i<p_test_dsss->frm_num && test_cb.rf_state == RF_STATE_TX; i++)
			{
				test_cb.rf_send_data = PLAT_TRUE;				
				p_rf_frm1->payload.data[p_rf_frm1->payload.crc_offset] 
								= crc8_tab((uint8_t *)p_rf_frm1, 0, p_test_dsss->frm_len-1);

				hal_rf_cfg_int(radio_id, HAL_RF_TXOK_INT, PLAT_TRUE);
                
				hal_rf_write(radio_id, (uint8_t *)p_rf_frm1, p_test_dsss->frm_len);
				hal_rf_set_state(radio_id, HAL_RF_TX_STATE);
                    
				while(test_cb.rf_send_data == PLAT_TRUE);                    
				test_cb.rf_send_data = PLAT_FALSE;
				
				p_rf_frm1->payload.seq++;

				DBG_PRINTF("Send:%u\r\n", p_rf_frm1->payload.seq);

				delay_ms(p_test_dsss->frm_interv_ms);
			}			
			break;
		case TEST_DSSS_SEND_CW:
			DBG_PRINTF("RF Carrier Wave Test\r\n");
			hal_rf_set_state(radio_id, HAL_RF_CON_TX_STATE);
			test_cb.rf_state = RF_STATE_TX;
			while(test_cb.rf_state == RF_STATE_TX);			
			break;
		case TEST_DSSS_SEND_FULL:
			DBG_PRINTF("RF Send Full Test not support\r\n");			
			break;
	}
	
	hal_rf_unreg_int(radio_id, HAL_RF_TXOK_INT);
	
	hal_rf_set_state(radio_id, HAL_RF_OFF_STATE);

	if (p_test_dsss->sync_send)
	{
		hal_rf_set_state(radio_sync_id, HAL_RF_OFF_STATE);
		DBG_PRINTF("Continue Send Stop by RF:%d\r\n", radio_sync_id);
	}
}

static uint8_t radio_rx_id = 0;
static test_dsss_frame_t *p_recv_rf_frm = PLAT_NULL;

static void test_dsss_recv_cb(uint16_t cap_time)
{    
	if (p_rf_frm == p_rf_frm1)
	{
		p_recv_rf_frm = p_rf_frm2;
	}
	else
	{
		p_recv_rf_frm = p_rf_frm1;
	}

	p_recv_rf_frm->rssi_lqi = hal_rf_read_reg(radio_rx_id, BB_CCA);

	hal_rf_read(radio_rx_id, (uint8_t *)&p_recv_rf_frm->phr, 1);	
	
    if(((p_recv_rf_frm->phr.len) > 0) && ((p_recv_rf_frm->phr.len) < 15))
    {
    	p_recv_rf_frm->frm_len = p_recv_rf_frm->phr.len * 9;
		
    	hal_rf_read(radio_rx_id, (uint8_t *)&p_recv_rf_frm->payload, p_recv_rf_frm->frm_len - 1);
		
		p_recv_rf_frm->payload.crc_offset = p_recv_rf_frm->frm_len - PHY_SIZE - PAYLOAD_HEAD_SIZE - 1;

		test_cb.rf_recv_data = PLAT_TRUE;
	}
    else
    {
    	p_recv_rf_frm = PLAT_NULL;
        hal_rf_flush(radio_rx_id);
    }       
}

static void test_dsss_recv(test_dsss_t *p_test_dsss)
{
	volatile uint32_t total_frame_count = 0;
	volatile uint32_t loss_frame_count = 0;
	volatile uint32_t err_frame_count = 0;
	volatile uint32_t recv_frame_count = 0;
	volatile uint16_t seq_num = 0;
	volatile int16_t prev_seq_num = 0;
	volatile uint16_t total_seq_num = 0;
	int16_t index;
	
	uint8_t radio_sync_id = 0;
	uint8_t radio_id = p_test_dsss->radio_num;	

	radio_rx_id = radio_id;
	
	hal_rf_set_state(radio_id, HAL_RF_IDLE_STATE);

	hal_rf_unreg_int(radio_id, HAL_RF_RXOK_INT);
	hal_rf_reg_int(radio_id, HAL_RF_RXOK_INT, test_dsss_recv_cb);	
	
	switch (p_test_dsss->mode)
	{
		case TEST_DSSS_RECV_SIG_DEV: DBG_PRINTF("Signal Device Test!\r\n");
			index = 0;			
			break;
		case TEST_DSSS_RECV_P2P: DBG_PRINTF("P2P Test!\r\n");
			index = -1;
			break;
		case TEST_DSSS_RECV_DISP: DBG_PRINTF("Display!\r\n");
			break;
		default:
			hal_rf_set_state(radio_id, HAL_RF_OFF_STATE);
			hal_rf_unreg_int(radio_id, HAL_RF_RXOK_INT);
			return;
	}

	if (p_test_dsss->sync_send)
	{
		if (radio_id == RF_RADIO_0) radio_sync_id = RF_RADIO_1;
		else radio_sync_id = RF_RADIO_0;

		hal_rf_set_state(radio_sync_id, HAL_RF_CON_TX_STATE);

		DBG_PRINTF("Continue Send by RF:%d\r\n", radio_sync_id);
	}
	
	//test_rf_timeout_id = hal_timer_alloc(REFRESH_TIME_MS*1000, test_rf_timeout_cb);	
	//test_cb.rf_time_out = PLAT_TRUE;
	
	test_cb.rf_state = RF_STATE_RX;
	test_cb.rf_recv_data = PLAT_FALSE;	
	test_cb.rf_recv_inside_first = PLAT_TRUE;
	////////////////////////////////////////////////
	
    hal_rf_set_state(radio_id, HAL_RF_RX_STATE);
	
	hal_rf_cfg_int(radio_id, HAL_RF_RXOK_INT, PLAT_TRUE);
	
	while(test_cb.rf_state == RF_STATE_RX)
	{		
		while(!test_cb.rf_recv_data);
		if (test_cb.rf_state != RF_STATE_RX) goto RECV_OVER;
		
		//test_cb.rf_time_out = PLAT_FALSE;

		if (p_recv_rf_frm)
		{
			p_rf_frm = p_recv_rf_frm;
		}
		
		test_cb.rf_recv_data = PLAT_FALSE;
		
		switch (p_test_dsss->mode)
		{
			case TEST_DSSS_RECV_SIG_DEV:
				seq_num = p_rf_frm->payload.seq;
				total_seq_num = p_rf_frm->payload.seq_tot;				
				
				if ((seq_num>=1)&&(seq_num<=MAX_FRAME_SEQ_INSIDE))
				{
					if ((seq_num - index)==1)
					{
						total_frame_count++;								
						
                        DBG_PRINTF("Received:%u\tL:%03x\tP:%04x\r\n", 
									total_frame_count, 
									RF_RSSI_LEVEL(p_rf_frm->rssi_lqi), 
									RF_RSSI_LQI(p_rf_frm->rssi_lqi));
					}
					else if (!test_cb.rf_recv_inside_first)
					{
						if(seq_num > index)
						{
							loss_frame_count += seq_num - 1 - index;
							PRINTF_STR("Loss Frame!\r\n");
						}
						else
						{
							loss_frame_count += seq_num + 4 - index;
							PRINTF_STR("Loss Frame!\r\n");
						}
					}
					index = (seq_num)%MAX_FRAME_SEQ_INSIDE;
				}
				else
				{
					DBG_PRINTF("Error Frame\tL:%03x\tP:%04x\r\n",
								RF_RSSI_LEVEL(p_rf_frm->rssi_lqi), 
								RF_RSSI_LQI(p_rf_frm->rssi_lqi));
                    total_frame_count++;
					err_frame_count++;							
				}
				test_cb.rf_recv_inside_first = PLAT_FALSE;
				p_rf_frm = PLAT_NULL;
				break;
			case TEST_DSSS_RECV_P2P:
				seq_num = p_rf_frm->payload.seq;
				total_seq_num = p_rf_frm->payload.seq_tot + 1;
				//判断crc是否正确        
				if(p_rf_frm->payload.data[p_rf_frm->payload.crc_offset]
                   			== crc8_tab((uint8_t *)p_rf_frm, 0, p_rf_frm->frm_len-1))
				{
					DBG_PRINTF("Received:%u\tL:%03x\tP:%04x\r\n", 
								seq_num, 
								RF_RSSI_LEVEL(p_rf_frm->rssi_lqi), 
								RF_RSSI_LQI(p_rf_frm->rssi_lqi));
					
					if((seq_num - index)>1)
					{
						loss_frame_count += seq_num - index - 1;
						PRINTF_STR("Loss Frame!\r\n");							
					}
					index = seq_num;
					
					if(index + 1 == total_seq_num)
					{
						test_cb.rf_state = RF_STATE_SHUTDOWN;
					}
				}
				else
				{
					DBG_PRINTF("Error Frame\tL:%03x\tP:%04x\r\n",
								RF_RSSI_LEVEL(p_rf_frm->rssi_lqi), 
								RF_RSSI_LQI(p_rf_frm->rssi_lqi));
                   	err_frame_count++;
				}
				p_rf_frm = PLAT_NULL;
				break;
			case TEST_DSSS_RECV_DISP:
				hal_uart_send_string(UART_DEBUG, (uint8_t *)p_rf_frm, p_test_dsss->frm_disp);
				p_rf_frm = PLAT_NULL;
				break;
		}
	}
	
RECV_OVER:
	test_rf_timeout_id = hal_timer_free(test_rf_timeout_id);
	test_cb.rf_time_out = PLAT_FALSE;

	if (p_test_dsss->mode == TEST_DSSS_RECV_SIG_DEV)
	{
		total_frame_count += loss_frame_count;
					
		DBG_PRINTF("Total Frame Count=%lu\r\n", total_frame_count);
		
		DBG_PRINTF("Loss Frame Count=%lu\r\n", loss_frame_count);
		
		if(total_frame_count)
		{
			DBG_PRINTF("Loss Frame Rate=%.4f%%\r\n", (fp32_t)((fp32_t)loss_frame_count/(fp32_t)total_frame_count)*100);
		}
	}
	else if (p_test_dsss->mode == TEST_DSSS_RECV_P2P)
	{
		//因为lost帧数需要下一帧才能够检测出来，而error是当场检测，故存在一直error的情况，而lost为0
		if(loss_frame_count == 0 && err_frame_count != 0)
		{
			loss_frame_count = 0;
			total_frame_count = err_frame_count;
		}
		else
		{
			if (loss_frame_count >= err_frame_count)
			{
				loss_frame_count = loss_frame_count - err_frame_count;
			}
			total_frame_count = total_seq_num;					
		}
		
		DBG_PRINTF("Total Frame Count=%u\r\n", total_frame_count);				
		
		DBG_PRINTF("Error Frame Count=%u\r\n", err_frame_count);
		
		DBG_PRINTF("Loss Frame Count=%u\r\n", loss_frame_count);
		
		if(total_frame_count)
		{
			DBG_PRINTF("Loss Frame Rate=%.4f%%\r\n", ((fp32_t)((fp32_t)loss_frame_count/(fp32_t)total_frame_count))*100);
		}
	}
	
	hal_rf_cfg_int(radio_id, HAL_RF_RXOK_INT, PLAT_FALSE);

	hal_rf_unreg_int(radio_id, HAL_RF_RXOK_INT);	
	
	hal_rf_set_state(radio_id, HAL_RF_OFF_STATE);

	if (p_test_dsss->sync_send)
	{
		hal_rf_set_state(radio_sync_id, HAL_RF_OFF_STATE);
		DBG_PRINTF("Continue Send Stop by RF:%d\r\n", radio_sync_id);
	}
}



////////////////////////////////////////////////////////////
static void test_gpio_handler(void)
{
	char_t *start_str = PLAT_NULL;
	char_t *mark_str = PLAT_NULL;
	char_t *stop_str = PLAT_NULL;
	uint32_t value = 0;
	uint8_t pin_num = 0;
	uint32_t port_num = 0;
	
	PRINTF_STR("N:0 output low\r\n");
	PRINTF_STR("N:1 output high\r\n");
	PRINTF_STR("N:2 input state\r\n");
	PRINTF_STR("N:3 input read\r\n");

	while(1)
	{
		if (!test_cb.uart_recv_date) continue;
		
		test_cb.uart_recv_date = PLAT_FALSE;			
	
		if (strstr(uart_buf, "exit"))
		{
			return;
		}
		
        start_str = strstr(uart_buf, "P");
        if (start_str)
        {
            mark_str = start_str + 1;
            stop_str = start_str + 2;
            
            if (*stop_str != '.') continue;

            *stop_str = '\0';					
            sscanf(mark_str, "%d", &pin_num);
                                                
            mark_str = stop_str + 1;
            stop_str = stop_str + 2;

            if (*stop_str != '=') continue;
            
            *stop_str = '\0';
            sscanf(mark_str, "%d", &port_num);

            mark_str = stop_str + 1;
            stop_str = stop_str + 2;
            *stop_str = '\0';

            sscanf(mark_str, "%d", &value);               

            if (value == 0)
            {
                GPIO_setAsOutputPin(pin_num, (1u<<port_num));
                GPIO_setOutputLowOnPin(pin_num, (1u<<port_num));
				DBG_PRINTF("POUT%d.%d=%d\r\n", pin_num, port_num, value);
            }
            else if (value == 1)
            {
                GPIO_setAsOutputPin(pin_num, (1u<<port_num));
                GPIO_setOutputHighOnPin(pin_num, (1u<<port_num));
				DBG_PRINTF("POUT%d.%d=%d\r\n", pin_num, port_num, value);
            }
            else if (value == 2)
            {
            	GPIO_setAsInputPin(pin_num, (1u<<port_num));
				DBG_PRINTF("P%d.%d=%d INPUT OK!\r\n", pin_num, port_num, value);
            }
			else if (value == 3)
			{
				value = GPIO_getInputPinValue(pin_num, (1u<<port_num));
				DBG_PRINTF("PIN%d.%d=%d\r\n", pin_num, port_num, value);
			}           
        }
    }
}
///////////////////////////////////////////////////////
static void test_power_manage_handler(void)
{
	device_info_t *p_device_info = device_info_get();
	uint8_t bitmap;


	while(1)
	{
		if (!test_cb.uart_recv_date) continue;
		
		test_cb.uart_recv_date = PLAT_FALSE;			
	
		if (strstr(uart_buf, "exit"))
		{
			return;
		}

      	if (strstr(uart_buf, "save"))
		{
			device_info_set(p_device_info, PLAT_TRUE);
			DBG_PRINTF("OK\r\n");
			continue;
		}
    
    }
}

static bool_t fram_state = PLAT_TRUE;

static void graph_ctrl_fram_uart_rx_cb(void)
{
	uint8_t buffer[8], len;

	do
	{
		len = hal_uart_read(UART_LORA, buffer, 8);

		if (len)
		{
			if (buffer[0]=='H')
			{					
				hal_fram_get_spi_cs(PLAT_TRUE);
			}
			else if (buffer[0]=='L')
			{
				hal_fram_get_spi_cs(PLAT_FALSE);
			}
			else if (buffer[0]=='F')
			{
				fram_state = PLAT_TRUE;
				//ControlIO_Switch_Chip(FRAM_CONTROL_RELEASE, PLAT_FALSE);
				hal_fram_rdy_write(PLAT_FALSE);				
			}

			hal_uart_send_string(UART_DEBUG, buffer, len);
		}
	}while(len);
}

static void test_fram_handler(void)
{
	PRINTF_STR("Write or Read or Switch\r\n");

	while(1)
	{
		if (!test_cb.uart_recv_date) continue;
		
		test_cb.uart_recv_date = PLAT_FALSE;			
	
		if (strstr(uart_buf, "exit"))
		{
			//ControlIO_Switch_Chip(FRAM_CONTROL_RELEASE, PLAT_FALSE);

			if (fram_state == PLAT_FALSE)
			{
				fram_state = PLAT_TRUE;
				hal_fram_rdy_write(PLAT_FALSE);
			}
			
			hal_uart_rx_irq_disable(UART_LORA);
			hal_uart_rx_set_hiz_state(UART_LORA);			
			return;
		}		
			
		if (uart_buf[0] == 'w')
		{
			PRINTF_STR("Write FRAM address = 0x0, length = HAL_FRAM_SIZE, value=0x00-0xFF\r\n");
			//ControlIO_Switch_Chip(FRAM_CONTROL_RELEASE, PLAT_FALSE);

			hal_fram_get_ver(); 

			hal_fram_write(0, PLAT_NULL, HAL_FRAM_SIZE);
            
            PRINTF_STR("Write Over\r\n");
		}

		if (uart_buf[0] == 'r')
		{
			PRINTF_STR("Read FRAM address = 0x0, length = HAL_FRAM_SIZE\r\n");
			//ControlIO_Switch_Chip(FRAM_CONTROL_RELEASE, PLAT_FALSE);

			hal_fram_get_ver(); 

			uint8_t value = 0;

			for(uint32_t i=0; i<HAL_FRAM_SIZE; i++)
			{
				hal_fram_read(i, &value, 1);
				DBG_PRINTF("%02X", value);
                
                if (uart_buf[0] == 'q')
                {
                    break;
                }
			}

			PRINTF_STR("\r\nRead Over\r\n");
		}
		if (uart_buf[0] == 's')
		{
			PRINTF_STR("Switch FRAM Control\r\n");
			//ControlIO_DevPower(CTL_GRAPH_PROC_PWR, PLAT_TRUE, PLAT_FALSE);
			delay_ms(100);
			//ControlIO_Switch_Chip(FRAM_CONTROL_RELEASE, PLAT_FALSE);
			fram_state = PLAT_FALSE;
			hal_uart_init(UART_LORA, 115200);
			hal_uart_rx_irq_enable(UART_LORA, 1, graph_ctrl_fram_uart_rx_cb);				
			hal_fram_rdy_write(PLAT_TRUE);                
            //ControlIO_Switch_Chip(FRAM_CONTROL_RELEASE, PLAT_TRUE);
			//TODO send cmd
			uart_buf[0] = 0xD9;
			uart_buf[1] = 0xF4;
			uart_buf[2] = 0x01;//graph op
			uart_buf[3] = 0xFF;//test 533 between 432 interface
			uart_buf[4] = 0;
			uart_buf[5] = 0;
			uart_buf[6] = crc8_tab(uart_buf, 0, 6);
			hal_uart_send_string(UART_DEBUG, uart_buf, 7);
			//hal_uart_send_char(UART_GRAPH, 'C');
		}			
	}		
}

static bool_t uart_state = PLAT_FALSE;//FASLE=no used,TRUE=used

static void graph_tc_print(void)
{
	uint8_t buffer[8], len;

	do
	{
		len = hal_uart_read(UART_LORA, buffer, 8);

		hal_uart_send_string(UART_DEBUG, buffer, len);
	} while(len);
}

static void test_graph_ip_handler(void)
{
	uint32_t len;
	
	PRINTF_STR("Graph or IP[t/b]\r\n");

	while(1)
	{
		if (!test_cb.uart_recv_date) continue;
		
        test_cb.uart_recv_date = PLAT_FALSE;			
	
		if (uart_state!= PLAT_TRUE && strstr(uart_buf, "exit"))
		{
			return;
		}

		if (uart_state)
		{
			hal_uart_send_string(UART_LORA, uart_buf, len);
		}
		
		//input 'tc' means touchuan graph or ip uart transport to dbg 432 
		if (uart_buf[0] == 't')
		{
			if (uart_state == PLAT_FALSE)
			{
				//touchuan
                PRINTF_STR("Start TC\r\n");	
				hal_uart_init(UART_LORA, 115200);
				hal_uart_rx_irq_enable(UART_LORA, 1, graph_tc_print);
				uart_state = PLAT_TRUE;
			}
			else
			{
                PRINTF_STR("Stop TC\r\n");	
				hal_uart_rx_irq_disable(UART_LORA);
				hal_uart_rx_set_hiz_state(UART_LORA);
				uart_state = PLAT_FALSE;
			}
		}
		
		//input 'b' means bypass graph uart transport to debug
		if (uart_buf[0] == 'b')
		{
			if (uart_state == PLAT_FALSE)
			{
				//set Hiz state
                PRINTF_STR("set Hiz state\r\n");	
				hal_uart_rx_set_hiz_state(UART_LORA);
			}
		}	
	}
}

static void zdsx_bd_tc_print(void)
{
	uint8_t buffer[8], len;

	do
	{
		//UART_GPS = UART_ZDSX
		len = hal_uart_read(UART_GPS, buffer, 8);

		hal_uart_send_string(UART_DEBUG, buffer, len);
	} while(len);
}

static void test_zdsx_bd_handler(void)
{
	uint32_t len;
	device_info_t *p_device_info = device_info_get();
	//device_type_t dev_type = (device_type_t)GET_DEV_MAC(p_device_info->mac);
	
	PRINTF_STR("ZDSX or BD[t]\r\n");	

	test_cb.uart_state = UART_CMD_STATE;
	test_cb.uart_recv_date = PLAT_FALSE;

	while(1)
	{
		if (!test_cb.uart_recv_date) continue;
		
        test_cb.uart_recv_date = PLAT_FALSE;			
	
		if (uart_state!= PLAT_TRUE && strstr(uart_buf, "exit"))
		{
			return;
		}

		if (uart_state)
		{
			hal_uart_send_string(UART_GPS, uart_buf, len);
		}
			
		//input 'tc' means touchuan zdsx or gps uart transport to dbg 432 
		if (uart_buf[0] == 't')
		{
			if (uart_state == PLAT_FALSE)
			{
				//touchuan
                PRINTF_STR("Start TC\r\n");
				
				
				hal_uart_rx_irq_enable(UART_GPS, 1, zdsx_bd_tc_print);
				uart_state = PLAT_TRUE;
			}
			else
			{
                PRINTF_STR("Stop TC\r\n");	
				hal_uart_rx_irq_disable(UART_GPS);
				uart_state = PLAT_FALSE;
			}
		}
	}
}

static void test_hw_handler(void)
{
	uint32_t len;
	device_info_t *p_device_info = device_info_get();
	//device_type_t dev_type = (device_type_t)GET_DEV_MAC(p_device_info->mac);
	
	PRINTF_STR("HW test[detect][samp]\r\n");	

	test_cb.uart_state = UART_CMD_STATE;
	test_cb.uart_recv_date = PLAT_FALSE;

	while(1)
	{
		if (!test_cb.uart_recv_date) continue;
		
        test_cb.uart_recv_date = PLAT_FALSE;			
	
		if (strstr(uart_buf, "exit"))
		{
			//sensor_hw_stop(HW_STOP_MODE);
			return;
		}
			
		if (strstr(uart_buf, "detect"))
		{
			//sensor_hw_start(HW_DETECT_MODE);
			osel_task_suspend(app_task_h);
			osel_task_suspend(stack_task_h);
			osel_task_suspend(init_task_h);			
		}
        
		if (strstr(uart_buf, "samp"))
		{
			//sensor_hw_start(HW_DAQ_MODE);
			osel_task_suspend(app_task_h);
			osel_task_suspend(stack_task_h);
			osel_task_suspend(init_task_h);
		}
	}
}

static uint8_t aux_cmd_read_info_format[]={0xD9, 0xF4, 0x03, 0x01, 0x00, 0x00, 0x11};

const uint8_t aux_cmd_start_read_bd[]={0xD9, 0xF4, 0x03, 0x20, 0x01, 0x00, 0x01, 0x11};

const uint8_t aux_cmd_stop_read_bd[]={0xD9, 0xF4, 0x03, 0x20, 0x01, 0x00, 0x00, 0x11};

static uint8_t aux_data_array[20];

static uint8_t aux_timer_id = 0;

static uint8_t aux_data_array_index = 0;

static void aux_timeout_cb(void);

static void aux_uart_rx_cb()
{
	uint8_t buffer, len;

//	app_sensor_inside_frm_head *p_snr_inside_frm = (app_sensor_inside_frm_head *)aux_data_array;
//
//	app_aux_compass_t *p_aux_compass 
//			= (app_aux_compass_t *)(&aux_data_array[sizeof(app_sensor_inside_frm_head)]);
//	app_aux_postion_t *p_aux_position 
//			= (app_aux_postion_t *)(&aux_data_array[sizeof(app_sensor_inside_frm_head)]);
//	app_aux_altitude_t *p_aux_altitude 
//			= (app_aux_altitude_t *)(&aux_data_array[sizeof(app_sensor_inside_frm_head)]);
//	app_aux_illumination_t *p_aux_illumination 
//			= (app_aux_illumination_t *)(&aux_data_array[sizeof(app_sensor_inside_frm_head)]);
//	app_aux_cal_compass_t *p_aux_cal_compass 
//			= (app_aux_cal_compass_t *)(&aux_data_array[sizeof(app_sensor_inside_frm_head)]);
//	app_aux_version_t *p_aux_version
//			= (app_aux_version_t *)(&aux_data_array[sizeof(app_sensor_inside_frm_head)]);
	do
	{
		len = hal_uart_read(UART_DEBUG, &buffer, 1);

		if (len == 0) return;

		if (uart_state == PLAT_TRUE)
        {
            hal_uart_send_string(UART_DEBUG, &buffer, len);
        }
		else if (buffer == 0xD9 && aux_data_array_index == 0)
        {
        	aux_data_array[aux_data_array_index] = buffer;
			aux_data_array_index = 1;
		}
		else if (buffer == 0xF4 && aux_data_array_index == 1)
		{
			aux_data_array[aux_data_array_index] = buffer;
			aux_data_array_index = 2;
		}
		else if (aux_data_array_index >=2)
		{            
			aux_data_array[aux_data_array_index++] = buffer;
			
//			if (aux_data_array_index >= sizeof(app_sensor_inside_frm_head) 
//				&& p_snr_inside_frm->payload_len == aux_data_array_index - sizeof(app_sensor_inside_frm_head))
//			{
//				aux_data_array_index = 0;                
//                DBG_PRINTF("\r\n");                
//				switch (p_snr_inside_frm->sub_type)
//				{					
//					//case AUX_REQ_COMPASS_A:
//					//case AUX_REQ_COMPASS_B:
//						DBG_PRINTF("type=%d\r\n", p_aux_compass->type);
//						DBG_PRINTF("direction=%d.%d\r\n", p_aux_compass->dir_int, p_aux_compass->dir_dec);
//						DBG_PRINTF("pitch=");
//						if (p_aux_compass->pitch_pm)
//						{
//							DBG_PRINTF("-");
//						}
//						DBG_PRINTF("%d.%d\r\n", p_aux_compass->pitch_int, p_aux_compass->pitch_dec);
//						DBG_PRINTF("roll=");
//						if (p_aux_compass->roll_pm)
//						{
//							DBG_PRINTF("-");
//						}
//						DBG_PRINTF("%d.%d\r\n", p_aux_compass->roll_int, p_aux_compass->roll_dec);
//						break;
//					//case AUX_REQ_POSITION:
//						DBG_PRINTF("latitude=%c", p_aux_position->lat_type);
//						DBG_PRINTF("-%lf\r\n", (fp64_t)p_aux_position->lat/1000000);
//						DBG_PRINTF("longtitude=%c", p_aux_position->lngt_type);
//						DBG_PRINTF("-%lf\r\n", (fp64_t)p_aux_position->lngt/1000000);
//						break;
//					//case AUX_REQ_ALTITUDE:
//						DBG_PRINTF("altitude="); 
//						if (p_aux_altitude->alt_pm)
//						{
//							DBG_PRINTF("-");
//						}
//						DBG_PRINTF("%d.%d\r\n", p_aux_altitude->alt_int, p_aux_altitude->alt_dec);
//						break;
//					//case AUX_REQ_ILLUM:
//						DBG_PRINTF("illumination=%d\r\n", p_aux_illumination->illum);
//						break;
///case AUX_REQ_VERSION:
//						DBG_PRINTF("version=0x%04x\r\n", p_aux_version->version);
//						break;
//					case AUX_CAL_COMPASS:
//						DBG_PRINTF("Cal complete rate=%d%\r\n", p_aux_cal_compass->rate);
//						if (p_aux_cal_compass->rate == 100)
//						{
//                            DBG_PRINTF("Cal complete!\r\n");
//							aux_timeout_cb();
//						}
//						break;
//					default:break;
//				}
//			}
		}
		else
		{
			aux_data_array_index = 0;
		}
	} while(1);	
}

static void aux_timeout_cb(void)
{	
	aux_timer_id = hal_timer_free(aux_timer_id);
    
    if (uart_state)
	{
		hal_uart_send_string(UART_DEBUG, (uint8_t *)aux_cmd_stop_read_bd, sizeof(aux_cmd_stop_read_bd));
		delay_ms(10);
    }

	PRINTF_REGISTER_CB(test_uart_rx_cb);
	
	uart_state = PLAT_FALSE;
}

static void test_aux_handler(void)
{
	uint32_t len;
	uint8_t *p_cmd = PLAT_NULL;
	uint32_t wait_us = 0;
	
	PRINTF_STR("AUX\r\n[c]=Rd compass\r\n[p]=Rd position\r\n[a]=Rd altitiude\r\n[i]=Rd illum\r\n[t]=Rd BD\r\n");	
	PRINTF_STR("[m]=Enter Calibrate\r\n[v]=Version\r\n[exit]=quit\r\n");

	while(1)
	{
		if (!test_cb.uart_recv_date) continue;
		
        test_cb.uart_recv_date = PLAT_FALSE;        
        
        uart_state = PLAT_FALSE;
			
		if (len)
		{
			if (strstr(uart_buf, "exit"))
			{
				PRINTF_REGISTER_CB(test_uart_rx_cb);
				return;
			}			
			
            wait_us = 500000;
            
            switch (uart_buf[0])
            {
                case 'c':
                    p_cmd = (uint8_t *)aux_cmd_read_info_format;
					//p_cmd[3] = AUX_REQ_COMPASS_B;
                    len = sizeof(aux_cmd_read_info_format);						
                    break;
                case 'p':
                    p_cmd = (uint8_t *)aux_cmd_read_info_format;
					//p_cmd[3] = AUX_REQ_POSITION;
                    len = sizeof(aux_cmd_read_info_format);						
                    break;
                case 'a':
                    p_cmd = (uint8_t *)aux_cmd_read_info_format;
					//p_cmd[3] = AUX_REQ_ALTITUDE;
                    len = sizeof(aux_cmd_read_info_format);						
                    break;
                case 'i':
                    p_cmd = (uint8_t *)aux_cmd_read_info_format;
					//p_cmd[3] = AUX_REQ_ILLUM;
                    len = sizeof(aux_cmd_read_info_format);						
                    break;
				 case 'm':
                    p_cmd = (uint8_t *)aux_cmd_read_info_format;
					//p_cmd[3] = AUX_CAL_COMPASS;
                    wait_us = 60000000;
                    len = sizeof(aux_cmd_read_info_format);						
                    break;
				case 'M':
					p_cmd = (uint8_t *)aux_cmd_read_info_format;
					//p_cmd[3] = AUX_CAL_SOFT_COMPASS;
                    wait_us = 120000000;
                    len = sizeof(aux_cmd_read_info_format);
					uart_state = PLAT_TRUE;
                    break;					
				case 'v':
                    p_cmd = (uint8_t *)aux_cmd_read_info_format;
					//p_cmd[3] = AUX_REQ_VERSION;
                    len = sizeof(aux_cmd_read_info_format);						
                    break;	
                case 't':
                    p_cmd = (uint8_t *)aux_cmd_start_read_bd;					
                    len = sizeof(aux_cmd_start_read_bd);
                    wait_us = 60000000;
                    uart_state = PLAT_TRUE;
                    break;
                default:
                    continue;	
            }
            uart_buf[0] = 0;
            PRINTF_STR("Send CMD!\r\n");
            //ControlIO_Switch_Chip(UART_CONTROL_RELEASE, PLAT_TRUE);
            delay_ms(10);	
            hal_uart_rx_irq_enable(UART_DEBUG, 1, aux_uart_rx_cb);								
            aux_timer_id = hal_timer_alloc(wait_us, aux_timeout_cb);
			//add crc8 check
			p_cmd[len-1] = crc8_tab(p_cmd, 0, len-1);

			hal_uart_send_string(UART_DEBUG, p_cmd, len);			
		}			
	}
}