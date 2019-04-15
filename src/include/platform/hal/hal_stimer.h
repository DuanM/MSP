
/**
 * Hybrid timer module which combines hw timer and sw timer.
 *
 * @file
 * @author daoyuan (2009-03-03)
 *
 * @addtogroup LIB_LINKLIST Link list operations
 * @ingroup LIB
 * @{
 */

#ifndef __HAL_STIMER_H
#define __HAL_STIMER_H

typedef void (*cb_func_t)(void *);

#define HT_S (uint32_t)0x8003
#define HT_MS (uint32_t)0x33

//#define DBG_TIMER(X) hal_uart_send_char(UART_PC,X)
#define DBG_TIMER(X)
#define DBG_TIMER_INIT()
#define DBG_BLUE_TOGGLE()
#define DBG_RED_TOGGLE()
//#define DBG_TIMER_INIT() do {P6DIR |= 0x1; P8DIR |= 0x4;} while(0)
//#define DBG_BLUE_TOGGLE() do {P6OUT ^= 0x1;} while(0)
//#define DBG_RED_TOGGLE() do {P8OUT ^= 0x4;} while(0)

#define MAX_TIMERS 12

#define TIMER_START() \
do\
{\
    TA0CTL = TASSEL_1 + MC_2 + TACLR; \
    TA0CCTL0 = CM_1 + SCS + CAP + CCIE;\
}while(0)

#define TIMER_STOP() \
do\
{\
    TA0CTL = 0x0000; \
    TA0CCTL0 = 0x0000;\
}while(0)


#define HTIMER_HWCOUNT() TA0R
#define HTIMER_SET_COMPREG(x) TA0CCR1 = x
#define HTIMER_GET_COMPREG() TA0CCR1

#define HTIMER_COMPINT_ENABLE() TA0CCTL1 |= CCIE
#define HTIMER_COMPINT_DISABLE() TA0CCTL1 &= ~CCIE

#define HTIMER_FLG() (TA0CCTL1 & CCIFG)
#define HTIMER_CLEAR_FLG() TA0CCTL1 &= ~CCIFG

#define HTIMER_HALF_TURNOVER 0x8000
#define CMPLAG
#define TIMER_SET_TH 0x03
#define TIMER_SET_TH2 0x02


typedef uint16_t htimer_sw_t;
typedef uint16_t htimer_hw_t;

typedef struct spliced_time
{
	htimer_hw_t hw;
 	htimer_sw_t sw;
} spliced_time_t;

typedef union time_union
{
	spliced_time_t s;
	uint32_t w;
} time_t;

/*  timer error codes */
typedef enum
{
	TIMER_SUCCESS,
	TIMER_ERROR_ENTRY_EXIST,
	TIMER_ERROR_ENTRY_NOT_EXIST,
	TIMER_ERROR_INVALID_INPUT,
	TIMER_ERROR_FULL,
} htimer_err_t;

typedef	void (*tcbf_t)(void);

typedef struct htimer_callback
{
	tcbf_t func;
	void* parm;
} htimer_callback_t;

typedef struct htimer
{
	list_t list;
	time_t ftime;			
	tcbf_t func;
} htimer_t;

typedef struct htimer_controler
{
	htimer_err_t (*now) (time_t*);
	htimer_t* (*at) (time_t, tcbf_t, htimer_err_t*);
	htimer_err_t (*cancel) (htimer_t**);
} htimer_controler_t;

extern htimer_controler_t htimer;
void htimer_controler_init(htimer_controler_t* htimer_controler);
//void htimer_compare_ISR(void);

void htimer_init( void );
void htimer_deinit(void);
htimer_t* htimer_at (time_t ftime, tcbf_t func, htimer_err_t* err);
htimer_err_t htimer_now (time_t* now);
htimer_err_t htimer_cancel (htimer_t** tidd);

#define HAL_DECL_CRITICAL() OSEL_DECL_CRITICAL()
#define HAL_ENTER_CRITICAL() OSEL_ENTER_CRITICAL()
#define HAL_EXIT_CRITICAL() OSEL_EXIT_CRITICAL()

#endif