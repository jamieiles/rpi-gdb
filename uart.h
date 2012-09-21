#ifndef __UART_H__
#define __UART_H__

void uart_putc(int c);
int uart_getc(void);
void uart_flush_rx(void);
void uart_disable(void);

#define UART_INT_ENABLE	(1 << 0)
void uart_enable(unsigned long flags);

#endif /* __UART_H__ */
