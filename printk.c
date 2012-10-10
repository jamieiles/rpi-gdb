#include "printk.h"
#include "uart.h"

void puts(const char *str)
{
	const char *p = str;

	while (*p)
		uart_putc(*p++);
}

void put_hex_byte(char *dst, char b)
{
	char digits[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
        *dst++ = digits[(b & 0xf0) >> 4];
        *dst++ = digits[b & 0xf];
        *dst = '\0';
}

void print_hex(unsigned long r)
{
	char buf[9] = {};
	char digits[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	int i;

	for (i = 0; i < 8; i++) {
		unsigned long t = (r & (0xf << (i * 4))) >> (i * 4);
		buf[7 - i] = digits[t];
	}

	puts(buf);
}
