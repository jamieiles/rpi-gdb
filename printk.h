/*
 * Raspberry PI Remote Serial Protocol.
 * Copyright 2012 Jamie Iles, jamie@jamieiles.com.
 * Licensed under GPLv2.
 */
#ifndef __PRINTK_H__
#define __PRINTK_H__

void puts(const char *str);
void print_hex(unsigned long r);
void put_hex_byte(char *dst, char b);

#endif /* __PRINTK_H__ */
