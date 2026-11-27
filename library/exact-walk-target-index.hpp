#pragma once
#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace ahc {

// Exact-length walk queries to a fixed target set on a fixed undirected graph.
// Each allowed edge must be present in both adjacency lists (self-loops allowed).
// Negative neighbor slots are ignored. Vertices in walls cannot be traversed.
// A completely isolated open vertex can optionally stay; non-isolated ones
// must use an edge for each step. Build again if any graph input changes.
//
// Build O(V+E), memory O(V), query O(1). A finite shortest distance of the
// requested parity is sufficient: each remaining pair of steps can use an
// arbitrary open edge from the start and its reverse.
template<std::size_t N>
class ExactWalkTargetIndex {
    static_assert(N > 0 && N < std::size_t(std::numeric_limits<int>::max()) / 2);
public:
    template<class Neighbors>
    ExactWalkTargetIndex(const Neighbors& graph, const std::bitset<N>& walls,
                         const std::bitset<N>& targets, bool stay_at_isolated = false)
        : stay_(stay_at_isolated) {
        distance_[0].fill(-1);
        distance_[1].fill(-1);
        std::vector<int> queue;
        queue.reserve(2 * N);
        for (std::size_t v = 0; v < N; ++v) {
            if (walls[v]) continue;
            for (auto x : graph[v]) {
                if (x < 0) continue;
                const auto u = static_cast<std::size_t>(x);
                assert(u < N);
                if (!walls[u]) { has_open_edge_.set(v); break; }
            }
            if (targets[v]) {
                distance_[0][v] = 0;
                queue.push_back(2 * static_cast<int>(v));
            }
        }
        for (std::size_t head = 0; head < queue.size(); ++head) {
            const int p = queue[head] & 1;
            const int v = queue[head] >> 1;
            const int next_distance = distance_[p][v] + 1;
            for (auto raw : graph[v]) {
                if (raw < 0) continue;
                const auto u = static_cast<std::size_t>(raw);
                assert(u < N);
                if (walls[u] || distance_[1 - p][u] >= 0) continue;
                distance_[1 - p][u] = next_distance;
                queue.push_back(2 * static_cast<int>(u) + 1 - p);
            }
        }
    }

    bool can_reach(int source, std::size_t steps) const {
        assert(source >= 0 && static_cast<std::size_t>(source) < N);
        if (!has_open_edge_[source])
            return distance_[0][source] == 0 && (steps == 0 || stay_);
        const int d = distance_[steps & 1][source];
        return d >= 0 && static_cast<std::size_t>(d) <= steps;
    }

private:
    std::array<std::array<int, N>, 2> distance_;
    std::bitset<N> has_open_edge_;
    bool stay_;
};
}  // namespace ahc
