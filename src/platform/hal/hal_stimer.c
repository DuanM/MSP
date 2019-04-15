#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\timer_a.h>
#include <List.h>


/*********************************************/
typedef enum{
	TIMER_EXPECT_OFINT,
	TIMER_EXPECT_CMPINT,
} htimer_state_t;


/* static variables */
volatile htimer_sw_t sw_time;

list_t free_timer;
list_t queued_timer;
htimer_t htimer_arrary[MAX_TIMERS];
uint8_t htimer_count;
volatile htimer_state_t htimer_state;
htimer_controler_t htimer;


htimer_err_t htimer_now (time_t* now);
void compare_interrupt_handler (bool_t internal);
void set_next_timer (bool_t cancel);
void overflow_interrupt_handler (void);

static void hal_timerA0_cr1_2_isr(void);

//use the hidden api, list_entry_decap
htimer_t* htimer_alloc()
{
	htimer_t *atimer;
	atimer = list_entry_front_get(&free_timer,htimer_t,list);
	if (atimer != 0){
		htimer_count++;
	}
	return atimer;
}


//return the sbuf back to the free sbuf list
void htimer_free(htimer_t* tid)
{
	if (tid == NULL){
		return;
	}	
	//osel_memset(tid, 0, sizeof(htimer_t));

	htimer_count--;
	list_behind_put(&(tid->list),&free_timer);
	return;
}

bool_t htimer_compare(const htimer_t* ta, const htimer_t* tb)
{
	if (ta->ftime.s.sw == tb->ftime.s.sw)
	{
		if (ta->ftime.w < tb->ftime.w)
		{
			return PLAT_TRUE;
		}
		else
		{
			return PLAT_FALSE;
		}
	}

	if ((htimer_sw_t)(ta->ftime.s.sw - tb->ftime.s.sw) > HTIMER_HALF_TURNOVER)
	{
		return PLAT_TRUE;
	}
	else
	{
		return PLAT_FALSE;
	}
}

//定时计数器 启动
void hal_htimer_start()
{
	DBG_TIMER_INIT();
	
    TIMER_START();
	HTIMER_SET_COMPREG(0x0);
	HTIMER_CLEAR_FLG();	
	HTIMER_COMPINT_ENABLE();
}

void htimer_deinit(void)
{
	TIMER_STOP();
	HTIMER_CLEAR_FLG();
	HTIMER_COMPINT_DISABLE();
	return;
}

uint8_t dbgg2;


//计数器取消计数
htimer_err_t htimer_cancel (htimer_t** tidd)
{
	HAL_DECL_CRITICAL();
	htimer_hw_t now2;	
	htimer_hw_t now1 = HTIMER_HWCOUNT();
	htimer_t* tid = *tidd;
	
	htimer_t* ntime;
	DBG_TIMER(0x60);
	HAL_ENTER_CRITICAL();

	if (tid == NULL)
	{
		DBG_TIMER(0x61);
		HAL_EXIT_CRITICAL();
		*tidd = NULL;
		return TIMER_ERROR_INVALID_INPUT;
	}

	ntime = list_entry_get_head(&queued_timer,htimer_t,list);
	if (ntime == NULL)
	{
		DBG_TIMER(0x69);
		tid = NULL;
		HAL_EXIT_CRITICAL();
		*tidd = NULL;
		return TIMER_ERROR_INVALID_INPUT;
	}
		
	if (tid == ntime)
	{
		list_del(&tid->list);
		htimer_free(tid);

		if (tid->ftime.s.sw == sw_time)
		{	
			if(htimer_state != TIMER_EXPECT_CMPINT)
			{
				dbgg2++;
			}
			
			HTIMER_SET_COMPREG(HTIMER_HWCOUNT()-2);
			if ( HTIMER_FLG())
			{
				HTIMER_CLEAR_FLG();
			}

			now2 = HTIMER_HWCOUNT();			
			if (now2 < now1)
			{
				DBG_TIMER(0x62);
				overflow_interrupt_handler();
			}
			else
			{
				if(now2 + TIMER_SET_TH2 < now1)
				{
					dbgg2 = 0;
					while(HTIMER_HWCOUNT()!=0)
					{
						dbgg2++;
						if (dbgg2 > 100){
							dbgg2++;
						}
					}
					DBG_TIMER(0x63);
					overflow_interrupt_handler();			
				}
				else
				{
					DBG_TIMER(0x64);
					set_next_timer(PLAT_FALSE);
				}
			}
		}
		else
		{
			DBG_TIMER(0x63);
		}
		tid = NULL;
		HAL_EXIT_CRITICAL();
		*tidd = NULL;
		return TIMER_SUCCESS;
	}
	else
	{
		//check if the item is in the list
		list_t *pos;
		bool_t inlist = PLAT_FALSE;
		list_for_each(pos,queued_timer.next)
		{
			if(pos == &(tid->list))
			{
				inlist = PLAT_TRUE;
				break;
			}
		}

		if (inlist)
		{
			list_del(&tid->list);
			htimer_free(tid);
	
			DBG_TIMER(0x64);
			tid = NULL;
			HAL_EXIT_CRITICAL();
            *tidd = NULL;
			return TIMER_SUCCESS;
		}
		else
		{
			DBG_TIMER(0x65);
			tid = NULL;
			HAL_EXIT_CRITICAL();
            *tidd = NULL;
			return TIMER_ERROR_INVALID_INPUT;
		}
	}
}


//获取计数器 现在时间
htimer_err_t htimer_now (time_t* now)
{
	HAL_DECL_CRITICAL();

	HAL_ENTER_CRITICAL();
	now->s.hw = HTIMER_HWCOUNT();//获取 硬件定时器的计数
	now->s.sw = sw_time;//已过的计数超频 个数
	if (HTIMER_FLG()&& (htimer_state == TIMER_EXPECT_OFINT))
	{//如果现在仍然有超频 个数加1
		now->s.sw++;
	}
	HAL_EXIT_CRITICAL();
	return TIMER_SUCCESS;
}

//设置计数器的中断 函数
htimer_t* htimer_at (time_t ftime, tcbf_t func, htimer_err_t* err)
{
	HAL_DECL_CRITICAL();

	time_t now;
	htimer_t* tid;
	DBG_TIMER(0x10);

	HAL_ENTER_CRITICAL();
	htimer_now(&now); //获取现在的计数

	if (func == NULL)
	{
		*err = TIMER_ERROR_INVALID_INPUT;
		HAL_EXIT_CRITICAL();
		DBG_TIMER(0x1a);
		return NULL;		
	}

	/* software time have passed*/
	if ((htimer_sw_t)(ftime.s.sw - now.s.sw) > HTIMER_HALF_TURNOVER)
	{//设置时间 超频
		*err = TIMER_ERROR_INVALID_INPUT;
		HAL_EXIT_CRITICAL();
		DBG_TIMER(0x11);
		return NULL;
	}

	/* its in the current interval */
	if (ftime.s.sw == now.s.sw) 
	{//设置  计数   数值 等于	    现有时间 数值
		/* but the hardware time has passed*/
		if (ftime.s.hw < now.s.hw)
		{//硬件计数器超频个数 少于现有计数器超频个数 设置失败
			*err = TIMER_ERROR_INVALID_INPUT;
			HAL_EXIT_CRITICAL();
			DBG_TIMER(0x11);
			return NULL;			
		}

		/* or the delay time is not enough */
		if(ftime.s.hw - now.s.hw < TIMER_SET_TH)
		{//需要设置的时间的超频个数 超现有时间超频个数的的三个 以内  
			DBG_TIMER(0x12);
			func();
			*err = TIMER_SUCCESS;
			HAL_EXIT_CRITICAL();
			return NULL;
		}
	}

	tid = htimer_alloc();
	if (tid == 0)
	{//无多余计数队列
		*err = TIMER_ERROR_FULL;
		HAL_EXIT_CRITICAL();
		DBG_TIMER(0x13);		
		DBG_TRACE("fatal err, hal_stimer timer queue is empty, reset\r\n");
		delay_ms(1000);
		hal_board_reset();
		return NULL;
	}
	tid->ftime = ftime;
	tid->func = func;

	list_t* pos;
	list_sorted_add(&(tid->list),&queued_timer,htimer_t,list,htimer_compare,pos);

	if (list_get_head(&queued_timer) != &tid->list)
	{
		*err = TIMER_SUCCESS;
		HAL_EXIT_CRITICAL();
		DBG_TIMER(0x14);
		return tid;
	}

	if (ftime.s.sw != sw_time)
	{
		*err = TIMER_SUCCESS;
		HAL_EXIT_CRITICAL();
		DBG_TIMER(0x15);
		return tid;
	}
	HTIMER_SET_COMPREG(ftime.s.hw);

	htimer_state = TIMER_EXPECT_CMPINT;
	HAL_EXIT_CRITICAL();
	DBG_TIMER(0x16);
	return tid;
}

void htimer_controler_init(htimer_controler_t* htimer_controler)
{
	uint8_t i;

	/* init the two alarm lists */
	list_init(&free_timer);
	list_init(&queued_timer);
	
	/* link free alarms */
	for (i = 0; i < MAX_TIMERS; i++)
	{
		list_behind_put(&(htimer_arrary[i].list),&free_timer);
	}
	htimer_count = 0;
	htimer_state = TIMER_EXPECT_OFINT;
	sw_time = 0xFFAA;;
	
	htimer_controler->cancel = htimer_cancel;
	htimer_controler->now = htimer_now;
	htimer_controler->at = htimer_at;

	hal_htimer_start();
	return;
}

//硬计时清零 
void set_of_timer (void)
{
	HTIMER_SET_COMPREG(0x0);
	htimer_state = TIMER_EXPECT_OFINT;
}

void set_next_timer (bool_t cancel)
{
	htimer_t* ntimer;
	
	//获取一个队列
	ntimer = list_entry_get_head(&queued_timer,htimer_t,list);
	if (ntimer == NULL)
	{
		set_of_timer();
		DBG_TIMER(0x51);
		return;
	}
	
	if ((htimer_sw_t)(sw_time - ntimer->ftime.s.sw) > HTIMER_HALF_TURNOVER)
	{	//判断硬计时是否 超频 32.768M
		set_of_timer();
		DBG_TIMER(0x52);
	}
	else if (ntimer->ftime.s.sw == sw_time)
	{//记录时间 等于 计数时间，表示 定时计数器 没有工作
		htimer_hw_t now_hw = HTIMER_HWCOUNT();//获取此时的计数
		
		if((ntimer->ftime.s.hw < now_hw)||(ntimer->ftime.s.hw - now_hw < TIMER_SET_TH))
		{//表示 还在运行，或者在误差范围内
			DBG_TIMER(0x53);
			compare_interrupt_handler(PLAT_TRUE);
		}
		else
		{
			DBG_TIMER(0x54);
			HTIMER_SET_COMPREG(ntimer->ftime.s.hw);
			htimer_state = TIMER_EXPECT_CMPINT;
		}
	}
	else
	{
		DBG_TIMER(0x55);
		compare_interrupt_handler(PLAT_TRUE);
	}
	return;
}

void overflow_interrupt_handler (void)
{
	DBG_TIMER(0x40);
	sw_time++;  //计数加1
	set_next_timer(PLAT_FALSE);
}

void compare_interrupt_handler (bool_t internal)
{
	htimer_t* fire;
	htimer_hw_t now1, now2;
//	uint8_t i;

//	now1 = HTIMER_HWCOUNT() - TIMER_INT_DELAY;
	fire = list_entry_front_get(&queued_timer,htimer_t,list);
	now1 = fire->ftime.s.hw;
	
	if (internal == PLAT_FALSE)
	{//表示获取失败
		if (now1 != HTIMER_GET_COMPREG())
		{
			DBG_TIMER(0xA1);
//			i++;
		}
	}
	
	DBG_TIMER(0x30);
	if ((fire != NULL)&&(fire->func != NULL))
	{   //调用相关函数
		fire->func();
	}
	htimer_free(fire);

	now2 = HTIMER_HWCOUNT();
	
	if ((now2 < now1)&&(sw_time == fire->ftime.s.sw))
	{
		DBG_TIMER(0x31);
		overflow_interrupt_handler();
	}
	else
	{
		if((now2 + TIMER_SET_TH2 < now1)&&(sw_time == fire->ftime.s.sw))
		{
			dbgg2 = 0;
			while(HTIMER_HWCOUNT()!=0)
			{
				dbgg2++;
				if (dbgg2 > 100){
					dbgg2++;
				}
			}
			DBG_TIMER(0x32);
			overflow_interrupt_handler();			
		}
		else
		{
			DBG_TIMER(0x33);		
			set_next_timer(PLAT_FALSE);
		}
	}
	return;
}

void htimer_init( void )
{
	uint8_t i;

	/* init the two alarm lists */
	list_init(&free_timer);
	list_init(&queued_timer);
	
	/* link free alarms */
	for (i = 0; i < MAX_TIMERS; i++)
	{//计数成员 12 个
		list_behind_put(&(htimer_arrary[i].list),&free_timer);
	}
	
	htimer_count = 0;//已用成员个数
	htimer_state = TIMER_EXPECT_OFINT;
	sw_time = 0x0000;//软件计数
	
	//new added by fengli，超帧应用硬定时计数器中断配置 1tick 一中断
	Timer_A_registerInterrupt(TIMER_A0_BASE,
								TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,
								hal_timerA0_cr1_2_isr);
	
	hal_htimer_start();
	return;

}

void htimer_int_handler(void)
{
	if (htimer_state == TIMER_EXPECT_OFINT)
	{
		DBG_TIMER(0x21);
		overflow_interrupt_handler();//正常计时
	}
	else if (htimer_state == TIMER_EXPECT_CMPINT)
	{
		DBG_TIMER(0x22);
		compare_interrupt_handler(PLAT_FALSE);
	}
	else
	{
		DBG_TIMER(0xA1);
	}
}

/* The following code should be relocated */
void TA2_CCR_handler(void)
{	
	DBG_TIMER(0xBF);
}
/******************************************************/

static struct
{
	cb_func_t timer_isr;
	void *param;
}stimer_reg;

bool_t hal_stimer_isr_reg(cb_func_t cb_func, void *param)
{
	if (stimer_reg.timer_isr != NULL)
	{
		stimer_reg.timer_isr = cb_func;
		stimer_reg.param = param;
		return PLAT_TRUE;
	}	
	return PLAT_FALSE;
}

bool_t hal_stimer_isr_unreg(void)
{
	stimer_reg.timer_isr = NULL;
	stimer_reg.param = NULL;
	return PLAT_TRUE;
}


/*
 * isr for timer a0 ccr1-ccr2
 *
*/
static void hal_timerA0_cr1_2_isr(void)
{
	switch(((Timer_A_Type *)TIMER_A0_BASE)->IV)
	{
	case 2://正常计数
		htimer_int_handler();
		break;
	case 4:
		TA2_CCR_handler();
		break;			
	}
}
/*
 * isr for timer a0 ccr1-ccr4
 *
*/
