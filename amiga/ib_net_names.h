/* Dotted IPv4 addresses to and from the 32-bit number in network order
 * (big-endian on the 68k), with no TCP/IP stack needed. Kept apart from
 * ib_amiga.c for the host tests.
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#ifndef IB_NET_NAMES_H
#define IB_NET_NAMES_H

#include <stdio.h>
#include <string.h>

/* "a.b.c.d", each 0 to 255: *out in network byte order */
static inline int ib_parse_ipv4(const char *s, unsigned long *out)
{
	unsigned char b[4];
	int i;
	if (s == NULL)
		return 0;
	for (i = 0; i < 4; i++) {
		unsigned v = 0;
		int digits = 0;
		while (*s >= '0' && *s <= '9') {
			v = v * 10 + (unsigned)(*s++ - '0');
			if (++digits > 3 || v > 255)
				return 0;
		}
		if (digits == 0)
			return 0;
		b[i] = (unsigned char)v;
		if (i < 3 && *s++ != '.')
			return 0;
	}
	if (*s != '\0')
		return 0;
	/* network order is the bytes as written, whatever the host's order */
	memcpy(out, b, 4);
	if (sizeof(*out) > 4)
		memset((unsigned char *)out + 4, 0, sizeof(*out) - 4);
	return 1;
}

static inline void ib_format_ipv4(unsigned long v, char *buf)
{
	unsigned char b[4];
	memcpy(b, &v, 4);
	sprintf(buf, "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
}

#endif
