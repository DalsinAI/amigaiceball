/* Iceball on GFOS: the four zlib calls Iceball makes (file transfer to
 * clients, PNG pictures, their checksums) done by z.library (GFOpenZlib)
 * instead of a compiled-in zlib. ib_zlib.c opens the library when first
 * needed. GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#ifndef IB_AMIGA_ZLIB_H
#define IB_AMIGA_ZLIB_H

#include <libraries/z.h>       /* Z_OK, Z_BUF_ERROR, ... */
#ifndef Z_NULL
#define Z_NULL 0
#endif

typedef unsigned char Bytef;
typedef unsigned long uLong;
typedef unsigned long uLongf;

int ib_compress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
int ib_uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
uLong ib_compressBound(uLong sourceLen);
uLong ib_crc32(uLong crc, const Bytef *buf, unsigned len);

#define compress ib_compress
#define uncompress ib_uncompress
#define compressBound ib_compressBound
#define crc32 ib_crc32

#endif
