Iceball for GFOS
================

Iceball is a game of building, digging and team battles on an island made
of blocks, with mods written in Lua. This is it for GFOS (AmigaOS 3.2 with
GFOpenUp) on a 68040 or 68060 with an FPU.

What it needs
-------------

- The GFOpenGPU part: SDL 2 and OpenGL (LIBS:GFOpenGPU/).
- z.library (GFOpenZlib) in LIBS:, for pictures and file transfers.
- lua51.library: the drawer's own Libs/ has it, and LIBS: is used first
  when it is there.
- For network games only: a TCP/IP stack (Roadshow, AmiTCP, Miami or
  GFOS's own) running before Iceball starts.

Starting it
-----------

Double-click Iceball. With a TCP/IP stack running it shows Iceball's own
launcher: L for a local game, a number to join a server from the list, C
for settings. With no stack it goes straight into a local game on the base
mod, since the launcher's other choices need the network.

From a Shell, in the drawer, upstream's options work:

    Iceball -s 0 pkg/base            local game, no network
    Iceball -s 20737 pkg/base        local game others can join
    Iceball -c HOST PORT             join a server

clsave/config.json holds the screen size, full screen and the GL and sound
settings; ours is upstream's with 640 x 480 full screen, no GL shaders, no
FBOs, no smooth lighting and sound at 22050 Hz. Controls lists the keys.

How the port works
------------------

- Upstream's source is fetched at build time (package/iceball.lock), never
  kept here, with amiga/patches applied. The same goes for sackit (its .it
  music player) and ENet (its network layer).
- Byte order: Iceball read its files and packets by laying little-endian
  bytes over integers, which gives wrong numbers on the 68k. The patch reads
  and writes them byte by byte (amiga/ib_le.h): IceMap and PMF files, WAV
  sounds, TGA and PNG pictures, file-transfer and long-packet headers, and
  the numbers the mods pack with common.net_pack (so an Amiga plays with PC
  servers). Pixels stay 0xAARRGGBB values and go to GL as
  GL_UNSIGNED_INT_8_8_8_8_REV, which is right on either byte order. sackit's
  .it loader gets the same treatment. Two upstream bugs fixed on the way:
  the server-list lookup freed an unset pointer when there was no network,
  and net_unpack's 'd' read only 4 of a double's 8 bytes.
- GL: GFOpenGPU's libGL has the GL calls themselves, so glad (Iceball's GL
  loader) gets them by name from a table made at build time
  (amiga/gen_gl_procs.py) of the calls glad asks for that libGL has.
- Lua: lua51.library (GFOpenLua) instead of a compiled-in Lua. That
  library is soft-float and Iceball is built for the FPU, which return a
  double in different registers, so the link stub is rebuilt with its three
  number-returning calls taking the library's result from d0/d1
  (amiga/lua_fpu_stub.py).
- zlib: z.library (amiga/zlib.h, amiga/ib_zlib.c).
- Sockets: bsdsocket.library through amiga/ib_bsdsocket.h and ib_amiga.c,
  IPv4 only; with no stack every call fails cleanly and the local game,
  which needs no socket, still runs.

Tests
-----

`sh tests/host/test_all.sh` (needs cc, m68k-linux-gnu-gcc and qemu-m68k)
builds Iceball's dedicated server, sackit and the WAV parser for the PC and
for big-endian m68k Linux, and checks both read every map, model, picture,
sound and .it file the game ships, and pack numbers, the same (without the
patch the 68k build differs on 91 of 93 lines). It also tests the Amiga
helpers: the byte-order and IPv4 helpers on both, the GL table generator,
and the hard-float Lua stub, whose calls into a soft-float library are run
under qemu-m68k (the unconverted stub gets them wrong there).

Building
--------

`sh package/build-amiga.sh` with STOVE= an os32-gcc16-open6 stove: the
drawer in build/amigaos/, the .lha archive and the GFOpenUp part in
build-amiga-package/.
