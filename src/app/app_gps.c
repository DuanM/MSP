#include <platform.h>
#include <device.h>
#include <app.h>

#define NMEA_TIME_SIZE				16
#define NMEA_FLAG_SIZE				2
#define NMEA_LAT1_SIZE				12
#define NMEA_LAT2_SIZE				2
#define NMEA_LON1_SIZE				14
#define NMEA_LON2_SIZE				2
#define NMEA_DATE_SIZE				10

#define MAX_GPS_DATA				16

#define TIME_CHINA_ZONE				8

typedef struct
{	
    char_t time[NMEA_TIME_SIZE];
	char_t flag[NMEA_FLAG_SIZE];
	char_t latitude[NMEA_LAT1_SIZE];
	char_t latitude2[NMEA_LAT2_SIZE];
	char_t longitude[NMEA_LON1_SIZE];
	char_t longitude2[NMEA_LON2_SIZE];	
	char_t date[NMEA_DATE_SIZE];
}NMEA_RMC_GGA_MSG;

#define APP_GPS_BUF_SIZE 256
static uint16_t actual_size = 0;
static NMEA_RMC_GGA_MSG p_nmea_gps_msg;
uint8_t app_gps_buf[APP_GPS_BUF_SIZE]={0};

void app_gps_poll_callback(void)
{
	uint8_t value = 0;
	value = hal_uart_read_poll(UART_GPS);
    
	if(value && actual_size<APP_GPS_BUF_SIZE)
	{
		app_gps_buf[actual_size] = value;
		actual_size++;
        
		if (actual_size >= APP_GPS_BUF_SIZE)
		{
			uint16_t object	= APP_EVENT_GPS;
			osel_event_set(app_event_h, &object);
		}
	}
}

void app_gps_proc(char_t *p_data, uint16_t size)
{
	static uint8_t update_rtc_flag = 0;
	dev_time_t dev_time;
	bool_t flag_day, flag_year;
	hal_rtc_block_t rtc_block;
	uint16_t year;
	uint8_t counter = 0;
	uint8_t off_set = 0;	
	char_t *p_start = PLAT_NULL;
	char_t *p_stop = PLAT_NULL;
	device_info_t *p_device_info = device_info_get();
	p_start = p_data;
	counter = 0;
	p_start = strstr(p_data, ",");

	do
	{
		if (p_start) 
		{
			counter++;
			p_stop = strstr(p_start+1, ",");
			
			if (p_stop)
			{
				off_set = (uint32_t)p_stop - (uint32_t)p_start - 1;				
				switch(counter)
	            {
	                case 1://time
						if (off_set<NMEA_TIME_SIZE)mem_cpy(p_nmea_gps_msg.time, p_start+1, off_set);							
						break;
	                case 2://flag
						if (off_set<NMEA_FLAG_SIZE)mem_cpy(p_nmea_gps_msg.flag, p_start+1,off_set);							
						break;
	                case 3:	//lat						
						if (off_set<NMEA_LAT1_SIZE) mem_cpy(p_nmea_gps_msg.latitude, p_start+1,off_set);
						break;
	                case 4:
						if (off_set<NMEA_LAT2_SIZE)mem_cpy(p_nmea_gps_msg.latitude2, p_start+1,off_set);
						break;
	                case 5://long
						if (off_set<NMEA_LON1_SIZE)mem_cpy(p_nmea_gps_msg.longitude, p_start+1,off_set);
						break;
	                case 6:
						if (off_set<NMEA_LON2_SIZE)mem_cpy(p_nmea_gps_msg.longitude2, p_start+1,off_set);
						break;	                        
	                case 9://date
						if (off_set<NMEA_DATE_SIZE)mem_cpy(p_nmea_gps_msg.date, p_start+1,off_set);
						break;
		            default:break;
	            }
				p_start = p_stop;
			}				
		}			
	}while(p_start && p_stop);
    
	if (p_nmea_gps_msg.flag[0] == 'A' && update_rtc_flag == 0)
	{
		//DBG_TRACE("Gps Valid\r\n");
		update_rtc_flag++;
		mem_set(&dev_time, 0, sizeof(dev_time_t));
		mem_set(&rtc_block, 0, sizeof(hal_rtc_block_t));
		/*时间判断*/
		dev_time.hour = (p_nmea_gps_msg.time[0]-'0')*10 + p_nmea_gps_msg.time[1]-'0' + TIME_CHINA_ZONE;
		if (dev_time.hour >= 24)
		{
			dev_time.hour -= 24;
			flag_day = 1;
		}
		else
		{
			flag_day = 0;
		}
		dev_time.minute = (p_nmea_gps_msg.time[2]-'0')*10 + p_nmea_gps_msg.time[3]-'0';
		dev_time.second = (p_nmea_gps_msg.time[4]-'0')*10 + p_nmea_gps_msg.time[5]-'0';		
		//////////////////////////////////////////////////////////////////////
		dev_time.day = (p_nmea_gps_msg.date[0]-'0')*10 + p_nmea_gps_msg.date[1]-'0';
		dev_time.month = (p_nmea_gps_msg.date[2]-'0')*10 + p_nmea_gps_msg.date[3]-'0';
		dev_time.year = (p_nmea_gps_msg.date[4]-'0')*10 + p_nmea_gps_msg.date[5]-'0';
		year = 2000 + dev_time.year;		
	    //判断润年
		if (year % 4 == 0 || year % 400 == 0)
		{
			if (year % 100 != 0)
			{
				flag_year = 1;
			}
			else
			{
				flag_year = 0;
			}
		}
		else
		{
			flag_year = 0;
		}
		//递进判断
		if (flag_day)
		{
			dev_time.day += 1;
			flag_day = 0;
			//判断1 3 5 7 8 10 12月份31??
			if(dev_time.month == 1 ||
				dev_time.month == 3 ||
				dev_time.month == 5 ||
				dev_time.month == 7 ||
				dev_time.month == 8 ||
				dev_time.month == 10 ||
				dev_time.month == 12)
			{
				if (dev_time.day > 31)
				{
					dev_time.day -= 31; 
					dev_time.month++;
					if (dev_time.month > 12)
					{
						dev_time.month = 1;
						dev_time.year++;
					}
				}
			}
			else if (dev_time.month == 2)
			{
				/*闰年*/
				if (flag_year)
				{
					if (dev_time.day > 29)
					{
						dev_time.day -= 29; 
						dev_time.month++;
					}
				}
				else
				{
					if (dev_time.day > 28)
					{
						dev_time.day -= 28;
						dev_time.month++;
					}
				}
			}
			else if (dev_time.month == 4 ||
						dev_time.month == 6 ||
						dev_time.month == 9 ||
						dev_time.month == 11)
			{
				if (dev_time.day > 30)
				{
					dev_time.day -= 30; 
					dev_time.month++;					
				}
			}
		}
	    
	    if (dev_time.year == 0 
	        || dev_time.month == 0
	        || dev_time.day == 0 
	        || dev_time.hour == 0 
	        || dev_time.minute == 0 
	        || dev_time.second == 0     
	        || dev_time.month>12            
	        || dev_time.day>31
	        || dev_time.hour>24
	        || dev_time.minute>59
	        || dev_time.second>59)
	    {
	        DBG_TRACE("Gps time invalid\r\n");
	        return;
	    }
		
		rtc_block.year = 2000+dev_time.year;
		rtc_block.day = dev_time.day;
		rtc_block.month = dev_time.month;
		rtc_block.hour = dev_time.hour;
		rtc_block.minute = dev_time.minute;
		rtc_block.second = dev_time.second;
		rtc_block.week = 1;
		rtc_block.weekday = rtc_day_of_week(rtc_block.year,dev_time.month,dev_time.day);
		hal_rtc_set(&rtc_block);
        
        DBG_GPS_PRINTF("Y:%d-M:%d-D:%d h:%d-m:%d-s:%d\r\n",
                   rtc_block.year,rtc_block.month, rtc_block.day,
                   rtc_block.hour,rtc_block.minute, rtc_block.second);
	}
	
    if ((p_nmea_gps_msg.flag[0] == 'A') && (update_rtc_flag != 0))
	{
		//DBG_TRACE("Gps Valid\r\n");
		update_rtc_flag++;
		//add posistion to device info
		uint32_t tmp_value,gps_value;
		uint32_t int_value, dec_value;

		//latitude
		p_start = strstr(p_nmea_gps_msg.latitude, ".");
		{
			*p_start = '\0';
			
			sscanf(p_nmea_gps_msg.latitude, "%d", &tmp_value);
			int_value = tmp_value/100;
			//DBG_PRINTF("g1: %d\r\n",int_value);
			dec_value = tmp_value%100;
			//DBG_PRINTF("g2: %d\r\n",dec_value);
			
			p_start++;
			sscanf(p_start, "%d", &tmp_value);
			dec_value = dec_value*1000000+tmp_value;
			//DBG_PRINTF("g3: %d\r\n",dec_value);
			dec_value = dec_value/(60.0);
			//DBG_PRINTF("g4: %d\r\n",dec_value);
			
			gps_value = int_value*1000000 + dec_value; 
			//DBG_PRINTF("g5: %d\r\n",gps_value);
			
			p_device_info->pos.latitude = gps_value;
			p_device_info->pos.sn = (bool_t)p_nmea_gps_msg.latitude2[0];

			//纬度
			DBG_GPS_PRINTF("latitude:%d\r\nsn:%c\r\n",p_device_info->pos.latitude,p_device_info->pos.sn);
		}

		//longitude 
        p_start = strstr(p_nmea_gps_msg.longitude, ".");
		{
			*p_start = '\0';
			//整数部分
			sscanf(p_nmea_gps_msg.longitude, "%d", &tmp_value);
			int_value = tmp_value/100;
			dec_value = tmp_value%100;
			
			p_start++;
			sscanf(p_start, "%d", &tmp_value);
			dec_value = dec_value*1000000+tmp_value;
            
			dec_value = dec_value/(60.0);

			gps_value = int_value*1000000 + dec_value; 	

			p_device_info->pos.longitude = gps_value;
			p_device_info->pos.we = (bool_t)p_nmea_gps_msg.longitude2[0];

			//经度
			DBG_GPS_PRINTF("longitude:%d\r\nwe:%c\r\n",p_device_info->pos.longitude,p_device_info->pos.we);
		}
		ControlIO_StatusLight(GREEN);
	}
 	else 
	{//无效
		p_device_info->pos.we = 'V';
		p_device_info->pos.sn = 'V';
        
		ControlIO_StatusLight(RED);
		DBG_GPS_PRINTF("sn:%c\r\nwe:%c\r\n",p_device_info->pos.sn,p_device_info->pos.we);
	}
}

void app_gps_poll_handler(void)
{
	char_t *p_start = NULL;
	uint8_t offset = 0;
	uint8_t loop;
    
	p_start =  strstr((char_t*)app_gps_buf, "RMC");
    
	if (p_start != NULL)
	{	
		offset = actual_size - ((uint32_t)p_start - (uint32_t)app_gps_buf);			

		if (offset>10)
		{				
			for (loop=0; loop<offset; loop++)
			{				
				if (p_start[loop] == '\r' && p_start[loop+1] == '\n')
				{
					break;
				}
			}
			
			if (loop != offset)
			{                
                app_gps_proc(p_start+strlen("RMC"), loop-strlen("RMC"));
			}
		}				
	}
	actual_size = 0;
	mem_set(app_gps_buf,0, APP_GPS_BUF_SIZE);
}



