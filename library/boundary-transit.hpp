#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace ahc {

// Summarize an interior patch of an unweighted, undirected graph.
// Vertex IDs are [0,N); other neighbors are ignored. Pass the same
// fixed adjacency to construction and all subsequent distance queries.
// A port is an exterior neighbor of the patch. Paths in the signature
// may use only interior vertices between the two endpoint ports.
// The live/blocked state of ports is deliberately ignored.
template <std::size_t N>
class BoundaryTransit {
    static_assert(N > 0 && N < std::size_t(std::numeric_limits<int>::max()) - 1);
public:
    using Mask = std::bitset<N>;
    static constexpr int unreachable = std::numeric_limits<int>::max();

    template <class Neighbors>
    BoundaryTransit(Mask interior, const Neighbors& neighbors) : interior_(interior) {
        Mask seen;
        for (std::size_t p = 0; p < N; ++p) if (interior_[p])
            for (int u : neighbors(static_cast<int>(p)))
                if (valid(u) && !interior_[u]) seen.set(u);
        for (std::size_t p = 0; p < N; ++p)
            if (seen[p]) ports_.push_back(static_cast<int>(p));
    }

    [[nodiscard]] const std::vector<int>& ports() const { return ports_; }

    // Triangular output: (port i, port j) with j < i at i*(i-1)/2+j.
    // Keep direct exterior-exterior edges in the exterior graph itself.
    template <class Neighbors>
    [[nodiscard]] std::vector<int> signature(const Mask& blocked,
                                             const Neighbors& neighbors) const {
        const std::size_t p = ports_.size();
        std::vector<int> result;
        if (p < 2) return result;
        result.reserve(p * (p - 1) / 2);
        const Mask allowed = interior_ & ~blocked;
        std::array<int, N> queue, distance;
        for (std::size_t i = 1; i < p; ++i) {
            Mask seen;
            std::size_t end = 0;
            for (int v : neighbors(ports_[i]))
                if (valid(v) && allowed[v] && !seen[v]) {
                    seen.set(v);
                    distance[v] = 1;
                    queue[end++] = v;
                }
            for (std::size_t head = 0; head < end; ++head) {
                const int v = queue[head];
                for (int u : neighbors(v))
                    if (valid(u) && allowed[u] && !seen[u]) {
                        seen.set(u);
                        distance[u] = distance[v] + 1;
                        queue[end++] = u;
                    }
            }
            for (std::size_t j = 0; j < i; ++j) {
                int best = unreachable;
                for (int v : neighbors(ports_[j])) if (valid(v) && seen[v]) {
                    const int candidate = distance[v] + 1;
                    if (candidate < best) best = candidate;
                }
                result.push_back(best);
            }
        }
        return result;
    }

private:
    static bool valid(int v) { return v >= 0 && static_cast<std::size_t>(v) < N; }
    Mask interior_;
    std::vector<int> ports_;
};
} // namespace ahc
