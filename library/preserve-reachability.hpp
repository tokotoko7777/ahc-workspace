#pragma once
#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>

namespace ahc {
template<std::size_t N>
struct ReachabilityCut {
    std::uint32_t keep = 0;
    std::bitset<N> root_region;
    std::size_t visited_subsets = 0;
};

// Restore some proposed vertex cuts while keeping every root connected and
// preserving exactly the terminals reachable from ANY root with all cuts.
// The induced single root component is maximized. Ties prefer the smaller
// kept-bit mask. Neighbor ranges must describe the same undirected graph.
template<std::size_t N, class Neighbors>
std::optional<ReachabilityCut<N>> restore_cuts_preserving_terminals(
    const std::bitset<N>& blocked, std::span<const int> proposed,
    std::span<const int> roots, const std::bitset<N>& terminals,
    Neighbors&& neighbors) {
    using Mask = std::bitset<N>;
    if (roots.empty() || proposed.size() > 20)
        throw std::invalid_argument("nonempty roots and at most 20 proposed cuts required");
    Mask union_cuts;
    for (int p : proposed) {
        if (p < 0 || std::size_t(p) >= N || blocked[p] || union_cuts[p])
            throw std::invalid_argument("proposed cuts must be distinct open vertices");
        union_cuts.set(p);
    }
    for (int p : roots)
        if (p < 0 || std::size_t(p) >= N || blocked[p] || union_cuts[p])
            throw std::invalid_argument("roots must remain open");
    const std::uint32_t all = (1u << proposed.size()) - 1;
    Mask unavailable = blocked | union_cuts;
    std::array<int, N> queue{};
    auto flood = [&](Mask& seen, int start) {
        if (seen[start]) return;
        std::size_t length = 1;
        queue[0] = start;
        seen.set(start);
        for (std::size_t i = 0; i < length; ++i)
            for (int p : neighbors(queue[i]))
                if (p >= 0 && std::size_t(p) < N && !unavailable[p] && !seen[p]) {
                    seen.set(p);
                    queue[length++] = p;
                }
    };
    Mask union_with_all_cuts;
    for (int p : roots) flood(union_with_all_cuts, p);
    const Mask forbidden = terminals & ~union_with_all_cuts;
    std::optional<ReachabilityCut<N>> best;
    std::size_t visited = 0, best_area = 0;
    auto search = [&](auto&& self, unsigned from, std::uint32_t kept) -> void {
        ++visited;
        Mask seen;
        flood(seen, roots.front());
        const bool joined = std::all_of(roots.begin(), roots.end(), [&](int p) { return seen[p]; });
        if (!joined) for (int p : roots) flood(seen, p);
        // Opening additional vertices never loses reachability, so a
        // forbidden terminal here rules out every descendant.
        if ((seen & forbidden).any()) return;
        if (joined) {
            const auto area = seen.count();
            if (!best || area > best_area || (area == best_area && kept < best->keep)) {
                best = ReachabilityCut<N>{kept, seen, 0};
                best_area = area;
            }
        }
        for (unsigned i = from; i < proposed.size(); ++i) {
            unavailable.reset(proposed[i]);
            self(self, i + 1, kept ^ (1u << i));
            unavailable.set(proposed[i]);
        }
    };
    search(search, 0, all);
    if (best) best->visited_subsets = visited;
    return best;
}
}
