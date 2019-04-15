#ifndef __HAL_RTC_H
#define __HAL_RTC_H

#pragma pack(1)

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t week;
    uint8_t weekday;
	uint8_t day;
    uint8_t month;
	uint16_t year;
} hal_rtc_block_t;

#pragma pack()

void hal_rtc_init(void);
void hal_rtc_set(hal_rtc_block_t *rtc_block);
void hal_rtc_get(hal_rtc_block_t *rtc_block);
void hal_rtc_set(hal_rtc_block_t *rtc_block);
uint8_t rtc_day_of_week(uint16_t y,uint16_t m,uint16_t d);

#endif /*RTC_H_*/









