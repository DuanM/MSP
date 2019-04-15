 /**
 * provides an abstraction for uart.
 *
 * @file hal_uart.h
 * @author herry
 *
 * @addtogroup HAL_UART HAL system uart
 * @ingroup HAL
 * @{
 */

#ifndef __HAL_UART_H
#define __HAL_UART_H


#define UART_RECV_BUF_SIZE		    64
#define UART_RECV_BUF_OVF_SIZE		32
/**
 * 
 */
void hal_uart_init(uint8_t uart_id, uint32_t baud_rate);
void hal_uart_send_char(uint8_t uart_id, uint8_t data);
void hal_uart_send_string(uint8_t uart_id, uint8_t *string, uint32_t length);
void hal_uart_printf(uint8_t uart_id, uint8_t *string);
uint32_t hal_uart_read(uint8_t uart_id, void *buffer, uint32_t len);
uint8_t hal_uart_read_poll(uint8_t uart_id);
void hal_uart_rx_irq_enable(uint8_t uart_id, uint8_t fifo_thred, fpv_t func);
void hal_uart_rx_irq_disable(uint8_t uart_id);
void hal_uart_send_asic(uint8_t id, uint8_t value);
void hal_uart_rx_set_hiz_state(uint8_t uart_id);

#endif

