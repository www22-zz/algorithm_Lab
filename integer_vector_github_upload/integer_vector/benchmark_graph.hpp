#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace benchmark {

inline constexpr int max_n = 8192;
inline constexpr std::array<int, 8> sizes{64, 128, 256, 512, 1024, 2048, 4096, max_n};
inline constexpr double extra_edge_probability = 0.10;

template <typename T>
std::vector<T> generate_graph(int n) {
    std::vector<T> adj(static_cast<std::size_t>(n) * n, T{0});
    auto add_undirected_edge = [&](int u, int v) {
        adj[static_cast<std::size_t>(u) * n + v] = T{1};
        adj[static_cast<std::size_t>(v) * n + u] = T{1};
    };

    for (int u = 0; u < n; ++u)
        add_undirected_edge(u, (u + 1) % n);

    std::mt19937 rng(static_cast<std::uint32_t>(2026 + n));
    std::bernoulli_distribution add_edge(extra_edge_probability);
    for (int u = 0; u < n; ++u)
        for (int v = u + 1; v < n; ++v)
            if (add_edge(rng))
                add_undirected_edge(u, v);

    return adj;
}

} // namespace benchmark
