/* Loads each .it file through sackit (with our byte-order patch) and plays
 * the first 200 buffers with the integer IT 2.11 mixer, printing a hash of
 * the module's fields and of the sound. The PC and the 68k (big-endian)
 * builds must print the same.
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sackit.h"

static uint32_t fnv(uint32_t h, const void *p, size_t n)
{
	const uint8_t *b = (const uint8_t *)p;
	while (n--) { h ^= *b++; h *= 16777619u; }
	return h;
}

static uint32_t fnv32(uint32_t h, uint32_t v)
{
	uint8_t b[4] = { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) };
	return fnv(h, b, 4);
}

int main(int argc, char **argv)
{
	int a;
	for (a = 1; a < argc; a++) {
		FILE *f = fopen(argv[a], "rb");
		long len;
		char *buf;
		it_module_t *m;
		sackit_playback_t *pb;
		uint32_t h = 2166136261u;
		int i, j;

		if (!f) { printf("%s: cannot open\n", argv[a]); return 1; }
		fseek(f, 0, SEEK_END); len = ftell(f); fseek(f, 0, SEEK_SET);
		buf = malloc(len);
		if (fread(buf, 1, len, f) != (size_t)len) return 1;
		fclose(f);
		m = sackit_module_load_memory(buf, len);
		if (!m) { printf("%s: did not load\n", argv[a]); return 1; }

		h = fnv32(h, m->header.ordnum); h = fnv32(h, m->header.insnum);
		h = fnv32(h, m->header.smpnum); h = fnv32(h, m->header.patnum);
		h = fnv32(h, m->header.flags); h = fnv32(h, m->header.special);
		for (i = 0; i < m->header.insnum; i++)
			h = fnv32(h, m->instruments[i]->fadeout);
		for (i = 0; i < m->header.smpnum; i++) {
			it_sample_t *s = m->samples[i];
			h = fnv32(h, s->length); h = fnv32(h, s->loop_begin); h = fnv32(h, s->loop_end);
			h = fnv32(h, s->c5speed);
			if (s->data)
				for (j = 0; j < (int)s->length; j++)
					h = fnv32(h, (uint16_t)s->data[j]);
		}
		for (i = 0; i < m->header.patnum; i++)
			if (m->patterns[i]) {
				h = fnv32(h, m->patterns[i]->length);
				h = fnv32(h, m->patterns[i]->rows);
			}
		printf("%s module %08x\n", argv[a], h);

		pb = sackit_playback_new(m, 4096, 256, MIXER_IT211);
		h = 2166136261u;
		for (i = 0; i < 200; i++) {
			sackit_playback_update(pb);
			for (j = 0; j < pb->buf_len; j++)
				h = fnv32(h, (uint16_t)pb->buf[j]);
		}
		printf("%s sound %08x\n", argv[a], h);
		sackit_playback_free(pb);
		sackit_module_free(m);
		free(buf);
	}
	return 0;
}
