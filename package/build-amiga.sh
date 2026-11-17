#!/bin/sh
# The release archive and the GFOpenUp part, for Cradle's stove_build
# (target package):
#  1. upstream Iceball, sackit and ENet at the commits package/iceball.lock
#     names (not kept in this repository), with amiga/patches applied
#     (package/fetch.sh); GFOpenLua at the lock's commit, built for its
#     lua51.library and headers;
#  2. the game built by amiga/Makefile.amigaos;
#  3. the drawer, its icon and the Installer script beside it packed as
#     NAME-VERSION-68040.lha (tools/pack_dist.py), with its .sha256 and a
#     short .readme;
#  4. OUT/Parts/Iceball, a GFOpenUp part: its Part file and the drawer's
#     files for SYS:Games/Iceball, installed with
#       GFOpenUpTool INSTALL Parts/Iceball
#     It needs the GFOpenGPU part (SDL 2, GL) and z.library (GFOpenZlib).
# Nothing is published from here.
# GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team.
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/.." && pwd)
. "$HERE/package.env"
. "$HERE/iceball.lock"
OUT=${OUT:-"$ROOT/build-amiga-package"}
mkdir -p "$OUT"
OUT=$(CDPATH= cd -- "$OUT" && pwd)
STOVE=${STOVE:-"$HOME/AmiCradle/stoves/os32-gcc16-open6"}
# outside the checkout: Cradle cleans it (git clean -ffdx) before every job
WORK=${WORK:-"$HOME/.cache/amigaiceball"}
cd "$ROOT"

# a stove's compiler is in its prefix/, or STOVE is the prefix
if [ -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ]; then PREFIX="$STOVE/prefix"; else PREFIX="$STOVE"; STOVE=$(dirname "$PREFIX"); fi

# 1. upstream, patched afresh every time; GFOpenLua and its build
sh "$HERE/fetch.sh" "$WORK"
G="$WORK/gfopenlua"; GO="$WORK/gfopenlua-out"
if [ "$(git -C "$G" rev-parse HEAD 2>/dev/null)" != "$GFOPENLUA_COMMIT" ]; then
    rm -rf "$G"; mkdir -p "$G"
    git -C "$G" init -q
    git -C "$G" fetch -q --depth 1 "$GFOPENLUA_URL" "$GFOPENLUA_COMMIT"
    git -C "$G" checkout -q FETCH_HEAD
fi
git -C "$G" checkout -q -- . && git -C "$G" clean -qfdx
rm -rf "$GO"
[ -f "$WORK/lua-5.1.5.tar.gz" ] && export LUA_TGZ="$WORK/lua-5.1.5.tar.gz"
STOVE="$STOVE" OUT="$GO" sh "$G/build-amiga.sh" > "$OUT/gfopenlua-output.txt" 2>&1 ||
    { tail -n 60 "$OUT/gfopenlua-output.txt"; exit 1; }
cp "$GO/lua-5.1.5.tar.gz" "$WORK/" 2>/dev/null || true

# 2. the game
make -f amiga/Makefile.amigaos IB="$WORK/iceball" SACKIT="$WORK/sackit" ENET="$WORK/enet" \
    GFL="$G" GFLOUT="$GO" STOVE="$STOVE" PREFIX="$PREFIX" -j4 $TARGET \
    > "$OUT/build-output.txt" 2>&1 || { tail -n 60 "$OUT/build-output.txt"; exit 1; }

# 3. the archive
python3 "$ROOT/tools/pack_dist.py" --dist "$DIST" --name "$NAME" --out "$OUT"

# 4. the GFOpenUp part
sh "$ROOT/package/write-part.sh" "$DIST/$NAME" "$OUT/Parts/$NAME" "$NAME" "$PORT_VERSION" \
    "Iceball: build, dig and fight in team battles on a voxel island"
echo "part: $(grep -c '^copy' "$OUT/Parts/$NAME/Part") files, $(du -sk "$OUT/Parts/$NAME" | cut -f1) KB"
ls -l "$DIST/$NAME/$NAME"
