/* -*- linux-c -*- ------------------------------------------------------- *
 *
 *   Copyright (C) 1991, 1992 Linus Torvalds
 *   Copyright 2007 rPath, Inc. - All Rights Reserved
 *
 *   This file is part of the Linux kernel, and is made available under
 *   the terms of the GNU General Public License version 2.
 *
 * ----------------------------------------------------------------------- */

/*
 * arch/i386/boot/string.c
 *
 * Very basic string functions
 */
#include "code16gcc.h"
#include "boot.h"
#include <inc/types.h>
#include <stddef.h>
#include <stdarg.h>

int strcmp(const char *str1, const char *str2)
{
	const unsigned char *s1 = (const unsigned char *)str1;
	const unsigned char *s2 = (const unsigned char *)str2;
	int delta = 0;

	while (*s1 || *s2) {
		delta = *s2 - *s1;
		if (delta)
			return delta;
		s1++;
		s2++;
	}
	return 0;
}

size_t strnlen(const char *s, size_t maxlen)
{
	const char *es = s;
	while (*es && maxlen) {
		es++;
		maxlen--;
	}

	return (es - s);
}

unsigned int atou(const char *s)
{
	unsigned int i = 0;
	while (isdigit(*s))
		i = i * 10 + (*s++ - '0');
	return i;
}


void * __attribute__((regparm(3))) memcpy(void *dst, void *src, size_t n)
{
	const char *s;
	char *d;
	
	s = src;
	d = dst;
	if (s < d && s + n > d) {
		s += n;
		d += n;
		while (n-- > 0)
			*--d = *--s;
	} else
		while (n-- > 0)
			*d++ = *s++;

	return dst;
}

void * __attribute__((regparm(3))) memset(void *dst, int c, size_t len)
{
	char *p;
	p = (char *)dst;
	for(u16 i = 0; i < len; i ++)
		p[i] = (char)c;

	return (dst);
}

