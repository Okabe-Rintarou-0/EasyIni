#!/bin/bash
set -e

FLAGS="-std=c++26 -freflection-latest -fexpansion-statements"
LIBS="-L/opt/p2996/clang/lib/x86_64-unknown-linux-gnu -Wl,-rpath,/opt/p2996/clang/lib/x86_64-unknown-linux-gnu"
ROOT="$(cd "$(dirname "$0")" && pwd)"

# --- Download SimpleIni ---
if [ ! -f "$ROOT/SimpleIni.h" ]; then
    echo "=== Downloading SimpleIni ==="
    curl -fsSL https://raw.githubusercontent.com/brofield/simpleini/master/SimpleIni.h -o "$ROOT/SimpleIni.h"
fi

# --- Build benchmark ---
echo "=== Building benchmark ==="
clang++ $FLAGS \
    -I"$ROOT" \
    "$ROOT/benchmark/bench_ini_parser.cpp" \
    -o "$ROOT/benchmark/bench_ini_parser" \
    $LIBS

# --- Run benchmark → CSV ---
echo "=== Running benchmark ==="
"$ROOT/benchmark/bench_ini_parser" > "$ROOT/benchmark/results.csv"
echo "Results saved to benchmark/results.csv"

# --- Generate chart ---
echo "=== Generating chart ==="
python3 "$ROOT/benchmark/plot_results.py"

echo ""
echo "Done. See figures/bench_comparison.png"
