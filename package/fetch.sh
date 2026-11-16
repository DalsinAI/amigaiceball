#!/bin/sh
# Fetches the upstream sources package/iceball.lock names into $1 (made if
# missing) and applies amiga/patches. Used by build-amiga.sh and the tests.
#   sh package/fetch.sh WORKDIR [host]   ("host" also fetches Lua and zlib)
set -e
HERE=$(cd "$(dirname "$0")/.." && pwd)
W=$1
[ -n "$W" ] || { echo "usage: fetch.sh WORKDIR [host]"; exit 2; }
mkdir -p "$W"
. "$HERE/package/iceball.lock"

get() { # name url commit
    if [ ! -d "$W/$1/.git" ]; then
        rm -rf "$W/$1"
        git init -q "$W/$1"
        git -C "$W/$1" remote add origin "$2"
    fi
    if ! git -C "$W/$1" cat-file -e "$3^{commit}" 2>/dev/null; then
        git -C "$W/$1" fetch -q --depth 1 origin "$3"
    fi
    git -C "$W/$1" checkout -q -f "$3"
    git -C "$W/$1" clean -q -fdx
}

get iceball "$ICEBALL_URL" "$ICEBALL_COMMIT"
get sackit "$SACKIT_URL" "$SACKIT_COMMIT"
get enet "$ENET_URL" "$ENET_COMMIT"
for p in "$HERE"/amiga/patches/iceball/*.patch; do git -C "$W/iceball" apply "$p"; done
for p in "$HERE"/amiga/patches/sackit/*.patch; do git -C "$W/sackit" apply "$p"; done

if [ "$2" = host ]; then
    get zlib "$ZLIB_URL" "$ZLIB_COMMIT"
    if [ ! -f "$W/lua-5.1.5.tar.gz" ]; then
        curl -fsSL -o "$W/lua-5.1.5.tar.gz" "$LUA_URL" || curl -fsSL -o "$W/lua-5.1.5.tar.gz" "$LUA_MIRROR"
    fi
    echo "$LUA_SHA256  $W/lua-5.1.5.tar.gz" | sha256sum -c - >/dev/null
    rm -rf "$W/lua-5.1.5"
    tar xzf "$W/lua-5.1.5.tar.gz" -C "$W"
fi
