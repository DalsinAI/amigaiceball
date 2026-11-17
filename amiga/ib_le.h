/* Little-endian reads and writes for Iceball's file formats and network
 * packets (IceMap, PMF, WAV, TGA, the Lua pack calls), so the same bytes
 * come out on the Amiga's big-endian 68k as on a PC. Byte by byte, so they
 * also work on unaligned data.
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#ifndef IB_LE_H
#define IB_LE_H

#include <stdint.h>
#include <string.h>

static inline uint32_t ib_le16(const void *p)
{
	const uint8_t *b = (const uint8_t *)p;
	return (uint32_t)b[0] | ((uint32_t)b[1] << 8);
}

static inline int32_t ib_les16(const void *p)
{
	return (int32_t)(int16_t)ib_le16(p);
}

static inline uint32_t ib_le32(const void *p)
{
	const uint8_t *b = (const uint8_t *)p;
	return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

/* n (1 to 4) bytes, least significant first: a TGA pixel or palette entry */
static inline uint32_t ib_len(const void *p, int n)
{
	const uint8_t *b = (const uint8_t *)p;
	uint32_t v = 0;
	int i;
	for (i = n - 1; i >= 0; i--)
		v = (v << 8) | b[i];
	return v;
}

static inline uint32_t ib_be32(const void *p)
{
	const uint8_t *b = (const uint8_t *)p;
	return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | (uint32_t)b[3];
}

static inline uint64_t ib_le64(const void *p)
{
	return (uint64_t)ib_le32(p) | ((uint64_t)ib_le32((const uint8_t *)p + 4) << 32);
}

static inline float ib_lef(const void *p)
{
	uint32_t v = ib_le32(p);
	float f;
	memcpy(&f, &v, 4);
	return f;
}

static inline double ib_led(const void *p)
{
	uint64_t v = ib_le64(p);
	double d;
	memcpy(&d, &v, 8);
	return d;
}

static inline void ib_put_le16(void *p, uint32_t v)
{
	uint8_t *b = (uint8_t *)p;
	b[0] = (uint8_t)v;
	b[1] = (uint8_t)(v >> 8);
}

static inline void ib_put_le32(void *p, uint32_t v)
{
	uint8_t *b = (uint8_t *)p;
	b[0] = (uint8_t)v;
	b[1] = (uint8_t)(v >> 8);
	b[2] = (uint8_t)(v >> 16);
	b[3] = (uint8_t)(v >> 24);
}

static inline void ib_put_led(void *p, double d)
{
	uint64_t v;
	memcpy(&v, &d, 8);
	ib_put_le32(p, (uint32_t)v);
	ib_put_le32((uint8_t *)p + 4, (uint32_t)(v >> 32));
}

/* 16-bit samples from little-endian bytes into the host's order, in place */
static inline void ib_le16_samples(int16_t *s, uint32_t n)
{
	uint32_t i;
	for (i = 0; i < n; i++)
		s[i] = (int16_t)ib_le16(&s[i]);
}

/* A pixel as Iceball keeps it, 0xAARRGGBB, into 4 bytes B, G, R, A and back:
 * the PNG code works on the bytes. */
static inline void ib_put_bgra(uint8_t *p, uint32_t c)
{
	ib_put_le32(p, c);
}

static inline uint32_t ib_get_bgra(const uint8_t *p)
{
	return ib_le32(p);
}

#endif
