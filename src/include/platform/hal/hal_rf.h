/**
 * provides an abstraction for baseband.
 *
 * @file rf.h
 * @author qhw
 *
 * @addtogroup HAL_RF HAL Baseband Operation
 * @ingroup HAL
 * @{
 */

#ifndef __RF_H
#define __RF_H

#define DSSS_PN128		0
#define DSSS_PN64		1

#define RF_RADIO_0		0
#define RF_RADIO_1		1
#define RF_RADIO_MAX	1

#define RF_STORE_INDEX  0

/* baseband addr define */
#define BB_SNOP             0x00u
#define BB_STATE   		    0x01u
#define BB_AGCCTRL 		    0x02u
#define BB_TXDELAY	 		0x03u
#define BB_MISC	 			0x04u
#define BB_TXPOWER	 		0x05u
#define BB_TXFIFOSET   	    0x06u
#define BB_RXFIFOSET   	    0x07u
#define BB_FIFOVALUE   	    0x08u
#define BB_INTCONF0		    0x09u
#define BB_INTCONF1		    0x0Au
#define BB_INTSTATE   	    0x0Bu
#define BB_CCA 		        0x0Cu
#define BB_NODEID  			0x0Du
#define BB_SHUTDOWN		    0x0Eu
#define BB_RESET   		    0x0Fu
#define BB_TXCONF0 		    0x10u
#define BB_TXCONF1 		    0x11u
#define BB_RXCONF  		    0x12u
#define BB_RFTESTCTRL1 	    0x13u
#define BB_RFTESTCTRL2 	    0x14u
#define BB_VERSION  		0x15u
#define BB_AGCSET          	0x16u
#define BB_CATCH_GATE 		0x17u
#define BB_DIFF_GATE 		0x18u
#define BB_TRACE_GATE 		0x19u
#define BB_LOSE_GATE 		0x1Au
#define BB_43NCTRL 			0x1Cu
#define BB_43RCTRL 			0x1Du
#define BB_SFD 				0x1Eu
#define BB_TXPN_RAM0        0x20u
#define BB_TXPN_RAM1        (BB_TXPN_RAM0+1)
#define BB_TXPN_RAM2        (BB_TXPN_RAM0+2)
#define BB_TXPN_RAM3        (BB_TXPN_RAM0+3)
#define BB_TXPN_RAM4        (BB_TXPN_RAM0+4)
#define BB_TXPN_RAM5        (BB_TXPN_RAM0+5)
#define BB_TXPN_RAM6        (BB_TXPN_RAM0+6)
#define BB_TXPN_RAM7        (BB_TXPN_RAM0+7)
#define BB_RXPN_RAM0		0x40u
#define BB_RXPN_RAM1        (BB_RXPN_RAM0+1)
#define BB_RXPN_RAM2        (BB_RXPN_RAM0+2)
#define BB_RXPN_RAM3        (BB_RXPN_RAM0+3)
#define BB_RXPN_RAM4        (BB_RXPN_RAM0+4)
#define BB_RXPN_RAM5        (BB_RXPN_RAM0+5)
#define BB_RXPN_RAM6        (BB_RXPN_RAM0+6)
#define BB_RXPN_RAM7        (BB_RXPN_RAM0+7)
#define BB_TXFIFO  		    0x70u
#define BB_RXFIFO  		    0x71u

#define INT_DISABLE             0
#define INT_ENABLE              1

#define TX_SFD_ENABLE		        ((0x01<<8)&0x01FF)
#define RX_SFD_ENABLE		        ((0x01<<7)&0x01FF)
#define REC_DATA_ENABLE		        ((0x01<<6)&0x01FF)
#define SEND_OK_ENABLE		        ((0x01<<5)&0x01FF)
#define TXFIFO_UNDERFLOW_ENABLE	    ((0x01<<4)&0x01FF)
#define TXFIFO_OVERFLOW_ENABLE	    ((0x01<<3)&0x01FF)
#define RXFIFO_UNDERFLOW_ENABLE	    ((0x01<<2)&0x01FF)
#define RXFIFO_OVERFLOW_ENABLE	    ((0x01<<1)&0x01FF)
#define CONT_FINISH_ENABLE	        ((0x01<<0)&0x01FF)

/* RF中断类型宏定义 */
#define BB_INT_PIN0							0u
#define BB_INT_PIN1							1u

#define RF_INT_MAX_NUM                      4U

#define RF_INT_TYPE_TXSFD		            0U
#define RF_INT_TYPE_RXSFD		            1U
#define RF_INT_TYPE_RXOK		            2U
#define RF_INT_TYPE_TXOK		            3U
#define RF_INT_TYPE_TXFIFO_UNDERFLOW		4U
#define RF_INT_TYPE_TXFIFO_OVERFLOW			5U
#define RF_INT_TYPE_RXFIFO_UNDERFLOW		6U
#define RF_INT_TYPE_RXFIFO_OVERFLOW			7U
#define RF_INT_TYPE_CONT_FINISH				8U

#define RF_STATE_SHUTDOWN			    	0
#define RF_STATE_IDLE				    	1
#define RF_STATE_RX							2
#define RF_STATE_TX							3

#define REG_NUM					35u
#define GATE_REG_NUM			4u
#define CONF_REG_NUM			3u

#define REG_PN128_GROUP			12u
#define REG_PN128_NUM			8u

#define REG_PN64_GROUP			6u
#define REG_PN64_NUM			4u

#define BB_VER_ASIC_PN			0x8008

#define RF_RSSI_LEVEL(x)	(((x)&0xe000)>>13)
#define RF_RSSI_LQI(x)		((x)&0x1FFFF)

typedef struct
{
	uint8_t *name;
	uint8_t addr;
	uint16_t value;
}bb_reg_info_t;

typedef void (*rf_cb_func_t)(uint16_t);

typedef struct
{
    rf_cb_func_t cb_fun_ptr;
} rf_int_reg_t;

/***********************************************************************************
* CONSTANTS AND DEFINES
*/
#define HAL_RF_RX_STATE					0x04
#define HAL_RF_TX_STATE					0x03
#define HAL_RF_IDLE_STATE				0x02
#define HAL_RF_OFF_STATE				0x01
#define HAL_RF_CON_TX_STATE				0x00

#define HAL_RF_RXOK_INT				    0x00
#define HAL_RF_TXOK_INT				    0x01
#define HAL_RF_RXSFD_INT			    0x02
#define HAL_RF_TXSFD_INT			    0x03
#define HAL_RF_CCA_INT				    0x04
#define HAL_RF_FIFOP_INT			    0x05

extern const bb_reg_info_t rf_default_reg_array[REG_NUM];
/***********************************************************************************
* GLOBAL FUNCTIONS
*/

bool_t hal_rf_init(uint8_t radio_id, uint8_t pn_type, uint8_t pn_group);

void hal_rf_deinit(uint8_t radio_id);

uint8_t hal_rf_get_state(uint8_t radio_id);

bool_t hal_rf_set_state(uint8_t radio_id, uint8_t rf_state);

bool_t hal_rf_write(uint8_t radio_id, const uint8_t *buffer, uint8_t length);

bool_t hal_rf_read(uint8_t radio_id, uint8_t *buffer, uint8_t length);

void hal_rf_flush(uint8_t radio_id);

bool_t hal_rf_cfg_int(uint8_t radio_id, uint16_t int_type, bool_t flag);

bool_t hal_rf_reg_int(uint8_t radio_id, uint16_t int_type, rf_cb_func_t cb);

void hal_rf_reset(uint8_t radio_id);

bool_t hal_rf_unreg_int(uint8_t radio_id, uint8_t int_type);

void hal_rf_get_register(uint8_t radio_id, uint8_t reg_addr, uint16_t *reg_value);

bool_t hal_rf_set_register(uint8_t radio_id, uint8_t reg_addr, uint16_t *reg_value);

uint16_t hal_rf_read_reg(uint8_t radio_id, uint8_t addr);

void hal_rf_write_reg(uint8_t radio_id, uint8_t addr, uint16_t value);

#endif

/**
 * @}
 */