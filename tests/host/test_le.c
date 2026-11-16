/* ib_le.h: little-endian reads and writes give the same numbers on the PC
 * and on the 68k. GPL-3.0-or-later, like Iceball.
 * Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "ib_le.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

int main(void)
{
	const uint8_t b[] = { 0x34, 0x12, 0x78, 0x56, 0xFF, 0xFE, 0x00, 0x80, 0x01 };
	uint8_t o[8];
	int16_t s[2];
	float f = 1.5f;
	double d = -2.25;

	CHECK(ib_le16(b) == 0x1234);
	CHECK(ib_le32(b) == 0x56781234u);
	CHECK(ib_le32(b + 1) == 0xFF567812u);         /* unaligned */
	CHECK(ib_les16(b + 4) == -257);
	CHECK(ib_le16(b + 6) == 0x8000);
	CHECK(ib_len(b, 1) == 0x34 && ib_len(b, 2) == 0x1234 && ib_len(b, 3) == 0x781234);
	CHECK(ib_be32(b) == 0x34127856u);
	CHECK(ib_le64(b) == 0x8000FEFF56781234ull);

	ib_put_le16(o, 0xBEEF);
	CHECK(o[0] == 0xEF && o[1] == 0xBE);
	ib_put_le32(o, 0x01020304);
	CHECK(o[0] == 4 && o[1] == 3 && o[2] == 2 && o[3] == 1);

	/* 1.5f is 0x3FC00000, -2.25 is 0xC002000000000000 */
	{ uint8_t fb[4] = { 0, 0, 0xC0, 0x3F }; CHECK(ib_lef(fb) == f); }
	ib_put_led(o, d);
	CHECK(o[7] == 0xC0 && o[6] == 0x02 && o[0] == 0);
	CHECK(ib_led(o) == d);

	memcpy(s, "\x01\x02\xFF\x7F", 4);
	ib_le16_samples(s, 2);
	CHECK(s[0] == 0x0201 && s[1] == 0x7FFF);

	CHECK(ib_get_bgra((const uint8_t *)"\x11\x22\x33\x44") == 0x44332211u);
	ib_put_bgra(o, 0xFF336699u);
	CHECK(o[0] == 0x99 && o[1] == 0x66 && o[2] == 0x33 && o[3] == 0xFF);

	if (fails)
		return 1;
	puts("le: 16 checks");
	return 0;
}
