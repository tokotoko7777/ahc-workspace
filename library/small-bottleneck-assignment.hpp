#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace small_bottleneck_assignment {

struct Result {
    int latest = 0;
    std::int64_t sum = 0;
    std::vector<int> worker_for_job;
};

// Row-major [job][worker], -1 for forbidden; every job needs a distinct worker.
// O(workers * 2^workers + jobs * workers) time and O(2^workers) memory.
// Minimize largest cost first, then total cost. Costs must be nonnegative
// when present, including INT_MAX. Suitable for a small team (workers <= 16).
inline std::optional<Result> solve(int jobs, int workers,
                                   std::span<const int> costs) {
    if (workers < 0 || workers > 16 || jobs < 0 || jobs > 16 ||
        costs.size() != static_cast<std::size_t>(jobs * workers)) {
        throw std::invalid_argument("small bottleneck assignment: invalid matrix shape");
    }
    if (jobs == 0) return Result{};
    if (jobs > workers) return std::nullopt;
    const int states = 1 << workers;
    const auto inf = std::numeric_limits<std::int64_t>::max();
    auto cost = [&](int j, int i) { return costs[j * workers + i]; };
    std::vector<std::int64_t> dp(states, inf);
    dp[0] = 0;
    int cap = -1;
    for (int mask = 0; mask < states; ++mask) {
        if (dp[mask] == inf) continue;
        const int j = std::popcount(unsigned(mask));
        if (j == jobs) {
            const int value = static_cast<int>(dp[mask]);
            if (cap < 0 || value < cap) cap = value;
            continue;
        }
        for (int i = 0; i < workers; ++i) {
            const int d = cost(j, i), bit = 1 << i;
            if ((mask & bit) || d < 0) continue;
            auto& dst = dp[mask | bit];
            dst = std::min(dst, std::max(dp[mask], std::int64_t(d)));
        }
    }
    if (cap < 0) return std::nullopt;

    // A single lexicographic-state DP is not safe. Later larger costs can
    // erase the earlier max advantage while the total still differs.
    std::fill(dp.begin(), dp.end(), inf);
    std::vector<std::uint8_t> prev(states);
    dp[0] = 0;
    int goal = -1;
    for (int mask = 0; mask < states; ++mask) {
        if (dp[mask] == inf) continue;
        const int j = std::popcount(unsigned(mask));
        if (j == jobs) {
            if (goal < 0 || dp[mask] < dp[goal]) goal = mask;
            continue;
        }
        for (int i = 0; i < workers; ++i) {
            const int d = cost(j, i), bit = 1 << i;
            if ((mask & bit) || d < 0 || d > cap) continue;
            if (const auto nd = dp[mask] + d; nd < dp[mask | bit]) {
                dp[mask | bit] = nd;
                prev[mask | bit] = static_cast<std::uint8_t>(i);
            }
        }
    }
    Result r{cap, dp[goal], std::vector<int>(jobs)};
    for (int j = jobs - 1; j >= 0; --j) {
        int i = prev[goal];
        r.worker_for_job[j] = i;
        goal ^= 1 << i;
    }
    return r;
}
}  // namespace small_bottleneck_assignment
