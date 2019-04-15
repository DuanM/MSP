
#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\interrupt.h>
#include <ti\devices\msp432p4xx\driverlib\timer_a.h>
#include <ti\devices\msp432p4xx\driverlib\timer32.h>
#include <ti\devices\msp432p4xx\driverlib\rtc_c.h>
#include <ti\devices\msp432p4xx\driverlib\cs.h>


#define TIMER_MAX       MAX_TIMER_NUM
   
#define TA2CCR1_		0x40000814
#define TA2CCR2_		0x40000816
#define TA2CCR3_		0x40000818
#define TA2CCR4_		0x4000081A

#define TA2CCTL1_		0x40000804
#define TA2CCTL2_		0x40000806
#define TA2CCTL3_		0x40000808
#define TA2CCTL4_		0x4000080A

//mclk=48000000,prescale=16
#define ONE_US_EQ_TICK_NUM	3

#pragma pack(1)

typedef struct
{
	uint32_t ticks;
	uint32_t loops;
	fpv_t fun_ptr;
	bool_t used;	
}timer_reg_t;

#pragma pack()

volatile static timer_reg_t timer_reg[TIMER_MAX];

const uint32_t p_timer[TIMER_MAX] = {TA2CCR1_, TA2CCR2_, TA2CCR3_, TA2CCR4_};

const uint32_t p_timer_ctrl[TIMER_MAX] = {TA2CCTL1_, TA2CCTL2_, TA2CCTL3_, TA2CCTL4_};

static void hal_timer_a2_cr1_6_isr(void);
static void hal_timer_a2_cr0_isr(void);

static void hal_timer_tick_init( void )
{
    /* ACLK; Continous up; Timer A3 counter clear */
    TA2CTL |= TASSEL_1 + MC_2 + TACLR;
}

void hal_timer_init(void)
{	
    hal_timer_tick_init();
	
	Timer_A_registerInterrupt(TIMER_A2_BASE, 
                              TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,
                              hal_timer_a2_cr1_6_isr);
	
	Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_16, TIMER32_32BIT, TIMER32_PERIODIC_MODE);
	Timer32_disableInterrupt(TIMER32_0_BASE);
	Timer32_haltTimer(TIMER32_0_BASE);
}

uint8_t hal_timer_alloc(uint64_t time_us, fpv_t func)
{
    uint16_t cap_time = TIMER_A_CMSIS(TIMER_A2_BASE)->R;
    uint16_t timer_id = 0;
    uint64_t tick_tmp = 0;
    uint32_t loop_tmp = 0;
	
    if (time_us == 0)
    {
      return 0;
    }
	
    for (timer_id=0; timer_id<TIMER_MAX; timer_id++)
    {
        if (timer_reg[timer_id].used == PLAT_FALSE)
        {
           timer_reg[timer_id].used = PLAT_TRUE;
           timer_reg[timer_id].fun_ptr = func;
           break;
        }
    }

    if (timer_id == TIMER_MAX)
    {
        return 0;
    }

    tick_tmp = (uint64_t) ((fp32_t)time_us*0.032768 + 0.5);

    /* Compute the integral number of 65536 tick delays   */
    if (tick_tmp > 0xFFFF)
    {
        loop_tmp = (uint32_t)(tick_tmp >> 16);
    }
    /* Obtain  the fractional number of ticks             */
    tick_tmp = tick_tmp & 0xFFFFL;
    timer_reg[timer_id].ticks = (uint16_t)tick_tmp;

	/* important */
	if (tick_tmp == 0)
	{
		loop_tmp--;
	}
	
    if (loop_tmp > 0)
    {
        timer_reg[timer_id].loops = loop_tmp;
        TA2CTL &= ~MC__CONTINUOUS;
        *(volatile uint16_t *)p_timer[timer_id] = cap_time; /* 1.99ms */
        *(volatile uint16_t *)p_timer_ctrl[timer_id] &= ~CCIFG;
        *(volatile uint16_t *)p_timer_ctrl[timer_id] |= CCIE;
        TA2CTL |= MC__CONTINUOUS;
    }
    else
    {
        timer_reg[timer_id].loops = 0;
        TA2CTL &= ~MC__CONTINUOUS;
        *(volatile uint16_t *)p_timer[timer_id] = tick_tmp + cap_time;
        *(volatile uint16_t *)p_timer_ctrl[timer_id] &= ~CCIFG;
        *(volatile uint16_t *)p_timer_ctrl[timer_id] |= CCIE;
        TA2CTL |= MC__CONTINUOUS;
    }
    return timer_id+1;
}

uint8_t hal_timer_free(uint8_t timer_id)
{
	if (timer_id <= 0)
	{
		return 0;
	}
	TA2CTL &= ~MC__CONTINUOUS;
	*(volatile uint16_t *)p_timer_ctrl[timer_id-1] &= ~(CCIE+CCIFG);
	*(volatile uint16_t *)p_timer[timer_id-1] = 0;
	TA2CTL |= MC__CONTINUOUS;

	timer_reg[timer_id-1].ticks     = 0;
	timer_reg[timer_id-1].loops     = 0;
	timer_reg[timer_id-1].fun_ptr   = PLAT_NULL;
	timer_reg[timer_id-1].used      = PLAT_FALSE;
	return 0;
}

void delay_us(uint32_t us)
{
	uint32_t during_cnt;
	OSEL_DECL_CRITICAL();
	
	if (us == 0 || us > 1000) return;
		
	during_cnt = us*ONE_US_EQ_TICK_NUM;
	
	OSEL_ENTER_CRITICAL();
	
	Timer32_setCount(TIMER32_0_BASE, during_cnt-1);
	
	Timer32_startTimer(TIMER32_0_BASE, PLAT_TRUE);
	
	OSEL_EXIT_CRITICAL();
	
	while(Timer32_getValue(TIMER32_0_BASE));
	
#if 0
	if (us == 0 || us > 10000000) return;
		
	during_cnt = us*ONE_US_EQ_TICK_NUM;

	Timer32_setCount(TIMER32_0_BASE, during_cnt-1);

	Timer32_startTimer(TIMER32_0_BASE, PLAT_TRUE);

	while(Timer32_getValue(TIMER32_0_BASE));
#endif
	
}

void delay_ms(uint32_t ms)
{
	for(uint32_t count = 0; count < ms; count++)
	{
		uint32_t sub_count = 4000;
		do
		{
			__no_operation();
	        __no_operation();
	        __no_operation();
	        __no_operation();
	        __no_operation();
	    }while(sub_count--);		
	}	
	
#if 0
    uint32_t i;
	if (ms == 0) return;

	if (ms<=1000)
	{
		delay_us(ms*1000);
	}
	else
	{
		for (i=0; i<ms; i++)
		{
			delay_us(1000);
		} 
	}	
#endif
}


#define INVALID_INDEX 0xFF
/*
 * isr for timera3 ccr0
 *
*/
static void hal_timer_a2_cr0_isr(void)
{
	
}

/*
 * isr for timea2 ccr1-ccr6 32.768KHz
 */
static void hal_timer_a2_cr1_6_isr(void)
{
    uint16_t tbr = TIMER_A_CMSIS(TIMER_A2_BASE)->R;
    uint16_t tmp;
    uint8_t index = INVALID_INDEX;

    switch(TIMER_A_CMSIS(TIMER_A2_BASE)->IV)
    {
    case  2:    index = 0;  break;
    case  4:    index = 1;  break;
    case  6:    index = 2;  break;
    case  8:    index = 3;  break;
    //case 10:    index = 4;  break;
    //case 12:    index = 5;  break;
    case 14:    break;
    default:    break;
    }

    //DBG_ASSERT(index != INVALID_INDEX);
    if (index == INVALID_INDEX)
    {
        return;
    }

    if (timer_reg[index].loops > 1)
    {
        timer_reg[index].loops--;
    }
    else if (timer_reg[index].loops == 1)
    {
        timer_reg[index].loops--;
        tmp = timer_reg[index].ticks;

        TA2CTL &= ~MC__CONTINUOUS;
        *(volatile uint16_t *)p_timer[index] = tmp + tbr;
        TA2CTL |= MC__CONTINUOUS;

    }
    else if (timer_reg[index].loops == 0)
    {
        timer_reg[index].ticks = 0;
        TA2CTL &= ~MC__CONTINUOUS;
        *(volatile uint16_t *)p_timer_ctrl[index] &= ~(CCIE+CCIFG);
         TA2CTL |= MC__CONTINUOUS;
        (* (timer_reg[index].fun_ptr) )();
    }
}


