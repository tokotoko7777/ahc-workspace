#pragma once

#include <concepts>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bounded_residual_cut {

template<std::integral Capacity>
struct Result {
    Capacity extra;
    std::vector<char> smallest_source;
    std::vector<char> largest_source;
};

// The supplied network must be a residual graph of a completed s-t max flow.
// An implication (u,v) requires that if u is in S, v is also in S.
// The cost is measured above the original optimum, before implications.
// Returns nullopt when no constrained cut is within budget.
// Network (e.g. MaxFlow<Capacity>) has public adjacency and Dinic methods.
template<class Network, std::integral Capacity>
std::optional<Result<Capacity>> query(
    const Network& residual, int s, int t, Capacity budget,
    std::span<const std::pair<int,int>> implications,
    bool need_largest = false) {
    if (budget < 0 || budget == std::numeric_limits<Capacity>::max() ||
        s < 0 || t < 0 || s >= residual.n || t >= residual.n || s == t)
        throw std::invalid_argument("bounded residual cut: invalid budget or terminals");
    const auto limit = static_cast<Capacity>(budget + 1);
    auto net = residual;
    for (auto [u,v] : implications) {
        if (u < 0 || v < 0 || u >= net.n || v >= net.n)
            throw std::invalid_argument("bounded residual cut: invalid implication");
        if (u != v) net.add_edge(u,v,limit);
    }
    const Capacity extra = net.flow(s,t,limit);
    if (extra == limit) return std::nullopt;
    auto small = net.min_cut(s);
    Result<Capacity> r{extra,std::move(small),{}};
    if (!need_largest) return r;
    std::vector<std::vector<int>> reverse(net.n);
    for (int u=0;u<net.n;++u)
        for (const auto& e : net.graph[u]) if (e.capacity > 0) reverse[e.to].push_back(u);
    r.largest_source.assign(net.n,1);
    std::vector<int> q{t};
    r.largest_source[t]=0;
    for (std::size_t i=0;i<q.size();++i)
        for (int u:reverse[q[i]]) if (r.largest_source[u]) {
            r.largest_source[u]=0;
            q.push_back(u);
        }
    return r;
}
}  // namespace bounded_residual_cut
