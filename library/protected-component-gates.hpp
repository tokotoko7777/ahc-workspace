#pragma once
#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace ahc {
// Returns the blocked optional vertices that must remain blocked so that
// NONE of the components containing a seed increases in size.
// Graph is undirected. Neighbor slots -1 mean a missing edge. All seeds
// must be originally open, and openable must be a subset of blocked.
template<std::size_t N, class Neighbors>
std::bitset<N> required_component_gates(const Neighbors& neighbors,
                                       const std::bitset<N>& blocked,
                                       const std::bitset<N>& openable,
                                       const std::bitset<N>& seeds) {
    assert((openable & ~blocked).none() && (seeds & blocked).none());
    std::bitset<N> need, seen = seeds;
    using Index = std::conditional_t<
        N <= std::numeric_limits<std::uint16_t>::max() + std::size_t{1},
        std::uint16_t, std::size_t>;
    std::array<Index,N> queue;
    std::size_t length = 0;
    for(std::size_t i = 0; i < N; ++i)
        if(seeds[i]) queue[length++] = static_cast<Index>(i);
    for(std::size_t i = 0; i < length; ++i){
        for(auto v : neighbors[queue[i]]) {
            if(v < 0) continue;
            const auto u = static_cast<std::size_t>(v);
            assert(u < N);
            if(blocked[u]) {
                if(openable[u]) need.set(u);
            } else if(!seen[u]) {
                seen.set(u);
                queue[length++] = static_cast<Index>(u);
            }
        }
    }
    return need;
}
}
