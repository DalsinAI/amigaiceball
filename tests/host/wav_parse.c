/* Parses each .wav through Iceball's own wav_parse (src/wav.c, with our
 * byte-order patch) and prints its rate, length and a hash of the samples.
 * The PC and the 68k (big-endian) builds must print the same.
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#include "../../src/wav.c"

/* wav_load's file reader, not used here */
char *net_fetch_file(const char *fname, int *flen)
{
	(void)fname; (void)flen;
	return NULL;
}

int main(int argc, char **argv)
{
	int a;
	for (a = 1; a < argc; a++) {
		FILE *f = fopen(argv[a], "rb");
		long len;
		char *buf;
		wav_t *w;
		uint32_t h = 2166136261u, i;

		if (!f) { printf("%s: cannot open\n", argv[a]); return 1; }
		fseek(f, 0, SEEK_END); len = ftell(f); fseek(f, 0, SEEK_SET);
		buf = malloc(len);
		if (fread(buf, 1, len, f) != (size_t)len) return 1;
		fclose(f);
		w = wav_parse(buf, (int)len);
		if (!w) { printf("%s: no\n", argv[a]); free(buf); continue; }
		for (i = 0; i < w->len; i++) {
			uint16_t v = (uint16_t)w->data[i];
			h = (h ^ (v & 0xFF)) * 16777619u;
			h = (h ^ (v >> 8)) * 16777619u;
		}
		printf("%s %u %u %08x\n", argv[a], (unsigned)w->freq, (unsigned)w->len, h);
		free(w);
		free(buf);
	}
	return 0;
}
