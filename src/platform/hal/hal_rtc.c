#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\rtc_c.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>
#include <ti\devices\msp432p4xx\driverlib\cs.h>

void hal_rtc_init(void)
{
	RTC_C_Calendar calendarTime;
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,GPIO_PIN0 | GPIO_PIN1,GPIO_PRIMARY_MODULE_FUNCTION);
    CS_setExternalClockSourceFrequency(32768,48000000);
    
	CS_startLFXT(false);
    calendarTime.year = RTC_C_convertBinaryToBCD(2019);
	calendarTime.month = 1;
	calendarTime.dayOfmonth = 1;
	calendarTime.dayOfWeek = rtc_day_of_week(2019,1,1);
	calendarTime.hours = 0;
	calendarTime.minutes = 0;
	calendarTime.seconds = 0;
    
	RTC_C_initCalendar(&calendarTime, RTC_C_FORMAT_BCD);
    RTC_C_startClock();
}

void hal_rtc_get(hal_rtc_block_t *rtc_block)
{
	RTC_C_Calendar calendarTime;
    mem_clr(&calendarTime,sizeof(RTC_C_Calendar));
    
	calendarTime = RTC_C_getCalendarTime(); 
	rtc_block->year = RTC_C_convertBCDToBinary(calendarTime.year);//calendarTime.year;
	rtc_block->month = RTC_C_convertBCDToBinary(calendarTime.month);//calendarTime.month;
	rtc_block->day = RTC_C_convertBCDToBinary(calendarTime.dayOfmonth);//calendarTime.dayOfmonth;
	rtc_block->weekday = RTC_C_convertBCDToBinary(calendarTime.dayOfWeek);//calendarTime.dayOfWeek;
	rtc_block->hour = RTC_C_convertBCDToBinary(calendarTime.hours);//calendarTime.hours;
	rtc_block->minute = RTC_C_convertBCDToBinary(calendarTime.minutes);//calendarTime.minutes;
	rtc_block->second = RTC_C_convertBCDToBinary(calendarTime.seconds);//calendarTime.seconds;
}

void hal_rtc_set(hal_rtc_block_t *rtc_block)
{
    OSEL_DECL_CRITICAL();
	RTC_C_Calendar calendarTime;
    mem_clr(&calendarTime,sizeof(RTC_C_Calendar));
   
	calendarTime.year = RTC_C_convertBinaryToBCD(rtc_block->year);//(yearH<<8 | yearL);//RTC_C_convertBinaryToBCD(rtc_block->year);
	calendarTime.month =  RTC_C_convertBinaryToBCD(rtc_block->month);
	calendarTime.dayOfmonth = RTC_C_convertBinaryToBCD(rtc_block->day);
	calendarTime.dayOfWeek = RTC_C_convertBinaryToBCD(rtc_block->weekday);
	calendarTime.hours =  RTC_C_convertBinaryToBCD(rtc_block->hour);
	calendarTime.minutes = RTC_C_convertBinaryToBCD(rtc_block->minute);
	calendarTime.seconds = RTC_C_convertBinaryToBCD(rtc_block->second);
    
    CS_startLFXT(false);
    CS_setExternalClockSourceFrequency(32768,48000000); 
    OSEL_ENTER_CRITICAL();	
	RTC_C_initCalendar(&calendarTime, RTC_C_FORMAT_BCD);
    OSEL_EXIT_CRITICAL();
    RTC_C_startClock();
}

//算法通过年月日，得知今天为周几
uint8_t rtc_day_of_week(uint16_t y,uint16_t m,uint16_t d)
{
    int c = 0,w = 0,y0=0,m0=0,d0=0;
    y0 = y;
    m0 = m;
    d0 = d;
	if(m0 == 1 || m0 == 2)
	{ 
          y0--;
          m0 += 12;
	}
	c = y0/100;
	y0 = y0-c*100;
	w = (c/4)-2*c+(y0+y0/4)+(13*(m0+1)/5)+d0-1;
	while(w<0)
		w += 7;
	w%=7;
	return w;
}















