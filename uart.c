#include "io.h"
#include "uart.h"

#define UART0_BASE		(PERIPH_BASE + 0x201000)

#define UART_DR_REG_OFFS	0x00
#define		DR_OE_MASK	(1 << 11)
#define		DR_BE_MASK	(1 << 10)
#define		DR_PE_MASK	(1 << 9)
#define		DR_FE_MASK	(1 << 8)
#define		DR_DATA_MASK	(0xff)

#define UART_FR_REG_OFFS     	0x18
#define		FR_TXFE_MASK	(1 << 7)
#define		FR_RXFF_MASK	(1 << 6)
#define		FR_TXFF_MASK	(1 << 5)
#define		FR_RXFE_MASK	(1 << 4)
#define		FR_BUSY_MASK	(1 << 3)

#define UART_IBRD_REG_OFFS   	0x24
#define UART_FBRD_REG_OFFS   	0x28
#define UART_LCRH_REG_OFFS   	0x2c
#define		LCRH_SPS_MASK	(1 << 7)
#define		LCRH_WLEN8_MASK	(0x3 << 5)
#define		LCRH_WLEN7_MASK	(0x2 << 5)
#define		LCRH_WLEN6_MASK	(0x1 << 5)
#define		LCRH_WLEN5_MASK	(0x0 << 5)
#define		LCRH_FEN_MASK	(1 << 4)
#define		LCRH_STP2_MASK	(1 << 3)
#define		LCRH_EPS_MASK	(1 << 2)
#define		LCRH_PEN_MASK	(1 << 1)
#define		LCRH_BRK_MASK	(1 << 0)

#define UART_CR_REG_OFFS     	0x30
#define		CR_CTSEN_MASK	(1 << 15)
#define		CR_RTSEN_MASK	(1 << 14)
#define		CR_OUT2_MASK	(1 << 13)
#define		CR_OUT1_MASK	(1 << 12)
#define		CR_RTS_MASK	(1 << 11)
#define		CR_DTR_MASK	(1 << 10)
#define		CR_RXE_MASK	(1 << 9)
#define		CR_TXE_MASK	(1 << 8)
#define		CR_LBE_MASK	(1 << 7)
#define		CR_UART_EN_MASK	(1 << 0)

#define UART_IFLS_REG_OFFS   	0x34
#define		IFLS_RXIFSEL_SHIFT	3
#define		IFLS_RXIFSEL_MASK	(0x7 << IFLS_RXIFSEL_SHIFT)
#define		IFLS_TXIFSEL_SHIFT	0
#define		IFLS_TXIFSEL_MASK	(0x7 << IFLS_RXIFSEL_SHIFT)

#define UART_IMSC_REG_OFFS   	0x38
#define UART_RIS_REG_OFFS    	0x3c
#define UART_MIS_REG_OFFS    	0x40
#define UART_ICR_REG_OFFS    	0x44
#define		INT_OE		(1 << 10)
#define		INT_BE		(1 << 9)
#define		INT_PE		(1 << 8)
#define		INT_FE		(1 << 7)
#define		INT_RT		(1 << 6)
#define		INT_TX		(1 << 5)
#define		INT_RX		(1 << 4)
#define		INT_DSRM	(1 << 3)
#define		INT_DCDM	(1 << 2)
#define		INT_CTSM	(1 << 1)

void uart_putc(int c)
{
	while (readl(UART0_BASE + UART_FR_REG_OFFS) & FR_TXFF_MASK)
		continue;
	writel(UART0_BASE + UART_DR_REG_OFFS, c);

	if (c == '\n')
		uart_putc('\r');
}

int uart_getc(void)
{
        while (readl(UART0_BASE + UART_FR_REG_OFFS) & FR_RXFE_MASK)
		continue;
	return readl(UART0_BASE + UART_DR_REG_OFFS);
}

void uart_flush_rx(void)
{
	while (!(readl(UART0_BASE + UART_FR_REG_OFFS) & FR_RXFE_MASK))
		uart_getc();
}

void uart_disable(void)
{
	writel(UART0_BASE + UART_CR_REG_OFFS,0);
}

/* Enable for 115200,8n1.  Does anyone actually use anything else? */
void uart_enable(void)
{
	writel(UART0_BASE + UART_ICR_REG_OFFS, 0x7FF);
	writel(UART0_BASE + UART_IBRD_REG_OFFS, 1);
	writel(UART0_BASE + UART_FBRD_REG_OFFS, 40);
	writel(UART0_BASE + UART_LCRH_REG_OFFS,
	       LCRH_FEN_MASK | LCRH_WLEN8_MASK);
	writel(UART0_BASE + UART_CR_REG_OFFS,
	       CR_UART_EN_MASK | CR_TXE_MASK | CR_RXE_MASK);
}
