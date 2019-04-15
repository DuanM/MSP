#include <ucos_ii.h>
#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\interrupt.h>


extern uint32_t SystemCoreClock;         /*!< System Clock Frequency (Core Clock) */

void osel_init(void)
{
	osel_task_init();
    osel_event_init();

	Interrupt_registerInterrupt(FAULT_PENDSV, OS_CPU_PendSVHandler);
	
   	Interrupt_registerInterrupt(FAULT_SYSTICK, OS_CPU_SysTickHandler);

	OSInit();
}

void osel_start(void)
{
	OSStart();
}

void osel_systick_init(void)
{
	OS_CPU_SysTickInitFreq(SystemCoreClock);
}

void osel_systick_delay(uint32_t tick)
{
    OSTimeDly(tick);
}

uint32_t osel_systick_get(void)
{
    return OSTimeGet();
}
