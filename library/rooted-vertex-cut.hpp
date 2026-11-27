#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>

namespace ahc {

struct RootedCutMass {
    std::size_t size = 0;
    std::uint64_t labels = 0;
    bool operator==(const RootedCutMass&) const = default;
};

// For each possible vertex deletion, summarize the component that still
// contains root in a fixed simple undirected graph. Inactive vertices
// are not traversable. Out-of-range neighbors are ignored.
// A label bit may occur at only one active vertex (several different
// labels may share a vertex). Use one build for many deletion queries.
template <std::size_t N>
class RootedVertexCut {
    static_assert(N > 0 && N <= std::size_t(std::numeric_limits<int>::max()));
public:
    template <class Neighbors, class Open>
    bool build(int root, const Neighbors& next, const Open& open,
               const std::array<std::uint64_t, N>& labels) {
        root_ = valid(root) && open(root) ? root : -1;
        time_.fill(-1);
        whole_ = {};
        if (root_ < 0) return false;

        std::array<int, N> low, stack;
        std::array<std::size_t, N> cursor;
        std::array<RootedCutMass, N> subtree;
        std::size_t active = 0;
        int seen = 0;
        auto push = [&](int v) {
            stack[active++] = v;
            time_[v] = low[v] = seen++;
            cursor[v] = 0;
            detached_[v] = subtree[v] = RootedCutMass{1, labels[v]};
        };
        push(root_);
        while (active) {
            const int v = stack[active - 1];
            const auto& edges = next(v);
            if (cursor[v] < std::size(edges)) {
                const int u = edges[cursor[v]++];
                if (!valid(u) || !open(u) ||
                    (active >= 2 && u == stack[active - 2])) continue;
                if (time_[u] >= 0) low[v] = std::min(low[v], time_[u]);
                else push(u);
                continue;
            }
            --active;
            if (!active) {
                whole_ = subtree[v];
                break;
            }
            const int p = stack[active - 1];
            add(subtree[p], subtree[v]);
            if (low[v] >= time_[p]) add(detached_[p], subtree[v]);
            low[p] = std::min(low[p], low[v]);
        }
        return true;
    }

    [[nodiscard]] RootedCutMass whole() const { return whole_; }

    // A vertex outside the rooted component leaves the result unchanged.
    // Invalid ID, invalid build, or deleting the root returns nullopt.
    [[nodiscard]] std::optional<RootedCutMass> without(int v) const {
        if (!valid(v) || root_ < 0 || v == root_) return std::nullopt;
        if (time_[v] < 0) return whole_;
        return RootedCutMass{whole_.size - detached_[v].size,
                             whole_.labels & ~detached_[v].labels};
    }

private:
    static bool valid(int v) { return v >= 0 && static_cast<std::size_t>(v) < N; }
    static void add(RootedCutMass& a, RootedCutMass b) {
        a.size += b.size;
        a.labels |= b.labels;
    }
    int root_ = -1;
    RootedCutMass whole_;
    std::array<int, N> time_;
    std::array<RootedCutMass, N> detached_;
};
} // namespace ahc
