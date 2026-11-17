/* ib_net_names.h: dotted IPv4 to network order and back, on PC and 68k.
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "ib_net_names.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

int main(void)
{
	unsigned long v;
	unsigned char b[4];
	char buf[16];

	CHECK(ib_parse_ipv4("192.168.1.20", &v));
	memcpy(b, &v, 4);
	CHECK(b[0] == 192 && b[1] == 168 && b[2] == 1 && b[3] == 20);
	ib_format_ipv4(v, buf);
	CHECK(!strcmp(buf, "192.168.1.20"));

	CHECK(ib_parse_ipv4("0.0.0.0", &v));
	CHECK(ib_parse_ipv4("255.255.255.255", &v));
	ib_format_ipv4(v, buf);
	CHECK(!strcmp(buf, "255.255.255.255"));

	CHECK(!ib_parse_ipv4("256.1.1.1", &v));
	CHECK(!ib_parse_ipv4("1.2.3", &v));
	CHECK(!ib_parse_ipv4("1.2.3.4.5", &v));
	CHECK(!ib_parse_ipv4("1.2.3.", &v));
	CHECK(!ib_parse_ipv4("magicannon.com", &v));
	CHECK(!ib_parse_ipv4("1..2.3", &v));
	CHECK(!ib_parse_ipv4("0001.2.3.4", &v));
	CHECK(!ib_parse_ipv4("", &v));
	CHECK(!ib_parse_ipv4(NULL, &v));

	if (fails)
		return 1;
	puts("net names: 16 checks");
	return 0;
}
