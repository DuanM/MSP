#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\uart.h>
#include <ti\devices\msp432p4xx\driverlib\debug.h>
#include <ti\devices\msp432p4xx\driverlib\interrupt.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>
#include <ti\devices\msp432p4xx\driverlib\cs.h>
#include <ti\devices\msp432p4xx\driverlib\pmap.h>

#define UART_A0 	0
#define UART_A1 	1
#define UART_A2 	2

typedef struct
{	
	bool_t valid_flag;
	uint8_t fifo_thred;
	fpv_t func;
	queue_t queue;
}uart_entity_t;

static uart_entity_t uart_entity[UART_NUM];

static void hal_uart_a0_irq_handler(void);
static void hal_uart_a1_irq_handler(void);
static void hal_uart_a2_irq_handler(void);

void hal_uart_init(uint8_t uart_id, uint32_t baud_rate)
{
	bool_t res;
	eUSCI_UART_Config uart_config;
	uint32_t smclk = 0;
	fp64_t N  = 0;
		
	if (uart_id == UART_A0)
	{		
		GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

		mem_clr(&uart_config, sizeof(eUSCI_UART_Config));

		smclk = CS_getSMCLK();
		N = (fp64_t)smclk/(baud_rate*16);		
		
		uart_config.uartMode = EUSCI_A_UART_MODE;
		uart_config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
		uart_config.clockPrescalar = (uint32_t)(N);
		uart_config.firstModReg = (uint32_t)((N-uart_config.clockPrescalar)*16);
		uart_config.secondModReg = 0x20;
		uart_config.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
		uart_config.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
		uart_config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
		uart_config.parity = EUSCI_A_UART_NO_PARITY;
		
		res = UART_initModule(EUSCI_A0_BASE, &uart_config);

		UART_enableModule(EUSCI_A0_BASE);

		uart_entity[UART_A0].valid_flag = PLAT_TRUE;
		res = queue_init(&uart_entity[UART_A0].queue, UART_RECV_BUF_SIZE);
		DBG_ASSERT(res != PLAT_FALSE);
	}
	else if (uart_id == UART_A1)
	{
		GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
  
		mem_clr(&uart_config, sizeof(eUSCI_UART_Config));
        
		smclk = CS_getSMCLK();
		N = (fp64_t)smclk/(baud_rate*16);		
		
		uart_config.uartMode = EUSCI_A_UART_MODE;
		uart_config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
		uart_config.clockPrescalar = (uint32_t)(N);
		uart_config.firstModReg = (uint32_t)((N-uart_config.clockPrescalar)*16);
		uart_config.secondModReg = 0x20;
		uart_config.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
		uart_config.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
		uart_config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
		uart_config.parity = EUSCI_A_UART_NO_PARITY;
		
		res = UART_initModule(EUSCI_A1_BASE, &uart_config);
		
		UART_enableModule(EUSCI_A1_BASE);
		if(!uart_entity[UART_A1].valid_flag)
		{
			uart_entity[UART_A1].valid_flag = PLAT_TRUE;
			res = queue_init(&uart_entity[UART_A1].queue, UART_RECV_BUF_SIZE);
			DBG_ASSERT(res != PLAT_FALSE);
		}
	}
	else if (uart_id == UART_A2)
	{
		GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

		mem_clr(&uart_config, sizeof(eUSCI_UART_Config));

		smclk = CS_getSMCLK();
		N = (fp64_t)smclk/(baud_rate*16);		
		
		uart_config.uartMode = EUSCI_A_UART_MODE;
		uart_config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
		uart_config.clockPrescalar = (uint32_t)(N);
		uart_config.firstModReg = (uint32_t)((N-uart_config.clockPrescalar)*16);
		uart_config.secondModReg = 0x20;
		uart_config.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
		uart_config.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
		uart_config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
		uart_config.parity = EUSCI_A_UART_NO_PARITY;
		
		res = UART_initModule(EUSCI_A2_BASE, &uart_config);
		
		UART_enableModule(EUSCI_A2_BASE);
		
		uart_entity[UART_A2].valid_flag = PLAT_TRUE;
		res = queue_init(&uart_entity[UART_A2].queue, UART_RECV_BUF_SIZE);
		DBG_ASSERT(res != PLAT_FALSE);
	}
}

void hal_uart_send_char(uint8_t uart_id, uint8_t data)
{ 
	if (uart_id == UART_A0)
	{
        uint16_t j=0;
		/* If interrupts are not used, poll for flags */ 
	    if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
            {
              j++;  
              if(j>0xfff){break;}
            }; 
	    EUSCI_A_CMSIS(EUSCI_A0_BASE)->TXBUF = data;
	}
	else if (uart_id == UART_A1)
	{
        uint16_t j=0;
		/* If interrupts are not used, poll for flags */ 
	    if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A1_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A1_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
            {
              j++;  
              if(j>0xfff){break;}
            }; 
	    EUSCI_A_CMSIS(EUSCI_A1_BASE)->TXBUF = data;
	}
	else if (uart_id == UART_A2)
	{
        uint16_t j=0;
		/* If interrupts are not used, poll for flags */ 
	    if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A2_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A2_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
            {
              j++;  
              if(j>0xfff){break;}
            }; 
	    EUSCI_A_CMSIS(EUSCI_A2_BASE)->TXBUF = data;
	}
}

void hal_uart_send_string(uint8_t uart_id, uint8_t *string, uint32_t length)
{
	if (uart_id == UART_A0)
	{
        uint16_t j=0;
		for(uint32_t i=0; i<length; i++)
		{
			/* If interrupts are not used, poll for flags */ 
	    	if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        	while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
                {
                  j++;  
                  if(j>0xfff){break;}
                };

            j = 0;
	    	EUSCI_A_CMSIS(EUSCI_A0_BASE)->TXBUF = *(string+i);
		}
	}
	else if (uart_id == UART_A1)
	{
        uint16_t j=0;
		for(uint32_t i=0; i<length; i++)
		{
			/* If interrupts are not used, poll for flags */ 
	    	if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A1_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        	while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A1_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
                {
                  j++;  
                  if(j>0xfff){break;}
                };
            
            j = 0;
	    	EUSCI_A_CMSIS(EUSCI_A1_BASE)->TXBUF = *(string+i);
		}
	}
	else if (uart_id == UART_A2)
	{
        uint16_t j=0;
		for(uint32_t i=0; i<length; i++)
		{
			/* If interrupts are not used, poll for flags */ 
	    	if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A2_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        	while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A2_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
                {
                  j++;  
                  if(j>0xfff){break;}
                };
            
            j = 0;
	    	EUSCI_A_CMSIS(EUSCI_A2_BASE)->TXBUF = *(string+i);
		}
	}
}


void hal_uart_send_asic(uint8_t id, uint8_t value)
{
	int8_t temp;
	
	temp = (value>>4)&0x0f;
	if(temp>0x09&&temp<0x10)
	{
		hal_uart_send_char(id, (temp-0x0a)+'a');
	}
	else if(temp>=0x00&&temp<=0x09)
	{
		hal_uart_send_char(id, temp+'0');
	}
	
	temp = value&0x0f;
	if(temp>0x09&&temp<0x10)
	{
		hal_uart_send_char(id, (temp-0x0a)+'a');
	}
	else if(temp>=0x00&&temp<=0x09)
	{
		hal_uart_send_char(id, temp+'0');
	}	
}

void hal_uart_printf(uint8_t uart_id, uint8_t *string)
{
    uint32_t i=0;
	if (uart_id == UART_A0)
	{
        uint16_t j=0;
		while(*(string+i) != 0x00)
		{
			/* If interrupts are not used, poll for flags */
	    	if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        	while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
                {
                  j++;  
                  if(j>0xfff){break;}
                };
            
            j=0;
            EUSCI_A_CMSIS(EUSCI_A0_BASE)->TXBUF = *(string+i);
                
            i++;
		}
	}
	else if (uart_id == UART_A1)
	{
        uint16_t j=0;
		while(*(string+i) != 0x00)
		{
			/* If interrupts are not used, poll for flags */
	    	if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A1_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        	while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A1_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
                {
                  j++;  
                  if(j>0xfff){break;}
                };
            
            j=0;
			EUSCI_A_CMSIS(EUSCI_A1_BASE)->TXBUF = *(string+i);

            i++;
		}
	}
	else if (uart_id == UART_A2)
	{
       uint16_t j=0;
		while(*(string+i) != 0x00)
		{
			/* If interrupts are not used, poll for flags */
	    	if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A2_BASE)->IE, EUSCI_A_IE_TXIE_OFS))
	        	while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A2_BASE)->IFG, EUSCI_A_IFG_TXIFG_OFS))
                {
                  j++;  
                  if(j>0xfff){break;}
                }; 
            
            j = 0;
			EUSCI_A_CMSIS(EUSCI_A2_BASE)->TXBUF = *(string+i);

            i++;
		}
	}
}

uint32_t hal_uart_read(uint8_t uart_id, void *buffer, uint32_t len)
{
	uint32_t i = 0;
    queue_data_t e;
    uint8_t *buf = (uint8_t *)buffer;
 
    if (queue_length(&uart_entity[uart_id].queue) >= len)
    {
        for (i=0; i<len; i++)
        {
            delete_queue(&uart_entity[uart_id].queue, &e);
            buf[i] = e.data;
        }
    }
    else
    {
        while((queue_length(&uart_entity[uart_id].queue) != 0) && (i<len))
        {
            delete_queue(&uart_entity[uart_id].queue, &e);
            buf[i++] = e.data;
        }
    } 

    return i;
}

uint8_t hal_uart_read_poll(uint8_t uart_id)
{
	uint16_t j = 0;
	
	if (uart_id == UART_A0)
	{
		/* If interrupts are not used, poll for flags */
	    if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IE, EUSCI_A_IE_RXIE_OFS))
	    {
		    while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A0_BASE)->IFG, EUSCI_A_IFG_RXIFG_OFS))
		    {
				j++;
				
				if(j>0xfff)
				{					
					return 0;
				}
			};
			return EUSCI_A_CMSIS(EUSCI_A0_BASE)->RXBUF;
	    }	
	}
	else if (uart_id == UART_A1)
	{
		/* If interrupts are not used, poll for flags */
	    if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A1_BASE)->IE, EUSCI_A_IE_RXIE_OFS))
	    {
		    while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A1_BASE)->IFG, EUSCI_A_IFG_RXIFG_OFS))
		    {
				j++;
				
				if(j>0xfff)
				{					
					return 0;
				}
			};
			return EUSCI_A_CMSIS(EUSCI_A1_BASE)->RXBUF;
	    }
	}
	else if (uart_id == UART_A2)
	{
		/* If interrupts are not used, poll for flags */
	    if (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A2_BASE)->IE, EUSCI_A_IE_RXIE_OFS))
	    {
		    while (!BITBAND_PERI(EUSCI_A_CMSIS(EUSCI_A2_BASE)->IFG, EUSCI_A_IFG_RXIFG_OFS))
		    {
				j++;
				
				if(j>0xfff)
				{					
					return 0;
				}
			};
			return EUSCI_A_CMSIS(EUSCI_A2_BASE)->RXBUF;
	    }
	}
    return 0;
}

void hal_uart_rx_set_hiz_state(uint8_t uart_id)
{
	if (uart_id == UART_A0)
	{
		GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN3);
		GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN2);		
	}
	else if (uart_id == UART_A1)
	{
		GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN3);
		GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN2);		
	}
	else if (uart_id == UART_A2)
	{
		GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN3);
		GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN2);
	}
}

void hal_uart_rx_irq_enable(uint8_t uart_id, uint8_t fifo_thred, fpv_t func)
{	
	if (uart_id == UART_A0)
	{
		uart_entity[UART_A0].func = func;
		
		if (fifo_thred<UART_RECV_BUF_OVF_SIZE)
        {
        	uart_entity[UART_A0].fifo_thred = fifo_thred;
		}
		else
		{
			uart_entity[UART_A0].fifo_thred = UART_RECV_BUF_OVF_SIZE;
		}
		
		UART_registerInterrupt(EUSCI_A0_BASE, hal_uart_a0_irq_handler);
		
		UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
	}
	else if (uart_id == UART_A1)
	{
		uart_entity[UART_A1].func = func;
		
		if (fifo_thred<UART_RECV_BUF_OVF_SIZE)
        {
        	uart_entity[UART_A1].fifo_thred = fifo_thred;
		}
		else
		{
			uart_entity[UART_A1].fifo_thred = UART_RECV_BUF_OVF_SIZE;
		}
		
		UART_registerInterrupt(EUSCI_A1_BASE, hal_uart_a1_irq_handler);

		UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
	}
	else if (uart_id == UART_A2)
	{
		uart_entity[UART_A2].func = func;
		
		if (fifo_thred<UART_RECV_BUF_OVF_SIZE)
        {
        	uart_entity[UART_A2].fifo_thred = fifo_thred;
		}
		else
		{
			uart_entity[UART_A2].fifo_thred = UART_RECV_BUF_OVF_SIZE;
		}
		
		UART_registerInterrupt(EUSCI_A2_BASE, hal_uart_a2_irq_handler);

		UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
	}
}

void hal_uart_rx_irq_disable(uint8_t uart_id)
{
	if (uart_id == UART_A0)
	{
		UART_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);        
		
		UART_unregisterInterrupt(EUSCI_A0_BASE);
        
        uart_entity[UART_A0].func = PLAT_NULL;
	}
	else if (uart_id == UART_A1)
	{
		UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);        
		
		UART_unregisterInterrupt(EUSCI_A1_BASE);
        
        uart_entity[UART_A1].func = PLAT_NULL;
	}
	else if (uart_id == UART_A2)
	{
		UART_disableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);        
		
		UART_unregisterInterrupt(EUSCI_A2_BASE);
        
        uart_entity[UART_A2].func = PLAT_NULL;
	}
}

static void hal_uart_a0_irq_handler(void)
{
	uint8_t int_status;	
	queue_data_t e;
	bool_t res;
	
	int_status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

	UART_clearInterruptFlag(EUSCI_A0_BASE, int_status);

	if (int_status == EUSCI_A_UART_RECEIVE_INTERRUPT)
	{		
		e.data = EUSCI_A_CMSIS(EUSCI_A0_BASE)->RXBUF;		

		if (uart_entity[UART_A0].func != PLAT_NULL)
		{
			res = enter_queue(&uart_entity[UART_A0].queue, e);
		
			if (res == PLAT_FALSE 
				|| queue_length(&uart_entity[UART_A0].queue) >= uart_entity[UART_A0].fifo_thred)
			{			 
                  (* (uart_entity[UART_A0].func) )(); 
			}
		}	
	}	
}

static void hal_uart_a1_irq_handler(void)
{
	uint8_t int_status;	
	queue_data_t e;
	bool_t res;
	
	int_status = UART_getEnabledInterruptStatus(EUSCI_A1_BASE);

	UART_clearInterruptFlag(EUSCI_A1_BASE, int_status);

	if (int_status == EUSCI_A_UART_RECEIVE_INTERRUPT)
	{		
		e.data = EUSCI_A_CMSIS(EUSCI_A1_BASE)->RXBUF;		

		if (uart_entity[UART_A1].func != PLAT_NULL)
		{
			res = enter_queue(&uart_entity[UART_A1].queue, e);
			
			if (res == PLAT_FALSE 
				|| queue_length(&uart_entity[UART_A1].queue) >= uart_entity[UART_A1].fifo_thred)
			{	
                  (* (uart_entity[UART_A1].func) )();
			}
		}	
	}	
}

static void hal_uart_a2_irq_handler(void)
{
	uint8_t int_status;	
	queue_data_t e;
	bool_t res;
	
	int_status = UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

	UART_clearInterruptFlag(EUSCI_A2_BASE, int_status);

	if (int_status == EUSCI_A_UART_RECEIVE_INTERRUPT)
	{		
		e.data = EUSCI_A_CMSIS(EUSCI_A2_BASE)->RXBUF;		

		if (uart_entity[UART_A2].func != PLAT_NULL)
		{
			res = enter_queue(&uart_entity[UART_A2].queue, e);
			
			if (res == PLAT_FALSE 
				|| queue_length(&uart_entity[UART_A2].queue) >= uart_entity[UART_A2].fifo_thred)
			{				 
                  (* (uart_entity[UART_A2].func) )(); 
			}
		}	
	}	
}



