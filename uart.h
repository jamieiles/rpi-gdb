#ifndef __UART_H__
#define __UART_H__

void uart_putc(int c);
int uart_getc(void);
void uart_flush_rx(void);
void uart_disable(void);
void uart_enable(void);

#endif /* __UART_H__ */
