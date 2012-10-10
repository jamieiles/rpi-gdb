/*
 * Raspberry PI Remote Serial Protocol.
 * Copyright 2012 Jamie Iles, jamie@jamieiles.com.
 * Licensed under GPLv2.
 */
#include "kernel.h"
#include "string.h"

size_t strlen(const char *str)
{
	size_t len = 0;

	while (*str++)
		++len;

	return len;
}

int strncmp(const char *a, const char *b, size_t len)
{
	int ac, bc;

	if (len == 0)
		return 0;

	while (len > 0) {
		ac = *a++;
		bc = *b++;

		if (ac == '\0' || ac != bc)
			return ac - bc;
		--len;
	}

	return ac - bc;
}

bool is_hex(char v)
{
	if (v >= 'A' && v <= 'F')
		v = (v - 'A') + 'a';
	return (v >= '0' && v <= '9') || (v >= 'a' && v <= 'f');
}

char *strchr(const char *str, int delim)
{
	while (*str) {
		if (*str == delim)
			return (char *)str;
		++str;
	}

	return NULL;
}

void *memcpy(void *dst, const void *src, size_t len)
{
	char *dst8 = dst;
	const char *src8 = src;

	while (len--)
		*dst8++ = *src8++;

	return dst8;
}
