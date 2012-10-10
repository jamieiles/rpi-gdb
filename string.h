/*
 * Raspberry PI Remote Serial Protocol.
 * Copyright 2012 Jamie Iles, jamie@jamieiles.com.
 * Licensed under GPLv2.
 */
#ifndef __STRING_H__
#define __STRING_H__

#include "types.h"

size_t __weak strlen(const char *str);
int strncmp(const char *a, const char *b, size_t len);
bool is_hex(char v);
char *strchr(const char *str, int delim);
void __weak *memcpy(void *dst, const void *src, size_t len);

#endif /* __STRING_H__ */
