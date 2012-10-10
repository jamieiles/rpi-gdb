/*
 * Raspberry PI Remote Serial Protocol.
 * Copyright 2012 Jamie Iles, jamie@jamieiles.com.
 * Licensed under GPLv2.
 */
#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned long size_t;
typedef long ssize_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

typedef char bool;
#define true 1
#define false 0

#define NULL	((void *)0)

struct iovec {
	const char *iov_base;
	size_t iov_len;
};

#endif /* __TYPES_H__ */
