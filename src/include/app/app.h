#ifndef __APP_H
#define __APP_H
#include <control_io.h>

#define APP_EVENT_DBG_RX		(1u<<1)
#define APP_EVENT_DBG_TX		(1u<<2)
#define APP_EVENT_GPS			(1u<<3)
#define APP_EVENT_LORA_RX		(1u<<4)
#define APP_EVENT_CFG			(1u<<5)
#define APP_EVENT_INDICATE		(1u<<6)
#define APP_EVENT_KEY			(1u<<7)


#define APP_TIMEOUT  500000
#define APP_EVENT_TIMEOUT  2000000

extern osel_task_t *app_task_h;
extern osel_event_t *app_event_h;
extern osel_task_t *init_task_h;

extern list_t app_data_tx_q;

#define LITTLE2BIG_ENDIAN_16B(a) ((a>>8)&0xFF)|((a&0xFF)<<8);
#define BIG2LITTLE_ENDIAN_16B(b) ((b>>8)&0xFF)|((b&0xFF)<<8);


///////////////////////////////////////////////////////////


void app_init(void);

void app_lora_recv_callback(void);

void app_indicate_event(void);

void app_handler(uint16_t event_type);

void app_gps_poll_callback(void);
void app_gps_poll_handler(void);
void app_gps_proc(char_t *p_data, uint16_t size);

#endif



