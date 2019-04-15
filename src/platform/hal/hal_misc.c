#include <platform.h>
#include <ti\devices\msp432p4xx\inc\msp.h>
#include <ti\devices\msp432p4xx\inc\system_msp432p401r.h>
#include <ti\devices\msp432p4xx\driverlib\cs.h>
#include <ti\devices\msp432p4xx\driverlib\pcm.h>
#include <ti\devices\msp432p4xx\driverlib\flash.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>
#include <ti\devices\msp432p4xx\driverlib\interrupt.h>
#include <ti\devices\msp432p4xx\driverlib\reset.h>
#include <ti\devices\msp432p4xx\driverlib\sysctl.h>

#define  CM4_NVIC_ST_CTRL    (*((volatile uint32_t *)0xE000E010uL))

static void hal_set_clock(void);
static void hal_output_clock(void);

void hal_board_init(void)
{
	//关闭中断
	Interrupt_disableMaster();
    //时钟设置
    hal_set_clock();
	//输出时钟
	hal_output_clock();
    //时钟获得更新
    SystemCoreClockUpdate();
    //set prigroup no pre-emption prio and 4bits sub prio 0~255
	Interrupt_setPriorityGrouping(0);
	//开启中断
    Interrupt_enableMaster();
}

void hal_board_reset(void)
{
	//关闭中断
	Interrupt_disableMaster();
    //reset system tick
	CM4_NVIC_ST_CTRL = 0;
    //product hard reset
    //ResetCtl_initiateHardReset();
	SysCtl_rebootDevice();
	//ResetCtl_initiateSoftReset();
	while(1);
}

void hal_board_soft_reset(void)
{
	//关闭中断
	Interrupt_disableMaster();
    //reset system tick
	CM4_NVIC_ST_CTRL = 0;
	
	ResetCtl_initiateSoftReset();
	while(1);
}

static void hal_set_clock(void)
{
    PCM_setPowerState(PCM_AM_LDO_VCORE1);
    
    PCM_setPowerMode(PCM_LDO_MODE);

	FlashCtl_setWaitState(FLASH_BANK0, 1);
	FlashCtl_setWaitState(FLASH_BANK1, 1);

    // set LFXT pin as second function
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_PJ, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    // set HFXT pin as second function
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_PJ, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	CS_enableClockRequest(CS_MCLK);
    CS_enableClockRequest(CS_SMCLK);
    CS_enableClockRequest(CS_HSMCLK);
    CS_enableClockRequest(CS_ACLK);
	//set clock
	CS_setExternalClockSourceFrequency(32768, 48000000);
    
	if (CS_startHFXTWithTimeout(0, 100000))
    {
    	if (!CS_startLFXTWithTimeout(0, 1000000))
    	DBG_ASSERT(0);
    }
	else
	{
		DBG_ASSERT(0);
	}    
    
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);

	CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);

	CS_initClockSignal(CS_HSMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_2);

	CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);

	CS_initClockSignal(CS_BCLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

static void hal_output_clock(void)
{
	// set MCLK pin as second function
    //GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    // set HSMCLK pin as second function
    //GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION); 
	// set SMCLK pin as second function
    //GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);
   	// set ACLK pin as second function
    //GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
}

void hal_hardfault_print(uint32_t *args, uint32_t arg_lr)
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
    uint32_t cfsr = SCB->CFSR;
    uint32_t bus_fault_addr = SCB->BFAR;
    uint32_t mem_fault_addr = SCB->MMFAR;
    
    r0 = (uint32_t)args[0];
    r1 = (uint32_t)args[1];
    r2 = (uint32_t)args[2];
    r3 = (uint32_t)args[3];
    r12 = (uint32_t)args[4];
    lr = (uint32_t)args[5];
	pc = (uint32_t)args[6];
	psr = (uint32_t)args[7];
PRINT_ENV:
	DBG_CRITICAL("[************HardFalt**************]\r\n");
	DBG_CRITICAL("Stack Frame\r\n");
	DBG_CRITICAL("SP=%0.8X\r\n", args);
	DBG_CRITICAL("R0=%0.8X\r\n", r0);
	DBG_CRITICAL("R1=%0.8X\r\n", r1);
	DBG_CRITICAL("R2=%0.8X\r\n", r2);
	DBG_CRITICAL("R3=%0.8X\r\n", r3);
	DBG_CRITICAL("R12=%0.8X\r\n", r12);
	DBG_CRITICAL("LR=%0.8X\r\n", lr);
	DBG_CRITICAL("PC=%0.8X\r\n", pc);
	DBG_CRITICAL("PSR=%0.8X\r\n", psr);
	
	DBG_CRITICAL("[FSR/FAR]\r\n");
	DBG_CRITICAL("CFSR=%0.8X\r\n", cfsr);
	DBG_CRITICAL("HFSR=%0.8X\r\n", SCB->HFSR);
	DBG_CRITICAL("DFSR=%0.8X\r\n", SCB->DFSR);
	DBG_CRITICAL("AFSR=%0.8X\r\n", SCB->AFSR);

	if (cfsr& 0x8000) DBG_CRITICAL("MMFAR=%0.8X\r\n", mem_fault_addr);
	if (cfsr& 0x8000) DBG_CRITICAL("BFAR=%0.8X\r\n", bus_fault_addr);

	DBG_CRITICAL("[MISC]\r\n");
	DBG_CRITICAL("LR/EXC_RETURN=%0.8X\r\n", arg_lr);
    while(1)
    {
        delay_ms(5000);
        goto PRINT_ENV;
    }
}
