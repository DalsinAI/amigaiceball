#!/usr/bin/env python3
"""Writes ib_gl_procs.c: ib_gl_proc(name), the address of each GL call glad
asks for by name (src/gl/glad.c's load("glX") lines) that GFOpenGPU's libGL
has, from that libGL's symbol list (m68k-amigaos-nm). Calls libGL lacks give
NULL, as a GL driver's GetProcAddress does, and Iceball checks for them.

    gen_gl_procs.py GLAD_C NM_OUTPUT OUT_C

GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team."""
import re
import sys


def glad_names(text):
    return sorted(set(re.findall(r'load\("(gl[A-Za-z0-9_]+)"\)', text)))


def lib_names(nm_text):
    """Defined text symbols in nm output, with the a.out underscore taken off."""
    out = set()
    for line in nm_text.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1] in ('T', 'W'):
            name = parts[2]
            if name.startswith('_gl'):
                name = name[1:]
            if name.startswith('gl'):
                out.add(name)
    return out


def generate(glad_text, nm_text):
    names = [n for n in glad_names(glad_text) if n in lib_names(nm_text)]
    lines = [
        '/* Written by amiga/gen_gl_procs.py: do not edit. */',
        '#include <string.h>',
        '#include <stdlib.h>',
        '',
    ]
    for n in names:
        lines.append('extern char %s[];' % n)
    lines += [
        '',
        'struct ib_gl_entry { const char *name; void *fn; };',
        '',
        'static const struct ib_gl_entry ib_gl_procs[] = {',
    ]
    for n in names:
        lines.append('\t{ "%s", (void *)%s },' % (n, n))
    lines += [
        '};',
        '',
        'static int ib_gl_cmp(const void *k, const void *e)',
        '{',
        '\treturn strcmp((const char *)k, ((const struct ib_gl_entry *)e)->name);',
        '}',
        '',
        'void *ib_gl_proc(const char *name)',
        '{',
        '\tconst struct ib_gl_entry *e = bsearch(name, ib_gl_procs,',
        '\t\tsizeof(ib_gl_procs) / sizeof(ib_gl_procs[0]), sizeof(ib_gl_procs[0]), ib_gl_cmp);',
        '\treturn e ? e->fn : NULL;',
        '}',
        '',
        'int ib_gl_proc_count = %d;' % len(names),
        '',
    ]
    return '\n'.join(lines), names


def main():
    glad, nm, out = sys.argv[1:4]
    text, names = generate(open(glad, encoding='latin-1').read(), open(nm).read())
    if not names:
        sys.exit('gen_gl_procs: libGL has none of the GL calls glad asks for')
    open(out, 'w').write(text)
    print('%s: %d GL calls' % (out, len(names)))


if __name__ == '__main__':
    main()
