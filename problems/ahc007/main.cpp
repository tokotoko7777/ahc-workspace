#include <algorithm>
#include <array>
#include <cmath>
#ifdef AHC_PAIRED_ROLLOUT
#include <chrono>
#endif
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include "../../library/dsu.hpp"
#include "../../library/kruskal-bottleneck.hpp"

constexpr int N = 400, M = 1995;
struct Edge { int from, to, cost, id; };

#ifdef AHC_PAIRED_ROLLOUT
// Reveal only the current sampled weight to the rollout policy. After a
// short prefix, score the remainder with exact sampled MST completion.
int play_then_complete(Dsu dsu, int first, int end,
                       const std::vector<Edge>& edges, const std::vector<Edge>& mean_order,
                       const std::array<int, M>& weights, const std::vector<Edge>& sampled_order) {
    Dsu scratch(N);
    int total = 0;
    for (int j = first; j < end; ++j) {
        const auto& e = edges[j];
        if (dsu.same(e.from, e.to)) continue;
        scratch = dsu;
        const auto cutoff = kruskal_path_bottleneck(scratch, mean_order, e.from, e.to,
            [j](const Edge& other) { return other.id > j; });
        if (!cutoff || weights[j] <= *cutoff) {
            dsu.unite(e.from, e.to);
            total += weights[j];
        }
    }
    for (const auto& e : sampled_order) {
        if (dsu.component_count() == 1) break;
        if (e.id >= end && dsu.unite(e.from, e.to)) total += e.cost;
    }
    if (dsu.component_count() != 1) return -1;
    return total;
}
#endif

int main() {
#ifdef AHC_PAIRED_ROLLOUT
    const auto started = std::chrono::steady_clock::now();
    // Stop added search early and leave margin for the cheap valid fallback.
    const auto rollout_deadline = started + std::chrono::milliseconds(1350);
    const auto scenario_deadline = started + std::chrono::milliseconds(1550);
    int rollouts = 0, changes = 0;
#endif
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
#ifdef AHC_PAIRED_ROLLOUT
    auto mean_order = edges;
    for (auto& e : mean_order) e.cost *= 2;
    std::sort(mean_order.begin(), mean_order.end(), [](const auto& a, const auto& b) {
        return a.cost != b.cost ? a.cost < b.cost : a.id < b.id;
    });
    std::array<std::array<int, M>, scenarios> weights;
    for (int k = 0; k < scenarios; ++k)
        for (const auto& e : orders[k]) weights[k][e.id] = e.cost;
#endif
    Dsu tree(N), scratch(N);
    for (int i = 0; i < M; ++i) {
        int length;
        if (!(std::cin >> length)) return 1;
        const auto& e = edges[i];
        bool take = false;
        if (!tree.same(e.from, e.to)) {
            // Connectivity without e is independent of scenario costs.
            int total_cutoff = 0, count = 0;
#ifdef AHC_PAIRED_ROLLOUT
            std::array<int, scenarios> cutoffs{};
            if (std::chrono::steady_clock::now() >= scenario_deadline) {
                scratch = tree;
                const auto cutoff = kruskal_path_bottleneck(scratch, mean_order, e.from, e.to,
                    [i](const Edge& other) { return other.id > i; });
                take = !cutoff || length <= *cutoff;
            } else {
#endif
            for (const auto& order : orders) {
                scratch = tree;
                auto cutoff = kruskal_path_bottleneck(scratch, order, e.from, e.to,
                    [i](const Edge& other) { return other.id > i; });
                if (!cutoff) { take = true; break; }
#ifdef AHC_PAIRED_ROLLOUT
                cutoffs[count] = *cutoff;
#endif
                ++count;
                total_cutoff += *cutoff;
            }
            take = take || scenarios * length <= total_cutoff;
#ifdef AHC_PAIRED_ROLLOUT
            // Only resolve near ties. Compute the excess online cost gap
            // relative to the exact per-scenario offline cut, then add this
            // paired control variate to the estimate from all scenarios.
            if (count == scenarios && rollouts < 160 && i + 5 < M &&
                12 * std::abs(scenarios * length - total_cutoff) <= total_cutoff &&
                std::chrono::steady_clock::now() < rollout_deadline) {
                ++rollouts;
                Dsu accepted = tree;
                accepted.unite(e.from, e.to);
                constexpr int paired = 6, horizon = 16;
                int correction = 0, sampled = 0;
                const int end = std::min(M, i + 1 + horizon);
                for (int s = 0; s < paired; ++s) {
                    if (std::chrono::steady_clock::now() >= rollout_deadline) break;
                    const int k = (i + s * 4) % scenarios;
                    int yes = play_then_complete(accepted, i+1, end, edges, mean_order, weights[k], orders[k]);
                    int no = play_then_complete(tree, i+1, end, edges, mean_order, weights[k], orders[k]);
                    if (yes < 0 || no < 0) break;
                    correction += yes - no + cutoffs[k];
                    ++sampled;
                }
                if (sampled == paired) {
                    const bool revised =
                        paired * (scenarios * length - total_cutoff) + scenarios * correction <= 0;
                    changes += take != revised;
                    take = revised;
                }
            }
            }  // scenario work pays attention to the wall clock
#endif
            if (take) tree.unite(e.from, e.to);
        }
        std::cout << take << '\n' << std::flush;
    }
#ifdef AHC_PAIRED_ROLLOUT
    std::cerr << "paired_probes=" << rollouts << " revised_actions=" << changes << '\n';
#endif
    return tree.component_count() == 1 ? 0 : 1;
}
