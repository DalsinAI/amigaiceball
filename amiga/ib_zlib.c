/* Iceball on GFOS: zlib's calls through z.library (amiga/zlib.h). The
 * library is opened on first use and closed at exit; without it compress
 * and uncompress fail (Z_VERSION_ERROR) and crc32 gives 0, so a PNG reads as
 * damaged and a file transfer as failed, instead of the program stopping.
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#include <stdlib.h>
#include <exec/types.h>
#include <proto/exec.h>
#include "zlib.h"
#undef compress
#undef uncompress
#undef compressBound
#undef crc32
#include <proto/z.h>

struct Library *ZBase;

static void close_z(void)
{
	if (ZBase) {
		CloseLibrary(ZBase);
		ZBase = NULL;
	}
}

static int open_z(void)
{
	if (!ZBase) {
		ZBase = OpenLibrary((CONST_STRPTR)"z.library", 53);
		if (!ZBase)
			return 0;
		atexit(close_z);
	}
	return 1;
}

int ib_compress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
	ULONG n = *destLen;
	LONG r;
	if (!open_z())
		return Z_VERSION_ERROR;
	r = Compress(dest, &n, source, sourceLen);
	*destLen = n;
	return (int)r;
}

int ib_uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
	ULONG n = *destLen;
	LONG r;
	if (!open_z())
		return Z_VERSION_ERROR;
	r = Uncompress(dest, &n, source, sourceLen);
	*destLen = n;
	return (int)r;
}

uLong ib_compressBound(uLong sourceLen)
{
	/* zlib's own bound, so it needs no library */
	return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + (sourceLen >> 25) + 13;
}

uLong ib_crc32(uLong crc, const Bytef *buf, unsigned len)
{
	if (!open_z())
		return 0;
	return CRC32(crc, buf, len);
}
