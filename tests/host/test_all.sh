#!/bin/sh
# Iceball host tests: builds Iceball's dedicated server, sackit and the WAV
# parser twice, for the PC and for big-endian m68k Linux (run under
# qemu-m68k), and checks both read the game's maps, models, pictures, sounds,
# music and packed numbers the same. Then the Amiga-side helpers' own tests.
#   sh tests/host/test_all.sh [WORKDIR]
# Needs: cc, m68k-linux-gnu-gcc, qemu-m68k, git, curl, python3.
set -e
HERE=$(cd "$(dirname "$0")/../.." && pwd)
W=${1:-${TMPDIR:-/tmp}/iceball-host-test}
T=$HERE/tests/host
sh "$HERE/package/fetch.sh" "$W" host
IB=$W/iceball
FAIL=0
ok() { echo "PASS $1"; }
bad() { echo "FAIL $1"; FAIL=1; }

deps() { # arch cc: Lua 5.1, zlib and ENet as static libraries
    D=$W/deps-$1; mkdir -p "$D/o"
    for f in "$W"/lua-5.1.5/src/*.c; do
        case $(basename "$f") in lua.c|luac.c|print.c) continue;; esac
        $2 -O1 -c "$f" -o "$D/o/lua_$(basename "$f" .c).o"
    done
    ar rcs "$D/liblua.a" "$D"/o/lua_*.o
    for f in adler32 crc32 deflate inflate inffast inftrees trees zutil compress uncompr; do
        $2 -O1 -c "$W/zlib/$f.c" -o "$D/o/z_$f.o"
    done
    ar rcs "$D/libz.a" "$D"/o/z_*.o
    for f in callbacks compress host list packet peer protocol unix; do
        $2 -O1 -DHAS_SOCKLEN_T -I"$W/enet/include" -c "$W/enet/$f.c" -o "$D/o/enet_$f.o"
    done
    ar rcs "$D/libenet.a" "$D"/o/enet_*.o
    for f in objects playroutine playroutine_effects playroutine_nna effects fixedmath tables mixer; do
        $2 -O1 -w -c "$W/sackit/$f.c" -o "$D/o/sk_$f.o"
    done
    ar rcs "$D/libsackit.a" "$D"/o/sk_*.o
    rm -f "$D/libcompat.a"
    if [ "$1" = m68k ]; then
        $2 -O1 -c "$T/m68k_linux_compat.c" -o "$D/o/compat.o"
        ar rcs "$D/libcompat.a" "$D/o/compat.o"
    else
        ar rcs "$D/libcompat.a"
    fi
}

build() { # arch cc ldflags
    deps "$1" "$2"
    D=$W/deps-$1
    INC="-I$W/lua-5.1.5/src -I$W/enet/include -I$W/zlib -I$HERE/amiga"
    make -s -C "$IB" -f Makefile.dedi CC="$2" HEADERS_ENet= HEADERS_Lua="$INC" \
        CFLAGS_EXTRA="-w" LDFLAGS="$3" OBJDIR="build/dedi-$1" BINNAME="dedi-$1" \
        LIBS="$D/liblua.a $D/libz.a $D/libenet.a $D/libcompat.a -lm"
    $2 -O1 -w -I"$W/sackit" -o "$W/it-$1" "$T/it_render.c" "$D/libsackit.a" "$D/libcompat.a" $3 -lm
    mkdir -p "$IB/tests/host"
    cp "$T/wav_parse.c" "$IB/tests/host/"
    $2 -O1 -w -DDEDI -I"$IB/include" $INC -o "$W/wav-$1" "$IB/tests/host/wav_parse.c" \
        "$D/liblua.a" "$D/libz.a" "$D/libcompat.a" $3 -lm
}

run() { # how: the server prints IBTEST lines, then "done", then keeps running
    out=$1; shift
    rm -f "$out"
    ( cd "$IB" && "$@" -d 0 pkg/ibtest > "$out" 2>&1 & echo $! > "$out.pid" )
    n=0
    until grep -qa "IBTEST	done" "$out" 2>/dev/null; do
        n=$((n + 1)); [ $n -lt 1200 ] || break
        sleep 1
    done
    kill "$(cat "$out.pid")" 2>/dev/null || true
    grep -a "^IBTEST" "$out" > "$out.lines" || true
}

build pc cc ""
build m68k m68k-linux-gnu-gcc "-static"
QEMU="qemu-m68k -cpu m68040"

# the test mod, with every model and picture the game ships
mkdir -p "$IB/pkg/ibtest"
M=$(cd "$IB" && find pkg -name '*.pmf' ! -path 'pkg/ibtest/*' | sort | sed 's/.*/"&"/' | paste -sd, -)
I=$(cd "$IB" && find pkg \( -name '*.tga' -o -name '*.png' \) ! -path 'pkg/ibtest/*' | sort | sed 's/.*/"&"/' | paste -sd, -)
sed -e "s|{ MODELS }|{ $M }|" -e "s|{ IMAGES }|{ $I }|" "$T/ibtest/main_server.lua" > "$IB/pkg/ibtest/main_server.lua"

run "$W/pc.txt" stdbuf -oL ./dedi-pc
run "$W/m68k.txt" $QEMU ./dedi-m68k
if [ -s "$W/pc.txt.lines" ] && grep -q "IBTEST	done" "$W/pc.txt.lines" && cmp -s "$W/pc.txt.lines" "$W/m68k.txt.lines"; then
    ok "maps, models, pictures, packed numbers: $(wc -l < "$W/pc.txt.lines") lines the same on PC and 68k"
else
    bad "maps, models, pictures, packed numbers differ"; diff "$W/pc.txt.lines" "$W/m68k.txt.lines" | head -20
fi
grep -q "IBTEST	unpack	-5	250	-1234	54321	-123456789	3000000000	1.5	-2.25	hi" "$W/pc.txt.lines" && ok "unpack values" || bad "unpack values"

ITS=$(cd "$IB" && find pkg -name '*.it' | sort)
(cd "$IB" && "$W/it-pc" $ITS) > "$W/it-pc.txt"
(cd "$IB" && $QEMU "$W/it-m68k" $ITS) > "$W/it-m68k.txt"
cmp -s "$W/it-pc.txt" "$W/it-m68k.txt" && ok "music: $(wc -l < "$W/it-pc.txt") lines the same" || { bad music; diff "$W/it-pc.txt" "$W/it-m68k.txt" | head; }

WAVS=$(cd "$IB" && find pkg -name '*.wav' | sort)
(cd "$IB" && "$W/wav-pc" $WAVS 2>/dev/null) > "$W/wav-pc.txt"
(cd "$IB" && $QEMU "$W/wav-m68k" $WAVS 2>/dev/null) > "$W/wav-m68k.txt"
cmp -s "$W/wav-pc.txt" "$W/wav-m68k.txt" && ok "sounds: $(wc -l < "$W/wav-pc.txt") the same" || { bad sounds; diff "$W/wav-pc.txt" "$W/wav-m68k.txt" | head; }

# the Amiga-side helpers
for t in "$T"/test_*.py; do python3 "$t" && ok "$(basename "$t")" || bad "$(basename "$t")"; done
for t in "$T"/test_*.c; do
    [ -f "$t" ] || continue
    cc -O1 -Wall -I"$HERE/amiga" -o "$W/t" "$t" && "$W/t" && ok "$(basename "$t") (PC)" || bad "$(basename "$t") (PC)"
    m68k-linux-gnu-gcc -O1 -static -I"$HERE/amiga" -o "$W/t68" "$t" && $QEMU "$W/t68" && ok "$(basename "$t") (68k)" || bad "$(basename "$t") (68k)"
done

[ $FAIL = 0 ] && echo "ALL PASS" || { echo "SOME FAILED"; exit 1; }
