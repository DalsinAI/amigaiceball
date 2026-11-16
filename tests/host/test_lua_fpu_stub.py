#!/usr/bin/env python3
"""amiga/lua_fpu_stub.py: a hard-float (-m68881) program calling a
soft-float library through the converted stub gets the right lua_Number
back. Built with m68k-linux-gnu-gcc and run under qemu-m68k, whose calling
convention is the AmigaOS one here: hard float returns a double in fp0, soft
float in d0/d1, and both pass it on the stack.
GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team."""
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'amiga'))
import lua_fpu_stub  # noqa: E402

# the shape of gfopenlua stub/lua51_calls.c (tools/gen_api.py writes it)
STUB = '''/* Written by tools/gen_api.py from api/lua51.api: do not edit. */
#include "gfl_api.h"

extern const struct GFLuaAPI *gfl_api;

int (lua_gettop)(lua_State *L)
{
	return gfl_api->lua_gettop(L);
}

lua_Number (lua_tonumber)(lua_State *L, int idx)
{
	return gfl_api->lua_tonumber(L, idx);
}

void (lua_pushnumber)(lua_State *L, lua_Number n)
{
	gfl_api->lua_pushnumber(L, n);
}

lua_Number (luaL_optnumber)(lua_State *L, int nArg, lua_Number def)
{
	return gfl_api->luaL_optnumber(L, nArg, def);
}
'''
API_H = '''typedef double lua_Number;
typedef struct lua_State lua_State;
struct GFLuaAPI {
	int (*lua_gettop)(lua_State *L);
	lua_Number (*lua_tonumber)(lua_State *L, int idx);
	void (*lua_pushnumber)(lua_State *L, lua_Number n);
	lua_Number (*luaL_optnumber)(lua_State *L, int nArg, lua_Number def);
};
'''
# the library side, soft float, with no float arithmetic (no soft-float
# helpers needed): numbers move as bits
LIB = r'''#include "gfl_api.h"
static unsigned long long last;
static int gettop(lua_State *L) { (void)L; return 7; }
static lua_Number tonumber(lua_State *L, int idx)
{
	union { unsigned long long u; lua_Number n; } r;
	(void)L;
	r.u = 0x4004000000000000ull;    /* 2.5 */
	if (idx == 2) r.u = 0xC0C3880000000000ull;   /* -10000 */
	return r.n;
}
static void pushnumber(lua_State *L, lua_Number n)
{
	union { unsigned long long u; lua_Number n; } r;
	(void)L; r.n = n; last = r.u;
}
static lua_Number optnumber(lua_State *L, int a, lua_Number def) { (void)L; (void)a; return def; }
static const struct GFLuaAPI api = { gettop, tonumber, pushnumber, optnumber };
const struct GFLuaAPI *gfl_api = &api;
unsigned long long lib_last(void) { return last; }
'''
MAIN = r'''#include <stdio.h>
#include "gfl_api.h"
int lua_gettop(lua_State *L);
lua_Number lua_tonumber(lua_State *L, int idx);
void lua_pushnumber(lua_State *L, lua_Number n);
lua_Number luaL_optnumber(lua_State *L, int nArg, lua_Number def);
unsigned long long lib_last(void);
int main(void)
{
	int bad = 0;
	volatile lua_Number x = 0.75;
	bad |= lua_gettop(0) != 7;
	bad |= lua_tonumber(0, 1) != 2.5;
	bad |= lua_tonumber(0, 2) != -10000.0;
	bad |= lua_tonumber(0, 1) * 4.0 != 10.0;
	lua_pushnumber(0, x);
	bad |= lib_last() != 0x3FE8000000000000ull;
	bad |= luaL_optnumber(0, 3, -0.125) != -0.125;
	printf("%s\n", bad ? "wrong" : "right");
	return bad;
}
'''


def main():
    out, n, missed = lua_fpu_stub.convert(STUB)
    assert n == 2 and missed == 0, (n, missed)
    assert 'return gfl_api->lua_gettop(L);' in out
    assert 'gfl_api->lua_pushnumber(L, n);' in out
    cc = shutil.which('m68k-linux-gnu-gcc')
    qemu = shutil.which('qemu-m68k')
    if not cc or not qemu:
        print('lua_fpu_stub: 4 checks (no m68k compiler: ABI run skipped)')
        return
    with tempfile.TemporaryDirectory() as d:
        for name, text in (('gfl_api.h', API_H), ('lib.c', LIB), ('main.c', MAIN), ('calls.c', out)):
            open(os.path.join(d, name), 'w').write(text)
        objs = []
        for src, flags in (('lib.c', ['-msoft-float']), ('calls.c', ['-std=gnu99']), ('main.c', [])):
            o = os.path.join(d, src + '.o')
            subprocess.check_call([cc, '-O2', '-m68040', '-I', d, '-c', os.path.join(d, src), '-o', o] + flags)
            objs.append(o)
        exe = os.path.join(d, 't')
        subprocess.check_call([cc, '-static', '-o', exe] + objs)
        res = subprocess.run([qemu, '-cpu', 'm68040', exe], capture_output=True, text=True)
        assert res.returncode == 0 and res.stdout.strip() == 'right', res.stdout + res.stderr
        # and the plain stub, the reason for all this, gets it wrong
        o = os.path.join(d, 'plain.o')
        open(os.path.join(d, 'plain.c'), 'w').write(STUB)
        subprocess.check_call([cc, '-O2', '-m68040', '-I', d, '-c', os.path.join(d, 'plain.c'), '-o', o])
        subprocess.check_call([cc, '-static', '-o', exe, objs[0], o, objs[2]])
        res = subprocess.run([qemu, '-cpu', 'm68040', exe], capture_output=True, text=True)
        assert res.stdout.strip() == 'wrong', 'the plain stub should fail here: ' + res.stdout
    print('lua_fpu_stub: 6 checks (hard-float caller, soft-float library, under qemu-m68k)')


if __name__ == '__main__':
    main()
