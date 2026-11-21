#pragma once
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>

// path_bottleneck returns the minimum possible maximum edge cost on a
// path between different components. The supplied DSU is mutated and
// represents pre-contracted components; edges must be sorted by cost.
// Excluded edges are skipped (e.g. only future edges remain in an online
// graph). Stop just when source and target are connected.
template<class Dsu, std::ranges::input_range Range, class Keep>
auto kruskal_path_bottleneck(Dsu& dsu, Range&& edges, int source, int target, Keep keep)
    -> std::optional<std::remove_cvref_t<decltype((*std::ranges::begin(edges)).cost)>> {
    for (const auto& edge : edges) {
        if (!keep(edge)) continue;
        if (dsu.unite(edge.from, edge.to) && dsu.same(source, target)) return edge.cost;
    }
    return std::nullopt;
}
