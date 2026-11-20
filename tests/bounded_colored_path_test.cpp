#include "../library/bounded-colored-path.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

int main() {
    std::mt19937 rng(41917);
    for (int trial = 0; trial < 1500; ++trial) {
        constexpr int n = 7, colors = 6;
        std::vector<std::vector<int>> adj(n);
        for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
            if (u != v && rng()%3 == 0) adj[u].push_back(v);
        std::vector<int> tile(n), point(n), blocked_color(colors);
        tile[0] = 0; tile[n-1] = 1;
        for (int v = 1; v+1 < n; ++v) { tile[v] = rng()%colors; point[v] = rng()%17; }
        for (int t = 2; t < colors; ++t) blocked_color[t] = rng()%5 == 0;
        const int max_edges = 1 + rng()%7;
        const auto neighbors = [&](int v)->const std::vector<int>& {return adj[v];};
        const auto color = [&](int v){return tile[v];};
        const auto value = [&](int v){return point[v];};
        const auto blocked = [&](int t){return bool(blocked_color[t]);};
        std::vector<int> actual, prefix;
        std::vector<unsigned char> state(colors);
        const int got = bounded_colored_path::maximize(0, n-1, max_edges, colors,
            neighbors, color, value, blocked, [](int){return 0;}, actual, prefix, state);
        // Reference enumerates uncolored walks, then independently validates.
        // No visited pruning is used to construct the reference.
        int expect = -1;
        std::vector<int> walk{0};
        auto check = [&](const std::vector<int>& route) {
            assert(route.front() == 0 && route.back() == n-1);
            if (int(route.size())-1 > max_edges) return -1;
            std::vector<int> occ(colors);
            int subtotal = 0;
            for (int i = 0; i < int(route.size()); ++i) {
                const int v = route[i], c = tile[v];
                if (occ[c]++ || blocked_color[c]) return -1;
                if (i && std::find(adj[route[i-1]].begin(),adj[route[i-1]].end(),v)
                         == adj[route[i-1]].end()) return -1;
                if (i > 0 && i+1 < int(route.size())) subtotal += point[v];
            }
            return subtotal;
        };
        auto enumerate = [&](auto&& self, int v, int left) -> void {
            if (v == n-1) { expect = std::max(expect, check(walk)); return; }
            if (left == 0) return;
            for (int u : adj[v]) {
                walk.push_back(u); self(self,u,left-1); walk.pop_back();
            }
        };
        enumerate(enumerate,0,max_edges);
        assert(got == expect);
        if (got >= 0) {
            std::vector<int> route{0};
            route.insert(route.end(),actual.begin(),actual.end());
            route.push_back(n-1);
            assert(check(route) == got);
        }
        assert(std::none_of(state.begin(),state.end(),[](int x){return x != 0;}));
    }
}
