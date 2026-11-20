#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include "../../library/dsu.hpp"
#include "../../library/kruskal-bottleneck.hpp"

constexpr int N = 400, M = 1995;
struct Edge { int from, to, cost, id; };

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::array<int, N> x{}, y{};
    for (int v = 0; v < N; ++v) if (!(std::cin >> x[v] >> y[v])) return 1;
    std::vector<Edge> edges;
    for (int i = 0; i < M; ++i) {
        int u, v;
        if (!(std::cin >> u >> v)) return 1;
        const int dx = x[u] - x[v], dy = y[u] - y[v];
        edges.push_back({u, v, static_cast<int>(std::lround(std::sqrt(dx*dx+dy*dy))), i});
    }
#ifndef AHC_MEAN_WEIGHTS
    constexpr int scenarios = 24;
#else
    constexpr int scenarios = 1;
#endif
    std::array<std::vector<Edge>, scenarios> orders;
#ifndef AHC_MEAN_WEIGHTS
    std::mt19937 rng(271907);
#endif
    for (auto& order : orders) {
        order = edges;
        for (auto& e : order) {
#ifndef AHC_MEAN_WEIGHTS
            e.cost = std::uniform_int_distribution<int>(e.cost, 3*e.cost)(rng);
#else
            e.cost *= 2;
#endif
        }
        std::sort(order.begin(), order.end(), [](const Edge& a, const Edge& b) {
            if (a.cost != b.cost) return a.cost < b.cost;
            return a.id < b.id;
        });
    }
    Dsu tree(N), scratch(N);
    for (int i = 0; i < M; ++i) {
        int length;
        if (!(std::cin >> length)) return 1;
        const auto& e = edges[i];
        bool take = false;
        if (!tree.same(e.from, e.to)) {
            // Connectivity without e is independent of scenario costs.
            int total_cutoff = 0;
            for (const auto& order : orders) {
                scratch = tree;
                auto cutoff = kruskal_path_bottleneck(scratch, order, e.from, e.to,
                    [i](const Edge& other) { return other.id > i; });
                if (!cutoff) { take = true; break; }
                total_cutoff += *cutoff;
            }
            take = take || scenarios * length <= total_cutoff;
            if (take) tree.unite(e.from, e.to);
        }
        std::cout << take << '\n' << std::flush;
    }
    return tree.component_count() == 1 ? 0 : 1;
}
