#ifndef __STACK_H
#define __STACK_H

#define STACK_FRM_MAX_LEN		125

typedef void (*fppcv_t)(uint8_t *,device_info_t *device_info);      /* function pointer void */      


extern osel_task_t *stack_task_h;
extern osel_event_t *stack_event_h;

extern list_t stack_data0_tx_q;
extern list_t stack_data1_tx_q;
extern list_t stack_data_rx_q;

void stack_init(void);
void stack_uart_lora_irq_enable_callback(void);
void stack_indicate(fpv_t func);
void stack_indicate_handle(void);

void stack_config(fppcv_t func);
void stack_config_handle(uint8_t *cfg_buf,device_info_t *device_info);

void stack_tx_event(void);
void stack_priv_list_cfg(list_t *list);
list_t * stack_priv_list_get_handle(void);
void stack_priv_list_send_cfg(fpv_t func);
void stack_priv_list_send_handle(void);



#endif



