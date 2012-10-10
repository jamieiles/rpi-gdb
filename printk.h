#ifndef __PRINTK_H__
#define __PRINTK_H__

void puts(const char *str);
void print_hex(unsigned long r);
void put_hex_byte(char *dst, char b);

#endif /* __PRINTK_H__ */
