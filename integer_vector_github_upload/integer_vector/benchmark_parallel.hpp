#pragma once

#include <algorithm>
#include <chrono>
#include <string>
#include <cstdlib>
#include <future>
#include <stdexcept>
#include <thread>
#include <vector>

namespace benchmark {

inline int cpu_threads() {
    if (const char* setting = std::getenv("BFS_CPU_THREADS")) {
        char* end = nullptr;
        const long count = std::strtol(setting, &end, 10);
        if (end == setting || *end != '\0' || count < 1 || count > 64)
            throw std::invalid_argument("BFS_CPU_THREADS must be between 1 and 64");
        return static_cast<int>(count);
    }
    return static_cast<int>(std::clamp(std::thread::hardware_concurrency(), 1u, 64u));
}

// Split the layer by disjoint target ranges, then wait before advancing the frontier.
template <typename Work>
void parallel_ranges(int targets, int threads, const Work& work) {
    const int count = std::min(targets, threads);
    if (count <= 1) {
        work(0, targets);
        return;
    }

    std::vector<std::future<void>> workers;
    workers.reserve(count - 1);
    for (int worker = 1; worker < count; ++worker) {
        const int begin = targets * worker / count;
        const int end = targets * (worker + 1) / count;
        workers.emplace_back(std::async(std::launch::async, work, begin, end));
    }
    work(0, targets / count);
    for (auto& worker : workers) worker.get();
}

struct CpuTimings {
    double single_ms = 0;
    double multi_ms = 0;
};

template <typename Work, typename Result>
CpuTimings measure_cpu(int n, int threads, int repeats, const Result& reference, const Work& run) {
    using Clock = std::chrono::steady_clock;
    auto measure = [&](int worker_count, double& total_ms) {
        const auto start = Clock::now();
        const auto result = run(worker_count);
        total_ms += std::chrono::duration<double, std::milli>(Clock::now() - start).count();
        if (result != reference)
            throw std::runtime_error("CPU result mismatch at n=" + std::to_string(n)
                                     + ", threads=" + std::to_string(worker_count));
    };

    // Warm up both CPU modes before alternating their timed run order.
    CpuTimings warmup;
    measure(1, warmup.single_ms);
    measure(threads, warmup.multi_ms);

    CpuTimings timing;
    for (int r = 0; r < repeats; ++r) {
        if (r % 2 == 0) {
            measure(1, timing.single_ms);
            measure(threads, timing.multi_ms);
        } else {
            measure(threads, timing.multi_ms);
            measure(1, timing.single_ms);
        }
    }
    timing.single_ms /= repeats;
    timing.multi_ms /= repeats;
    return timing;
}

} // namespace benchmark
