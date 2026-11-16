/* m68k Linux's static glibc has no plain fmod (only the shared library's
 * compat symbol): give the 68k test builds one. Host tests only.
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
double __ieee754_fmod(double, double);
double fmod(double x, double y) { return __ieee754_fmod(x, y); }
