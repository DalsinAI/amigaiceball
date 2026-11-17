# Iceball for GFOS

Iceball (the Iceball team, 2012-2016; GPL-3.0 code, its data under the
licences in its own `LICENCE-*` files) for AmigaOS 3.2 with GFOpenUp,
68040/68060 + FPU, as a release LhA with an Installer script and as the
GFOpenUp part `Parts/Iceball`.

- Upstream: <https://github.com/iamgreaser/iceball> with sackit (IT music)
  and ENet, at the commits `package/iceball.lock` names, fetched at build
  time, patched by `amiga/patches`.
- Lua is lua51.library from GFOpenLua (DalsinAI/gfopenlua, at the locked
  commit), shipped in the drawer's `Libs/`; compression is z.library
  (GFOpenZlib); sockets go to bsdsocket.library through `amiga/ib_amiga.c`.
- GL is the stove's libGL; the formats are read in the PC's byte order
  through `amiga/ib_le.h`.

Build (Cradle `stove_build`, target `package`): `sh package/build-amiga.sh`
with `STOVE=` (os32-gcc16-open6) and `OUT=`.

Host tests: `sh tests/host/test_all.sh` builds the dedicated server, the IT
player and the WAV reader for the PC and for 68k Linux, runs them (68k under
`qemu-m68k -cpu m68040`) and compares every map, model, picture, packed
number, song and sound; then the unit tests in `tests/host/test_*`.

GPL-3.0-or-later for the port's own files, like Iceball. See
`amiga/Licences.txt`.

Team 3
