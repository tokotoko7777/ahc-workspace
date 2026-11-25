#ifndef AHC_MAX_SAFE_OPENING_HPP
#define AHC_MAX_SAFE_OPENING_HPP

#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>

namespace ahc {

template <std::size_t V>
struct SafeOpeningResult {
    bool feasible = false;
    std::bitset<V> opened;
    std::bitset<V> reachable;
};

// On a static undirected graph, maximize the root component by opening only
// eligible blocked vertices, without reaching an initially unblocked hazard.
// neighbors(v) must enumerate IDs in [0,V), or negative sentinels.
// There are no costs, limits on the number of openings, or moving vertices.
template <std::size_t V, class Neighbors>
SafeOpeningResult<V> max_safe_opening(
    int root, const std::bitset<V>& blocked, const std::bitset<V>& eligible,
    const std::bitset<V>& hazards, const Neighbors& neighbors) {
    assert(root >= 0 && std::size_t(root) < V && !blocked[root]);
    assert((blocked & hazards).none());

    auto hazardous = hazards;
    std::bitset<V> protected_boundary;
    std::array<int, V> queue;
    std::size_t head = 0, tail = 0;
    for (std::size_t i = 0; i < V; ++i)
        if (hazards[i]) queue[tail++] = static_cast<int>(i);
    for (; head < tail; ++head)
        for (int v : neighbors(queue[head])) {
            if (v < 0) continue;
            assert(std::size_t(v) < V);
            if (blocked[v]) {
                protected_boundary.set(v);
            } else if (!hazardous[v]) {
                hazardous.set(v);
                queue[tail++] = v;
            }
        }
    if (hazardous[root]) return {};

    SafeOpeningResult<V> result;
    result.feasible = true;
    const auto may_open = blocked & eligible & ~protected_boundary;
    const auto remaining = blocked & ~may_open;
    head = 0;
    tail = 0;
    result.reachable.set(root);
    queue[tail++] = root;
    for (; head < tail; ++head)
        for (int v : neighbors(queue[head])) {
            if (v < 0) continue;
            assert(std::size_t(v) < V);
            if (remaining[v] || result.reachable[v]) continue;
            result.reachable.set(v);
            queue[tail++] = v;
        }
    result.opened = may_open & result.reachable;
    return result;
}

}  // namespace ahc

#endif
