#!/bin/bash
set -e

FLAGS="-std=c++26 -freflection-latest -fexpansion-statements"
LIBS="-L/opt/p2996/clang/lib/x86_64-unknown-linux-gnu -Wl,-rpath,/opt/p2996/clang/lib/x86_64-unknown-linux-gnu"
ROOT="$(cd "$(dirname "$0")" && pwd)"

# --- Google Test ---
GTEST_DIR="$ROOT/build/googletest"
if [ ! -f "$GTEST_DIR/lib/libgtest.a" ]; then
    echo "=== Building Google Test ==="
    mkdir -p "$GTEST_DIR"
    if [ ! -d "$GTEST_DIR/src" ]; then
        git clone --depth 1 https://github.com/google/googletest.git "$GTEST_DIR/src"
    fi
    mkdir -p "$GTEST_DIR/build"
    cmake -S "$GTEST_DIR/src" -B "$GTEST_DIR/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$GTEST_DIR/build" --parallel
    mkdir -p "$GTEST_DIR/lib"
    cp "$GTEST_DIR/build/lib/libgtest.a" "$GTEST_DIR/lib/"
    cp "$GTEST_DIR/build/lib/libgtest_main.a" "$GTEST_DIR/lib/"
fi

echo "=== Build: main ==="
clang++ $FLAGS ini_parser.cpp -o ini_parser $LIBS

echo "=== Build & run: unit tests ==="
clang++ $FLAGS test/test_ini_parser.cpp \
    -I"$GTEST_DIR/src/googletest/include" \
    -o test/test_ini_parser \
    "$GTEST_DIR/lib/libgtest.a" "$GTEST_DIR/lib/libgtest_main.a" -lpthread \
    $LIBS
./test/test_ini_parser

echo ""
echo "All OK"
