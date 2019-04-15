
#include <platform.h>
#include <ti\devices\msp432p4xx\inc\system_msp432p401r.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>
#include <ti\devices\msp432p4xx\driverlib\interrupt.h>
#include <ti\devices\msp432p4xx\driverlib\pmap.h>
#include <ti\devices\msp432p4xx\driverlib\timer_a.h>
#include <ti\devices\msp432p4xx\driverlib\spi.h>

//Radio0
#define RF0_BRD_PWR_PIN				52u
#define RF0_INT0_PIN	            73u
#define RF0_INT1_PIN		        10u
#define RF0_RST_PIN		        	60u
#define RF0_TX_PIN		        	71u
#define RF0_RX_PIN		        	42u
#define RF0_PWR_PIN		    		66u
#define RF0_PWR_CORE_PIN		    67u
#define RF0_CONT_PIN				61u
#define RF0_SPI_CS_PIN				62u
#define RF0_SPI_CLK_PIN				63u
#define RF0_SPI_MOSI_PIN			64u
#define RF0_SPI_MISO_PIN			65u

#define RF0_TX_EN()          		(P7OUT |= BIT1)
#define RF0_TX_DIS()         		(P7OUT &= ~BIT1)
#define RF0_TX_TRIG()            	{P7OUT |= BIT1; delay_us(5); P7OUT &= ~BIT1;}

#define RF0_TX_CONT_EN()     		(P6OUT |= BIT1)
#define RF0_TX_CONT_DIS()    		(P6OUT &= ~BIT1)

#define RF0_RX_EN()          		(P4OUT |= BIT2)
#define RF0_RX_DIS()         		(P4OUT &= ~BIT2)

#define RF0_SPI_CS_EN()				(P6OUT |= BIT2)
#define RF0_SPI_CS_DIS()			(P6OUT &= ~BIT2)

#define RF0_INT0_CLR()           	(P7IFG &= ~BIT3)
#define RF0_INT1_CLR()           	(P7IFG &= ~BIT2)

#define RF0_INT0_IND()           	(P7IFG&BIT3)
#define RF0_INT1_IND()           	(P7IFG&BIT2)

#define RF0_PWR_EN()	           	(P6OUT |= BIT6)
#define RF0_PWR_CORE_EN()           (P6OUT |= BIT7)
#define RF0_PWR_DIS()	           	(P6OUT &= ~BIT6)
#define RF0_PWR_CORE_DIS()          (P6OUT &= ~BIT7)

#define RF0_SENSOR_BRD_PWR_DIS()	(P4OUT &= ~BIT4)
#define RF0_SENSOR_BRD_PWR_EN()		(P4OUT |= BIT4)

#define RF0_COMM_BRD_PWR_DIS()	    (P2OUT &= ~BIT0)
#define RF0_COMM_BRD_PWR_EN()		(P2OUT |= BIT0)

#define RF0_RST_EN()           		(P6OUT |= BIT0)
#define RF0_RST_DIS()	           	(P6OUT &= ~BIT0)

#define RF0_BRD_PWR_EN()			(P5OUT |= BIT2)	
#define RF0_BRD_PWR_DIS()			(P5OUT &= ~BIT2)	

///////////////////////////////////////////////////////////////////
//Radio1 Config
#if 0
#define RF1_BRD_PWR_PIN
#define RF1_INT0_PIN
#define RF1_INT1_PIN
#define RF1_RST_PIN	
#define RF1_TX_PIN
#define RF1_RX_PIN
#define RF1_PWR_PIN
#define RF1_PWR_CORE_PIN
#define RF1_CONT_PIN
#define RF1_SPI_CS_PIN
#define RF1_SPI_CLK_PIN
#define RF1_SPI_MOSI_PIN
#define RF1_SPI_MISO_PIN

#define RF1_TX_EN()          		()
#define RF1_TX_DIS()         		()
#define RF1_TX_TRIG()            	{}

#define RF1_TX_CONT_EN()     		()
#define RF1_TX_CONT_DIS()    		()

#define RF1_RX_EN()          		()
#define RF1_RX_DIS()         		()

#define RF1_SPI_CS_EN()				()
#define RF1_SPI_CS_DIS()			()

#define RF1_INT0_CLR()           	()
#define RF1_INT1_CLR()           	()

#define RF1_INT0_IND()           	()
#define RF1_INT1_IND()           	()

#define RF1_PWR_EN()	           	()
#define RF1_PWR_CORE_EN()           ()
#define RF1_PWR_DIS()	           	()
#define RF1_PWR_CORE_DIS()          ()

#define RF1_BRD_PWR_DIS()	    	()
#define RF1_BRD_PWR_EN()			()

#define RF1_RST_EN()           		()
#define RF1_RST_DIS()	           	()

#endif

const bb_reg_info_t rf_default_reg_array[REG_NUM] =
{
    {"AGCCTRL", 	BB_AGCCTRL, 	0x85ef},  
	{"TXDELAY", 	BB_TXDELAY, 	0x2580},	
	{"MISC", 		BB_MISC, 		0x7fce},		//7fc5:25dBm, 7fc8:29dBm, 7fce:31dBm
	{"TXPOWER", 	BB_TXPOWER, 	0x0d60},
	{"RSSI_LQI", 	BB_CCA, 		0x0000},
	{"SHUTDOWN", 	BB_SHUTDOWN, 	0xaac0},
	{"LISTENDELAY", BB_RESET, 		0x0960},
	{"TXCONF0", 	BB_TXCONF0, 	0x0000},
	{"TXCONF1", 	BB_TXCONF1, 	0x0000},
	{"RXCONF",		BB_RXCONF, 		0x0000},
	{"VERSION", 	BB_VERSION, 	0x0000},
	{"AGCSET", 		BB_AGCSET, 		0x7820},
	{"CATCH_GATE",	BB_CATCH_GATE, 	0x0000},
	{"DIFF_GATE", 	BB_DIFF_GATE, 	0x0000},
	{"TRACE_GATE",	BB_TRACE_GATE, 	0x0000},
	{"LOSE_GATE", 	BB_LOSE_GATE, 	0x0000},
	{"43NCTRL", 	BB_43NCTRL, 	0x0352},		//@310MHz		//@340MHz{"43NCTRL", 	BB_43NCTRL, 0x0352},
	{"43RCTRL", 	BB_43RCTRL, 	0x0018},
	{"SFD", 		BB_SFD, 		0x0000},
	{"TXPN_RAM0", 	BB_TXPN_RAM0, 	0x0000},
	{"TXPN_RAM1", 	BB_TXPN_RAM1, 	0x0000},
	{"TXPN_RAM2", 	BB_TXPN_RAM2, 	0x0000},
	{"TXPN_RAM3", 	BB_TXPN_RAM3, 	0x0000},
	{"TXPN_RAM4", 	BB_TXPN_RAM4, 	0x0000},
	{"TXPN_RAM5", 	BB_TXPN_RAM5, 	0x0000},
	{"TXPN_RAM6", 	BB_TXPN_RAM6, 	0x0000},
	{"TXPN_RAM7", 	BB_TXPN_RAM7, 	0x0000},
	{"RXPN_RAM0", 	BB_RXPN_RAM0, 	0x0000},
	{"RXPN_RAM1", 	BB_RXPN_RAM1, 	0x0000},
	{"RXPN_RAM2", 	BB_RXPN_RAM2, 	0x0000},
	{"RXPN_RAM3", 	BB_RXPN_RAM3, 	0x0000},
	{"RXPN_RAM4", 	BB_RXPN_RAM4, 	0x0000},
	{"RXPN_RAM5", 	BB_RXPN_RAM5, 	0x0000},
	{"RXPN_RAM6", 	BB_RXPN_RAM6, 	0x0000},
	{"RXPN_RAM7", 	BB_RXPN_RAM7, 	0x0000}	
};

/////////////////////////////////////////////////////////////////
const uint16_t bb_reg_pn128[REG_PN128_GROUP][REG_PN128_NUM] =
{
	{0x8126, 0x9EE1, 0xFC76, 0x297D, 0x50B7, 0x9CAC, 0xC1B5, 0xD190},
	{0x81FD, 0x533B, 0xA58D, 0xED6C, 0x91C2, 0xF95C, 0xD13C, 0x50C0},
	{0x819B, 0x1CEB, 0x84C1, 0x5692, 0x9E46, 0xA1FD, 0xDBD1, 0x65F0},
	{0x8113, 0x175B, 0x066A, 0x73DA, 0x157D, 0x28DC, 0x7F0E, 0xF2C8},
	{0x81BE, 0x0B08, 0x7462, 0x729A, 0x5EBB, 0x8F32, 0x455B, 0x3FB4},
	{0x81DB, 0xE7F4, 0x95D5, 0x38CB, 0x317B, 0x910D, 0x68F0, 0x4D84},
	{0x8106, 0x1479, 0x1675, 0x3E87, 0x126D, 0x6F63, 0x4BB9, 0x957E},
	{0x81D4, 0x5C7B, 0xB5FD, 0x30D0, 0xA5B2, 0xAC41, 0x3E72, 0x466E},
	{0x81EC, 0xC49C, 0xF904, 0x6A9B, 0x4A16, 0x197F, 0x5BBC, 0x7456},
	{0x8143, 0x641E, 0x2D61, 0x13BD, 0x19A6, 0x3957, 0x525F, 0xCFB6},
	{0x815B, 0xF9B5, 0x4499, 0xE3BA, 0xF4B2, 0x9C8C, 0x5C21, 0xA0FA},
	{0x811F, 0x4D17, 0xB77F, 0x0AC4, 0xF292, 0xD506, 0x43AE, 0x71B2}
};

const uint16_t bb_reg_sfd128[REG_PN128_GROUP] =
{
	//SFD
	0xF465,
	0x67E6,	
	0x7367,
	0x76DC,
	0x7B6F, 
	0xA7CA,
	0xB8E8,
	0xBE7D,
	0xD396,
	0xD7D6,
	0xE6CE,
	0xEB5C
};

const bb_reg_info_t bb_reg_gate128[GATE_REG_NUM] = 
{
	{"CATCH_GATE",	BB_CATCH_GATE, 	0xb830},
	{"DIFF_GATE", 	BB_DIFF_GATE, 	0x0030},
	{"TRACE_GATE",	BB_TRACE_GATE, 	0x0078},
	{"LOSE_GATE", 	BB_LOSE_GATE, 	0x0260}	
};

const bb_reg_info_t bb_reg_conf128[CONF_REG_NUM] = 
{
	{"TXCONF0", 	BB_TXCONF0, 	0x9c00},
	{"TXCONF1", 	BB_TXCONF1, 	0x1000},
	{"RXCONF",		BB_RXCONF, 		0x870a},
};
//////////////////////////////////////////////////////////////
const uint16_t bb_reg_pn64[REG_PN64_GROUP][REG_PN64_NUM] =
{
	{0x83F5, 0x66ED, 0x2717, 0x9460},
	{0x8384, 0x8D96, 0xBBCC, 0x54FC},
	{0x8218, 0xA7A3, 0x92DD, 0x9ABE},
	{0x8373, 0x1D7E, 0xD10B, 0x2A4E},
	{0x82FC, 0xA8CF, 0x75A6, 0xC486},
	{0x83C9, 0x5342, 0x2DFA, 0xE33A}
};

const uint16_t bb_reg_sfd64[REG_PN64_GROUP] =
{
	//SFD
	0xF465,
	0x67E6,	
	0x7367,
	0x76DC,
	0x7B6F, 
	0xA7CA
};

const bb_reg_info_t bb_reg_gate64[GATE_REG_NUM] = 
{
	{"CATCH_GATE",	BB_CATCH_GATE, 	0x9830},
	{"DIFF_GATE", 	BB_DIFF_GATE, 	0x002a},
	{"TRACE_GATE",	BB_TRACE_GATE, 	0x0068},
	{"LOSE_GATE", 	BB_LOSE_GATE, 	0x0500}
};

const bb_reg_info_t bb_reg_conf64[CONF_REG_NUM] = 
{
	{"TXCONF0", 	BB_TXCONF0, 	0x7c00},
	{"TXCONF1", 	BB_TXCONF1, 	0x1000},
	{"RXCONF",		BB_RXCONF, 		0x670a},
};
///////////////////////////////////////////////////////////////

static rf_int_reg_t rf_int_reg[RF_RADIO_MAX][RF_INT_MAX_NUM];
static uint16_t rf_reg_value[RF_RADIO_MAX][REG_NUM];
static uint8_t prev_state[RF_RADIO_MAX];
//保存射频参数的索引号
/* static function */
static void rf_write_reg(uint8_t radio_id, uint8_t addr, uint16_t value);
static uint16_t rf_read_reg(uint8_t radio_id, uint8_t addr);
static void rf_cfg(uint8_t radio_id);
static void rf_cfg_int(uint8_t radio_id, uint16_t int_id, uint16_t ctrl_type, uint16_t int_type);
static bool_t rf_reg_int(uint8_t radio_id, uint16_t int_type, rf_cb_func_t cb_fun_ptr);
static bool_t rf_unreg_int(uint8_t radio_id, uint8_t int_type);
static void hal_rf_port1_isr(void);
static void hal_timerA0_cr0_isr(void);

/* global function */
bool_t hal_rf_init(uint8_t radio_id, uint8_t pn_type, uint8_t pn_group)
{
    OSEL_DECL_CRITICAL();
    bool_t res;
    uint16_t version = 0;
	uint8_t loop_cnt = 0;	
	uint16_t (* rf_flash_value)[REG_NUM] = PLAT_NULL;	
	
	if (radio_id == RF_RADIO_0)
	{	
    	/* 供电IO初始�? */
		GPIO_setAsOutputPin((RF0_BRD_PWR_PIN/10), (1u<<(RF0_BRD_PWR_PIN%10)));
		RF0_BRD_PWR_EN();		
		GPIO_setAsOutputPin((RF0_PWR_PIN/10), (1u<<(RF0_PWR_PIN%10)));
		RF0_PWR_DIS();
		GPIO_setAsOutputPin((RF0_PWR_CORE_PIN/10), (1u<<(RF0_PWR_CORE_PIN%10)));
		RF0_PWR_CORE_DIS();
        //Continue Tx
		GPIO_setAsOutputPin((RF0_CONT_PIN/10), (1u<<(RF0_CONT_PIN%10)));
    	RF0_TX_CONT_DIS();
		/* Reset baseband */
		GPIO_setAsOutputPin((RF0_RST_PIN/10), (1u<<(RF0_RST_PIN%10)));
		RF0_RST_DIS();
		/* pull down tx&rx enable pin */
		GPIO_setAsOutputPin((RF0_TX_PIN/10), (1u<<(RF0_TX_PIN%10)));
		RF0_TX_DIS();
		GPIO_setAsOutputPin((RF0_RX_PIN/10), (1u<<(RF0_RX_PIN%10)));
		RF0_RX_DIS();
		
    	/* 配置基带的SPI接口 */
		GPIO_setAsOutputPin((RF0_SPI_CS_PIN/10), (1u<<(RF0_SPI_CS_PIN%10)));
		RF0_SPI_CS_EN();
		GPIO_setAsPeripheralModuleFunctionOutputPin((RF0_SPI_CLK_PIN/10), 
													(1u<<(RF0_SPI_CLK_PIN%10)), 
													GPIO_PRIMARY_MODULE_FUNCTION);
		GPIO_setAsPeripheralModuleFunctionOutputPin((RF0_SPI_MOSI_PIN/10), 
													(1u<<(RF0_SPI_MOSI_PIN%10)), 
													GPIO_PRIMARY_MODULE_FUNCTION);
    	GPIO_setAsPeripheralModuleFunctionInputPin((RF0_SPI_MISO_PIN/10), 
													(1u<<(RF0_SPI_MISO_PIN%10)), 
													GPIO_PRIMARY_MODULE_FUNCTION);
		eUSCI_SPI_MasterConfig config;
		config.spiMode = EUSCI_B_SPI_3PIN;
		config.selectClockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK;
		config.msbFirst = EUSCI_B_SPI_MSB_FIRST;
		config.desiredSpiClock = 4000000;
		config.clockSourceFrequency = 12000000;
		config.clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
		config.clockPhase = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
		
		SPI_initMaster(EUSCI_B1_SPI_BASE, &config);
		
		SPI_enableModule(EUSCI_B1_SPI_BASE);
		
	   	/* 基带中断的IO配置    	sfd 中断配置 */
		//管脚映射 pin73 管脚，映射到 硬定�?
		
		const uint8_t portmap[1] = 
		{
			PMAP_TA0CCR0A //RF0_INT0_73			
		};
			
		PMAP_configurePorts(portmap, PMAP_P7MAP+3, 1, PMAP_ENABLE_RECONFIGURATION);		
		GPIO_setAsPeripheralModuleFunctionInputPin((RF0_INT0_PIN/10), 
													(1u<<(RF0_INT0_PIN%10)), 
													GPIO_PRIMARY_MODULE_FUNCTION);		
		//对映射的定时进行配置
		
		TA0CTL = TASSEL_1 + MC_2 + TACLR + TAIE;
		TA0CCTL0 = CM_1 + SCS + CAP + CCIE;
		
		//配置此定时的中断回调，当 PIN73 产生中断，则定时器就会调用此中断
    	
		Timer_A_registerInterrupt(TIMER_A0_BASE, 
									TIMER_A_CCR0_INTERRUPT, 
									hal_timerA0_cr0_isr);
		
		//数据发�?�接收中断配�?
		
		GPIO_setAsInputPin((RF0_INT1_PIN/10), (1u<<(RF0_INT1_PIN%10)));
		
		GPIO_interruptEdgeSelect((RF0_INT1_PIN/10), (1u<<(RF0_INT1_PIN%10)), GPIO_LOW_TO_HIGH_TRANSITION);
		
		GPIO_registerInterrupt((RF0_INT1_PIN/10), hal_rf_port1_isr);

		GPIO_clearInterruptFlag((RF0_INT1_PIN/10), (1u<<(RF0_INT1_PIN%10)));

		GPIO_enableInterrupt((RF0_INT1_PIN/10), (1u<<(RF0_INT1_PIN%10)));

		//Power on	
		RF0_PWR_EN();
		delay_us(5);
		RF0_PWR_CORE_EN();
		delay_ms(1);
		RF0_RST_EN();
        delay_us(500);		
	}	
	else
	{
		return PLAT_FALSE;
	}

	while(version != BB_VER_ASIC_PN)
	{
		//获得基带版本�?0x15
		version	= rf_read_reg(radio_id, BB_VERSION);
       	loop_cnt++;
		if (loop_cnt>100)
		{
			DBG_TRACE("No BB%d find!\r\n", radio_id);
			return PLAT_FALSE;
		}
        delay_ms(10);
	}
  
	DBG_TRACE("BB%d Version=0x%X!\r\n",radio_id, version);
	/* 关闭基带 */
	hal_rf_set_state(radio_id, HAL_RF_OFF_STATE);

	if (pn_type == DSSS_PN128)
	{
		if (pn_group >= REG_PN128_GROUP) return PLAT_FALSE;
	}
	else if (pn_type == DSSS_PN64)
	{
		if (pn_group >= REG_PN64_GROUP) return PLAT_FALSE;
	}
	else
	{
		return PLAT_FALSE;
	}

	//查看保存�?
	res = hal_flash_alloc(RF_STORE_INDEX, (uint8_t *)rf_reg_value, sizeof(rf_reg_value));
	if (res == PLAT_FALSE)
	{
		return PLAT_FALSE;
	}
	
	rf_flash_value = (uint16_t (*)[REG_NUM])hal_flash_read(RF_STORE_INDEX);
	
	uint16_t value = (rf_flash_value)[radio_id][0];
	uint8_t index;
	uint8_t config_index;
	uint8_t gate_index;
	uint8_t pn_index;
	uint8_t sfd_index;
	
	if (value == 0xFFFF || value == 0)
	{
		for (index=0; index<REG_NUM; index++)
		{
			if (rf_default_reg_array[index].addr == BB_TXCONF0)
			{					
				for (config_index=0; config_index<CONF_REG_NUM; config_index++)
				{
					if (pn_type == DSSS_PN128)
					{
						rf_reg_value[radio_id][config_index+index] = bb_reg_conf128[config_index].value;
					}
					else if (pn_type == DSSS_PN64)
					{
						rf_reg_value[radio_id][config_index+index] = bb_reg_conf64[config_index].value;
					}
					else
					{
						rf_reg_value[radio_id][config_index+index] = 0;
					}
				}
				index += CONF_REG_NUM-1;
			}
			else if (rf_default_reg_array[index].addr == BB_CATCH_GATE)
			{
				for (gate_index=0; gate_index<GATE_REG_NUM; gate_index++)
				{
					if (pn_type == DSSS_PN128)
					{
						rf_reg_value[radio_id][gate_index+index] = bb_reg_gate128[gate_index].value;
					}
					else if (pn_type == DSSS_PN64)
					{
						rf_reg_value[radio_id][gate_index+index] = bb_reg_gate64[gate_index].value;
					}
					else
					{
						rf_reg_value[radio_id][gate_index+index] = 0;
					}
				}
				index += GATE_REG_NUM-1;
			}
			else if (rf_default_reg_array[index].addr == BB_SFD)
			{
				sfd_index = index;
				
				if (pn_type == DSSS_PN128)
				{
					rf_reg_value[radio_id][sfd_index] = bb_reg_sfd128[pn_group];
				}
				else if (pn_type == DSSS_PN64)
				{
					rf_reg_value[radio_id][sfd_index] = bb_reg_sfd64[pn_group];
				}
				else
				{
					rf_reg_value[radio_id][sfd_index] = 0;
				}
			}
			else if (rf_default_reg_array[index].addr == BB_TXPN_RAM0)
			{
				if (pn_type == DSSS_PN128)
				{
					for (pn_index=0; pn_index<REG_PN128_NUM; pn_index++)
					{
						rf_reg_value[radio_id][pn_index+index] = bb_reg_pn128[pn_group][pn_index];
						rf_reg_value[radio_id][pn_index+index+REG_PN128_NUM] = bb_reg_pn128[pn_group][pn_index];	
					}
				}
				else if (pn_type == DSSS_PN64)
				{
					for (pn_index=0; pn_index<REG_PN64_NUM; pn_index++)
					{
						rf_reg_value[radio_id][pn_index+index] = bb_reg_pn64[pn_group][pn_index];
						rf_reg_value[radio_id][pn_index+index+REG_PN128_NUM] = bb_reg_pn64[pn_group][pn_index];
					}
				}
				else
				{
					//TODO
				}
				index += REG_PN128_NUM+REG_PN128_NUM-1;
			}			
			else
			{
				rf_reg_value[radio_id][index] = rf_default_reg_array[index].value;

				if (rf_default_reg_array[index].addr == BB_VERSION)
		        {
		            rf_reg_value[radio_id][index] = version;
		        }
			}
		}
		
        OSEL_ENTER_CRITICAL();
		hal_flash_write(RF_STORE_INDEX);
        OSEL_EXIT_CRITICAL();
	}
	else
	{
		for(index=0; index<REG_NUM; index++)
		{
			rf_reg_value[radio_id][index] = rf_flash_value[radio_id][index];

			if (rf_default_reg_array[index].addr == BB_VERSION)
	        {
	            rf_reg_value[radio_id][index] = version;
	        }
		}
	}		

	return PLAT_TRUE;
}

void hal_rf_deinit(uint8_t radio_id)
{
	if (radio_id == RF_RADIO_0)
	{
		RF0_TX_CONT_DIS();
		RF0_PWR_DIS();
		RF0_PWR_CORE_DIS();
		RF0_RST_DIS();
		RF0_SPI_CS_DIS();

		RF0_TX_DIS();
		RF0_RX_DIS();
		
		UCB1CTL1 |= UCSWRST;
		UCB1CTL0 = 0x0000;
		UCB1CTL1 = 0x0000;
		UCB1STAT = 0x0000;
		UCB1BR0 = 0;
		UCB1BR1 = 0;	
		UCB1CTL1 &= ~UCSWRST;		
	}
}

uint8_t hal_rf_get_state(uint8_t radio_id)
{
	return prev_state[radio_id];
}

bool_t hal_rf_set_state(uint8_t radio_id, uint8_t rf_state)
{
	if (radio_id >= RF_RADIO_MAX) return PLAT_FALSE;

	if (radio_id == RF_RADIO_0)
	{	
		if ((prev_state[radio_id] == HAL_RF_OFF_STATE) && (rf_state != HAL_RF_OFF_STATE))
		{
			RF0_PWR_EN();
			RF0_PWR_CORE_EN();	
			delay_us(600);
			RF0_RST_EN();
			delay_us(20);
	        rf_cfg(radio_id);			
	        prev_state[radio_id] = HAL_RF_IDLE_STATE;		
		}

		if (rf_state == HAL_RF_RX_STATE)
		{
			RF0_TX_DIS();
			RF0_TX_CONT_DIS();
			RF0_RX_EN();
	        prev_state[radio_id] = HAL_RF_RX_STATE;
			return PLAT_TRUE;
		}
		else if (rf_state == HAL_RF_TX_STATE)
		{
			RF0_RX_DIS();
			RF0_TX_CONT_DIS();
			RF0_TX_TRIG();
			prev_state[radio_id] = HAL_RF_TX_STATE;
			return PLAT_TRUE;
		}
		else if (rf_state == HAL_RF_IDLE_STATE)
		{
			RF0_TX_DIS();
			RF0_RX_DIS();
			RF0_TX_CONT_DIS();
			prev_state[radio_id] = HAL_RF_IDLE_STATE;
			return PLAT_TRUE;
		}
		else if (rf_state == HAL_RF_OFF_STATE)
		{
	        RF0_TX_DIS();
			RF0_RX_DIS();
			RF0_TX_CONT_DIS();
			RF0_RST_DIS();
			RF0_PWR_CORE_DIS();
			RF0_PWR_DIS();			
			prev_state[radio_id] = HAL_RF_OFF_STATE;
			return PLAT_TRUE;
		}
		else if (rf_state == HAL_RF_CON_TX_STATE)
		{
			RF0_TX_DIS();
			RF0_RX_DIS();
			RF0_TX_CONT_EN();
			prev_state[radio_id] = HAL_RF_CON_TX_STATE;
			return PLAT_TRUE;
		}
		else
		{
			return PLAT_FALSE;		
		}
	}
    return PLAT_FALSE;	
}

bool_t hal_rf_write(uint8_t radio_id, const uint8_t *buffer, uint8_t length)
{
    uint16_t i;

	if (radio_id == RF_RADIO_0)
	{
	   	RF0_SPI_CS_DIS();
	    UCB1TXBUF = BB_TXFIFO;
	    while (UCB1STAT & UCBUSY);

	    for( i = 0 ; i < length ; i++ )
	    {
	        UCB1TXBUF = buffer[i];
	        while (UCB1STAT & UCBUSY);
	    }
	    RF0_SPI_CS_EN();
	    return PLAT_TRUE;
	}

	return PLAT_FALSE;
}


bool_t hal_rf_read(uint8_t radio_id, uint8_t *buffer, uint8_t length)
{
	uint16_t i;

	if (radio_id == RF_RADIO_0)
	{
	    RF0_SPI_CS_DIS();

	    UCB1TXBUF = (BB_RXFIFO|0x80);
	    while (UCB1STAT & UCBUSY);

	    for(i = 0 ; i < length-1 ; i++)
	    {
	        UCB1TXBUF = 0x00;
	        while (UCB1STAT & UCBUSY);
	        buffer[i] = UCB1RXBUF;
	    }

	    UCB1TXBUF = 0xFF;
	    while (UCB1STAT & UCBUSY);
	    buffer[length-1] = UCB1RXBUF;

	    RF0_SPI_CS_EN();
		
	    return PLAT_TRUE;
	}
	
	return PLAT_FALSE;
}

void hal_rf_flush(uint8_t radio_id)
{	
	uint16_t i;
	uint8_t buffer;
	uint8_t length = 255;

	if (radio_id == RF_RADIO_0)
	{
	    RF0_SPI_CS_DIS();

	    UCB1TXBUF = (BB_RXFIFO|0x80);
	    while (UCB1STAT & UCBUSY);

	    for(i = 0 ; i < length-1 ; i++)
	    {
	        UCB1TXBUF = 0x00;
	        while (UCB1STAT & UCBUSY);
	        buffer = UCB1RXBUF;
			(void)buffer;
	    }

	    UCB1TXBUF = 0xFF;
	    while (UCB1STAT & UCBUSY);
	    buffer = UCB1RXBUF;

	    RF0_SPI_CS_EN();
	}	
}

bool_t hal_rf_cfg_int(uint8_t radio_id, uint16_t int_type, bool_t flag)
{
	if (radio_id >= RF_RADIO_MAX) return PLAT_FALSE;
	
	switch (int_type)
    {
    case HAL_RF_RXOK_INT:
        rf_cfg_int(radio_id, BB_INT_PIN1, flag, REC_DATA_ENABLE);
        break;

    case HAL_RF_TXOK_INT:
        rf_cfg_int(radio_id, BB_INT_PIN1, flag, SEND_OK_ENABLE);
        break;

    case HAL_RF_RXSFD_INT:
        rf_cfg_int(radio_id, BB_INT_PIN0, flag, RX_SFD_ENABLE);
        break;

    case HAL_RF_TXSFD_INT:
        rf_cfg_int(radio_id, BB_INT_PIN0, flag, TX_SFD_ENABLE);
        break;

    default:
        break;
    }
	return PLAT_TRUE;	
}


bool_t hal_rf_reg_int(uint8_t radio_id, uint16_t int_type, rf_cb_func_t cb)
{
	if (radio_id >= RF_RADIO_MAX) return PLAT_FALSE;	
	
    switch (int_type)
    {
    case HAL_RF_RXOK_INT:
        int_type = RF_INT_TYPE_RXOK;
        break;

    case HAL_RF_TXOK_INT:
        int_type = RF_INT_TYPE_TXOK;
        break;

    case HAL_RF_RXSFD_INT:
        int_type = RF_INT_TYPE_RXSFD;
        break;

    case HAL_RF_TXSFD_INT:
        int_type = RF_INT_TYPE_TXSFD;
        break;
    }    
	
    return rf_reg_int(radio_id, int_type, cb);
}

void hal_rf_reset(uint8_t radio_id)
{
	if (radio_id >= RF_RADIO_MAX) return;
	
	hal_rf_set_state(radio_id, HAL_RF_OFF_STATE);
	delay_ms(10);
	hal_rf_set_state(radio_id, HAL_RF_IDLE_STATE);
    
	//读取基带寄存�?,因为这里是上电状�?
	for (uint8_t i=0; i<REG_NUM; i++)
	{
		rf_reg_value[radio_id][i] = rf_read_reg(radio_id, rf_default_reg_array[i].addr);
	}
}

bool_t hal_rf_unreg_int(uint8_t radio_id, uint8_t int_type)
{
	if (radio_id >= RF_RADIO_MAX) return PLAT_FALSE;
	
    switch (int_type)
    {
    case HAL_RF_RXOK_INT:
        int_type = RF_INT_TYPE_RXOK;
        break;

    case HAL_RF_TXOK_INT:
        int_type = RF_INT_TYPE_TXOK;
        break;

    case HAL_RF_RXSFD_INT:
        int_type = RF_INT_TYPE_RXSFD;
        break;

    case HAL_RF_TXSFD_INT:
        int_type = RF_INT_TYPE_TXSFD;
        break;
    }   

	return rf_unreg_int(radio_id, int_type);	
}

void hal_rf_get_register(uint8_t radio_id, uint8_t reg_addr, uint16_t *reg_value)
{
	if (radio_id >= RF_RADIO_MAX) return;
	
	if (reg_value == PLAT_NULL)
	{
		return;
	}
	
	for (uint8_t i=0; i<REG_NUM; i++)
	{
		if (rf_default_reg_array[i].addr == reg_addr)
		{
			*reg_value = rf_reg_value[radio_id][i];
			break;
		}
	}
}

bool_t hal_rf_set_register(uint8_t radio_id, uint8_t reg_addr, uint16_t *reg_value)
{
	OSEL_DECL_CRITICAL();
	
	if (radio_id >= RF_RADIO_MAX) return PLAT_FALSE;
		
	if (reg_value == PLAT_NULL)
	{
		return PLAT_FALSE;
	}

	for (uint8_t i=0; i<REG_NUM; i++)
	{
		if (rf_default_reg_array[i].addr == reg_addr)
		{
			if (rf_reg_value[radio_id][i] != *reg_value)
			{
				rf_reg_value[radio_id][i] = *reg_value;
				//save to flash				
				OSEL_ENTER_CRITICAL();
				hal_flash_write(RF_STORE_INDEX);
				OSEL_EXIT_CRITICAL();
				return PLAT_TRUE;
			}			
			break;
		}
	}
		
	return PLAT_FALSE;	
}

uint16_t hal_rf_read_reg(uint8_t radio_id, uint8_t addr)
{
	return rf_read_reg(radio_id, addr);
}

void hal_rf_write_reg(uint8_t radio_id, uint8_t addr, uint16_t value)
{
	rf_write_reg(radio_id, addr, value);
}

/* static function */
static void rf_write_reg(uint8_t radio_id, uint8_t addr, uint16_t value)
{
	if (radio_id == RF_RADIO_0)
	{	
	    RF0_SPI_CS_DIS();

	    UCB1TXBUF = addr;
	    while (UCB1STAT & UCBUSY);

	    UCB1TXBUF = (uint8_t)((value>>8)&0xFF);
	    while (UCB1STAT & UCBUSY);

	    UCB1TXBUF = (uint8_t)(value&0xFF);
	    while (UCB1STAT & UCBUSY);

	    RF0_SPI_CS_EN();
	}	
}

static uint16_t rf_read_reg(uint8_t radio_id, uint8_t addr)
{
    uint16_t reg_value = 0;

	if (radio_id == RF_RADIO_0)
	{	
		RF0_SPI_CS_DIS();

	    UCB1TXBUF = (addr|0x80);
	    while (UCB1STAT & UCBUSY);

	    UCB1TXBUF = 0x00;
	    while (UCB1STAT & UCBUSY);
	    reg_value = ((UCB1RXBUF<<8)&0xFF00);

	    UCB1TXBUF = 0xFF;
	    while (UCB1STAT & UCBUSY);
	    reg_value |= (UCB1RXBUF&0x00FF);

    	RF0_SPI_CS_EN();
	}
	
    return reg_value;
}

uint16_t bb_reg_value[REG_NUM];
void hal_rf_reg_value_printf(void)
{
	uint8_t i = 0;
	for (i=0; i<REG_NUM; i++)
	{
		bb_reg_value[i] = rf_read_reg(0,rf_default_reg_array[i].addr);
	}
	for(i = 0;i<REG_NUM;i++)
	{
		hal_uart_printf(UART_DEBUG, rf_default_reg_array[i].name);
		hal_uart_printf(UART_DEBUG, "=0x");					
		hal_uart_send_asic(UART_DEBUG, ((bb_reg_value[i]>>8)&0x00ff));
		hal_uart_send_asic(UART_DEBUG, (bb_reg_value[i]&0x00ff));
		hal_uart_printf(UART_DEBUG, "\r\n");
	}	

}

static void rf_cfg(uint8_t radio_id)
{
	for (uint8_t i=0; i<REG_NUM; i++)
	{
		rf_write_reg(radio_id, rf_default_reg_array[i].addr, rf_reg_value[radio_id][i]);
	}
    //获得基带版本�?0x15
	rf_reg_value[radio_id][10] = rf_read_reg(radio_id, rf_default_reg_array[10].addr);
    
    
    //打印DSSS芯片里的寄存器�??
	//hal_rf_reg_value_printf();
}

static void rf_cfg_int(uint8_t radio_id, uint16_t int_id, uint16_t ctrl_type, uint16_t int_type)
{
    uint8_t addr = 0;
    uint16_t value = 0;

	if (radio_id >= RF_RADIO_MAX) return;

    if (int_id == BB_INT_PIN0)
    {
        addr = BB_INTCONF0;
    }
    else
    {
        addr = BB_INTCONF1;
    }

    value = rf_read_reg(radio_id, addr)&0x01FF; /* save the value of existing */

    if (ctrl_type == INT_DISABLE)
    {
        value &= ~int_type;
    }
    else
    {
        value |= int_type;
    }
    rf_write_reg(radio_id, addr, (0xC000|value));
}

/* 中断回调注册函数 */
static bool_t rf_reg_int(uint8_t radio_id, uint16_t int_type, rf_cb_func_t cb_fun_ptr)
{
	if (radio_id >= RF_RADIO_MAX) return PLAT_FALSE;
	
    if (rf_int_reg[radio_id][int_type].cb_fun_ptr == NULL)
    {
        rf_int_reg[radio_id][int_type].cb_fun_ptr = cb_fun_ptr;
        return PLAT_TRUE;
    }
    else
    {
 
        return PLAT_FALSE;
    }
}

static bool_t rf_unreg_int(uint8_t radio_id, uint8_t int_type)
{
	if (radio_id >= RF_RADIO_MAX) return PLAT_FALSE;
	
    rf_int_reg[radio_id][int_type].cb_fun_ptr = PLAT_NULL;
    return PLAT_TRUE;
}

/* RF接收、发送成功中断处理函�? */
static void rf_isr(uint8_t radio_id, uint16_t cap_time)
{
    uint8_t int_type = (uint8_t)(rf_read_reg(radio_id, BB_INTSTATE)&0x0F);

    if (int_type == RF_INT_TYPE_RXOK)
    {
		rf_cfg_int(radio_id, BB_INT_PIN1, INT_ENABLE, REC_DATA_ENABLE);
		if (rf_int_reg[radio_id][int_type].cb_fun_ptr)
		{
			(*(rf_int_reg[radio_id][int_type].cb_fun_ptr))(cap_time);
		}
    }
	else if (int_type == RF_INT_TYPE_TXOK)
	{
		rf_cfg_int(radio_id, BB_INT_PIN1, INT_DISABLE, SEND_OK_ENABLE);
        //rf_cfg_int(radio_id, BB_INT_PIN1, INT_ENABLE, SEND_OK_ENABLE);
		if (rf_int_reg[radio_id][int_type].cb_fun_ptr)
		{	
			(*(rf_int_reg[radio_id][int_type].cb_fun_ptr))(cap_time);
		}       
	}
}

static void rf_cap_sfd_isr(uint8_t radio_id, uint16_t cap_time)
{
	uint8_t int_type = (uint8_t)((rf_read_reg(radio_id, BB_INTSTATE)>>8)&0x0F);

    if (int_type == RF_INT_TYPE_RXSFD)
	{//sfd 接收中断
		rf_cfg_int(radio_id, BB_INT_PIN0, INT_ENABLE, RX_SFD_ENABLE);
		if (rf_int_reg[radio_id][int_type].cb_fun_ptr)
    	{
			(*(rf_int_reg[radio_id][int_type].cb_fun_ptr))(cap_time);
    	}
		//rf_cfg_int(radio_id, BB_INT_PIN0, INT_ENABLE, RX_SFD_ENABLE);
	}	
	else if(int_type == RF_INT_TYPE_TXSFD)
	{//发�?�中�?
		rf_cfg_int(radio_id, BB_INT_PIN0, INT_DISABLE, TX_SFD_ENABLE);
		if (rf_int_reg[radio_id][int_type].cb_fun_ptr)
    	{
			(*(rf_int_reg[radio_id][int_type].cb_fun_ptr))(cap_time);
    	}
	}	
}

/*
 * isr for gpio for irq handler
 *
*/

static void hal_rf_port1_isr(void)
{
	uint32_t status;
	
	status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
	
	if (status & BIT0)
	{
		rf_isr(RF_RADIO_0, 0);
	}
	
	GPIO_clearInterruptFlag(GPIO_PORT_P1, status);
}

/*
 * isr for timer a0 ccr0 for irq handler
 * 定时器的中断调用
*/
static void hal_timerA0_cr0_isr(void)
{
	uint16_t cap_time;
	uint32_t status;
	
	status = Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A0_BASE, 
															TIMER_A_CAPTURECOMPARE_REGISTER_0);
	//获取中断标志�?
	if (status & TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG)
	{   //记录此时，硬中断�? 数据 记录情况
		cap_time = TA0CCR0;
		//清除 定时中断 标志�?
		Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
		//DBG_PRINTF("Capture0%d\r\n", cap_time);
		//将获取到�? 时间数据传给 rf 层sfd中断
		rf_cap_sfd_isr(RF_RADIO_0, cap_time);
	}
	else if (status & TIMER_A_CAPTURE_OVERFLOW)
	{
		TA0CCR0;
		Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
		//DBG_PRINTF("Capture0 Overflow\r\n");
	}
}

