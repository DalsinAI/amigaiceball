#!/usr/bin/env python3
"""Writes a hard-float copy of GFOpenLua's link stub calls
(gfopenlua stub/lua51_calls.c) for programs built -m68881, as Iceball is.

lua51.library is soft-float: a call that returns a lua_Number gives it in
d0/d1, where a hard-float caller looks in fp0. Arguments are the same either
way (a double is 8 bytes on the stack). So the calls that return lua_Number
take the result as the 64-bit integer the library left in d0/d1 and hand back
its bits as a double; every other call is copied as it is.

    lua_fpu_stub.py LUA51_CALLS_C OUT_C

GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team."""
import re
import sys

FUNC = re.compile(
    r'^lua_Number \((?P<name>\w+)\)\((?P<params>[^)]*)\)\n\{\n\treturn gfl_api->(?P=name)\((?P<args>[^)]*)\);\n\}',
    re.M)


def convert(src):
    count = 0

    def repl(m):
        nonlocal count
        count += 1
        return ('lua_Number (%(name)s)(%(params)s)\n{\n'
                '\tunion { unsigned long long bits; lua_Number n; } r;\n'
                '\t/* the soft-float library leaves the double in d0/d1 */\n'
                '\tr.bits = ((unsigned long long (*)())gfl_api->%(name)s)(%(args)s);\n'
                '\treturn r.n;\n}') % m.groupdict()

    out = FUNC.sub(repl, src)
    left = re.findall(r'^lua_Number \(', out, re.M)
    return out, count, len(left) - count


def main():
    src, dst = sys.argv[1:3]
    out, n, missed = convert(open(src).read())
    if n == 0 or missed:
        sys.exit('lua_fpu_stub: %d converted, %d lua_Number calls not understood' % (n, missed))
    open(dst, 'w').write('/* Written by amiga/lua_fpu_stub.py from GFOpenLua\'s stub: do not edit. */\n' + out)
    print('%s: %d lua_Number calls for hard float' % (dst, n))


if __name__ == '__main__':
    main()
