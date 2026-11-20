#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

cpu_threads=${1:-16}
rounds=${2:-5}
if (( $# > 2 )) || [[ ! $cpu_threads =~ ^[1-9][0-9]*$ ]] || (( cpu_threads > 64 )) \
    || [[ ! $rounds =~ ^[1-9][0-9]*$ ]]; then
    echo "Usage: bash run_integer_vector.sh [CPU threads: 1..64, default 16] [rounds, default 5]" >&2
    exit 2
fi
export BFS_CPU_THREADS="$cpu_threads"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target bfs_benchmark_integer_vector -j2

results="results_integer_vector_$(date +%Y%m%d_%H%M%S)_$$"
mkdir -p "$results"
printf 'BFS_CPU_THREADS=%s\nRounds=%s\n' "$cpu_threads" "$rounds" > "$results/config.txt"
sha256sum bfs_benchmark_integer_vector.asc benchmark_parallel.hpp benchmark_graph.hpp \
    CMakeLists.txt run_integer_vector.sh build/bfs_benchmark_integer_vector > "$results/sha256.txt"
for (( round=1; round<=rounds; round++ )); do
    printf '\n=== Integer vector round %s/%s ===\n' "$round" "$rounds"
    ./build/bfs_benchmark_integer_vector 2>&1 | tee "$results/integer_vector_${round}.log"
    if ! grep -Fxq 'All BFS tests completed. All correctness checks passed.' "$results/integer_vector_${round}.log"; then
        echo "Round $round did not complete all correctness checks; stopping." >&2
        exit 1
    fi
done
printf '\nResults: %s/%s\n' "$PWD" "$results"
