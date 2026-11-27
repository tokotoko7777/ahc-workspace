#pragma once

#include <array>
#include <bitset>
#include <cstddef>

namespace ahc {
struct TargetBfsStats {
    std::size_t expanded = 0;
    bool all_targets_found = false;
};

// Distances are guaranteed only for marked vertices, up to max_distance.
// Unreached marks remain at inf. Other output entries may be left at inf
// even if reachable. This deliberately is not a complete distance field.
// Contract: start in [0,N), inf > max_distance >= 0; every nonnegative
// neighbor index is < N. Negative neighbors are ignored. open(v) selects
// traversable destinations. The source is always enqueued at distance zero.
// O(N) scratch, no dynamic allocation; BFS expands only the needed frontier.
template <std::size_t N, class Distance, class Neighbors, class Open>
void target_limited_bfs(int start, int max_distance, Distance inf,
                        std::bitset<N> wanted, std::array<Distance, N>& d,
                        Neighbors&& neighbors, Open&& open,
                        TargetBfsStats* stats = nullptr) {
    d.fill(inf);
    d[start] = 0;
    wanted.reset(start);
    if (stats) *stats = {};
    if (wanted.none()) {
        if (stats) stats->all_targets_found = true;
        return;
    }
    std::array<int, N> queue;
    std::size_t len = 1;
    queue[0] = start;
    for (std::size_t i = 0; i < len; ++i) {
        const int p = queue[i];
        const int next = static_cast<int>(d[p]) + 1;
        if (next > max_distance) break;
        if (stats) ++stats->expanded;
        for (int v : neighbors(p)) {
            if (v < 0 || !open(v) || d[v] != inf) continue;
            queue[len++] = v;
            d[v] = static_cast<Distance>(next);
            if (wanted[v]) {
                wanted.reset(v);
                if (wanted.none()) {
                    if (stats) stats->all_targets_found = true;
                    return;
                }
            }
        }
    }
}
}  // namespace ahc
