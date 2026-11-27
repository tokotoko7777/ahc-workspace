#pragma once

#include <array>
#include <bitset>
#include <cstddef>

namespace ahc {

enum class ComponentProbe {
    disjoint,       // The entire reachable set avoids every marked vertex.
    intersects,     // A reachable marked vertex was found.
    over_limit,     // Inconclusive without a larger search.
    invalid_start,  // Start is outside the graph or not traversable.
};

// Examine the out-reachable set with at most Limit discovered vertices.
// next(v) yields integral neighbor IDs; negative or out-of-range IDs are ignored.
// allowed(v) and marked(v) apply to a single immutable snapshot.
// The cache must be cleared when the graph, allowed, or marked rule changes.
// Only fully verified disjoint searches add their complete visited sets to it.
template <std::size_t Vertices, std::size_t Limit,
          class Neighbors, class Allowed, class Marked>
ComponentProbe bounded_component_probe(
    int start, Neighbors&& next, Allowed&& allowed, Marked&& marked,
    std::bitset<Vertices>& known_disjoint)
{
    static_assert(Vertices > 0 && Limit > 0 && Limit <= Vertices);
    if (start < 0 || static_cast<std::size_t>(start) >= Vertices || !allowed(start))
        return ComponentProbe::invalid_start;
    if (known_disjoint[start]) return ComponentProbe::disjoint;
    if (marked(start)) return ComponentProbe::intersects;

    std::bitset<Vertices> seen;
    std::array<int, Limit> queue;
    std::size_t count = 1;
    queue[0] = start;
    seen.set(start);
    for (std::size_t head = 0; head != count; ++head) {
        for (auto v : next(queue[head])) {
            if (v < 0 || static_cast<std::size_t>(v) >= Vertices ||
                seen[v] || !allowed(v))
                continue;
            if (marked(v)) return ComponentProbe::intersects;
            if (count == Limit) return ComponentProbe::over_limit;
            seen.set(v);
            queue[count++] = static_cast<int>(v);
        }
    }
    known_disjoint |= seen;
    return ComponentProbe::disjoint;
}

}  // namespace ahc
