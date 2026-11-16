#!/usr/bin/env python3
"""amiga/gen_gl_procs.py: only the GL calls both glad asks for and libGL has,
found by name, NULL for the rest; built and run with the PC's compiler.
GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team."""
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'amiga'))
import gen_gl_procs  # noqa: E402

GLAD = '''
	glad_glClear = (PFNGLCLEARPROC)load("glClear");
	glad_glBegin = (PFNGLBEGINPROC)load("glBegin");
	glad_glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)load("glTexStorage2D");
	glad_glClear = (PFNGLCLEARPROC)load("glClear");
'''
NM = '''
gl_clear.o:
00000000 T _glClear
00000010 W _glBegin
00000020 T _glVertex3f
00000000 D _glState
         U _malloc
'''
MAIN = r'''
#include <stdio.h>
void *ib_gl_proc(const char *name);
extern int ib_gl_proc_count;
char glClear[4], glBegin[4];
int main(void)
{
	int bad = 0;
	bad |= ib_gl_proc("glClear") != (void *)glClear;
	bad |= ib_gl_proc("glBegin") != (void *)glBegin;
	bad |= ib_gl_proc("glTexStorage2D") != NULL;   /* glad wants it, libGL lacks it */
	bad |= ib_gl_proc("glVertex3f") != NULL;       /* libGL has it, glad never asks */
	bad |= ib_gl_proc("glNothing") != NULL;
	bad |= ib_gl_proc_count != 2;
	return bad;
}
'''


def main():
    assert gen_gl_procs.glad_names(GLAD) == ['glBegin', 'glClear', 'glTexStorage2D']
    assert gen_gl_procs.lib_names(NM) == {'glClear', 'glBegin', 'glVertex3f'}
    text, names = gen_gl_procs.generate(GLAD, NM)
    assert names == ['glBegin', 'glClear'], names
    with tempfile.TemporaryDirectory() as d:
        open(os.path.join(d, 'procs.c'), 'w').write(text)
        open(os.path.join(d, 'main.c'), 'w').write(MAIN)
        exe = os.path.join(d, 't')
        subprocess.check_call(['cc', '-Wall', '-o', exe, os.path.join(d, 'procs.c'), os.path.join(d, 'main.c')])
        assert subprocess.call([exe]) == 0, 'lookup gave the wrong address'
    print('gen_gl_procs: 4 checks')


if __name__ == '__main__':
    main()
