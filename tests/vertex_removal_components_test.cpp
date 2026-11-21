#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <vector>
#include "../library/vertex-removal-components.hpp"

int main() {
    std::mt19937 rng(20261122);
    constexpr int N = 28;
    VertexRemovalComponents<int> cut(N);
    for (int test = 0; test < 4000; ++test) {
        std::array<std::vector<int>, N> g;
        std::array<int, N> weight{}, active{};
        for (int i = 0; i < N; ++i) {
            active[i] = rng() % 5 != 0;
            weight[i] = int(rng() % 21) - 10;
            for (int j = 0; j < i; ++j) if (rng() % (2 + test % 25) == 0) {
                g[i].push_back(j);
                g[j].push_back(i);
            }
        }
        cut.build([&](int v) -> const auto& { return g[v]; },
                  [&](int v) { return active[v]; }, [&](int v) { return weight[v]; });
        for (int v = 0; v < N; ++v) if (active[v]) {
            std::array<int, N> orig{}, seen{}, labels;
            labels.fill(-1);
            auto bfs = [&](int u, int removed, auto& flag) {
                std::vector<int> q{u};
                flag[u] = 1;
                for (std::size_t k = 0; k < q.size(); ++k) for (int y : g[q[k]])
                    if (active[y] && y != removed && !flag[y]) flag[y] = 1, q.push_back(y);
                return q;
            };
            bfs(v, -1, orig);
            std::vector<std::array<int, 3>> slow, fast;
            for (int u = 0; u < N; ++u) if (u != v && orig[u] && !seen[u]) {
                const int label = cut.side_of(v, u);
                assert(label >= 0 && labels[label] == -1);
                labels[label] = u;
                int sum = 0;
                const auto q = bfs(u, v, seen);
                for (int y : q) {
                    assert(cut.side_of(v, y) == label);
                    sum += weight[y];
                }
                slow.push_back({label, int(q.size()), sum});
            }
            for (int u = 0; u < N; ++u) if (!orig[u] || u == v) assert(cut.side_of(v, u) == -1);
            cut.after_removing(v, [&](int label, int sz, int sum) { fast.push_back({label, sz, sum}); });
            std::sort(slow.begin(), slow.end());
            std::sort(fast.begin(), fast.end());
            assert(slow == fast && cut.is_cut(v) == (slow.size() > 1));
        }
    }
}
