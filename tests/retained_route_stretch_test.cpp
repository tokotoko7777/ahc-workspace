#include "../library/retained-route-stretch.hpp"
#include <cstdint>
#include <iostream>
#include <random>

using Graph = std::vector<std::vector<int>>;
void link(Graph& g, int a, int b) {
    g[a].push_back(b);
    g[b].push_back(a);
}
std::vector<std::vector<int>> all_dist(const Graph& g, const std::vector<int>& mask) {
    constexpr int inf = 1000000;
    const int n = int(g.size());
    std::vector d(n, std::vector<int>(n, inf));
    for (int i = 0; i < n; ++i) {
        if (mask[i]) continue;
        d[i][i] = 0;
        for (const int j : g[i]) if (!mask[j]) d[i][j] = 1;
    }
    for (int k = 0; k < n; ++k) for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
        d[i][j] = std::min(d[i][j], d[i][k] + d[k][j]);
    return d;
}
void compare(const Graph& g, const std::vector<int>& removed, int anchor) {
    const int n = int(g.size());
    std::vector<int> mask(n);
    const auto before = all_dist(g, mask);
    for (const int v : removed) mask[v] = 1;
    const auto after = all_dist(g, mask);
    int retained = 0, maximum = 0, boundary = 0;
    for (int i = 0; i < n; ++i) if (after[anchor][i] < 1000000) {
        ++retained;
        boundary += std::ranges::any_of(g[i], [&](int j) { return mask[j]; });
        for (int j = i + 1; j < n; ++j) if (after[anchor][j] < 1000000)
            maximum = std::max(maximum, after[i][j] - before[i][j]);
    }
    const auto got = retained_route::maximum_stretch(g, removed, anchor);
    assert(got.retained_size == retained && got.boundary_size == boundary && got.max_extra == maximum);
    if (maximum) {
        assert(after[anchor][got.from] < 1000000 && after[anchor][got.to] < 1000000);
        assert(before[got.from][got.to] == got.before && after[got.from][got.to] == got.after);
        assert(got.after - got.before == maximum);
    } else assert(got.from == -1 && got.to == -1);
}
int main() {
    std::mt19937 rng(234871);
    for (int rep = 0; rep < 4500; ++rep) {
        const int width = 3 + rng() % 4, height = 3 + rng() % 4;
        const int n = width * height;
        Graph g(n);
        if (rep % 2 == 0) {
            for (int i = 0; i < n; ++i) {
                if (i % width != width - 1 && rng() % 10 > 1) link(g, i, i + 1);
                if (i / width != height - 1 && rng() % 10 > 1) link(g, i, i + width);
            }
        } else {
            for (int i = 0; i < n; ++i) for (int j = i + 1; j < n; ++j)
                if (rng() % 10 < 2) link(g, i, j);
        }
        const int anchor = rng() % n;
        std::vector<int> del;
        for (int j = rep % 6; j > 0; --j) {
            const int u = rng() % n;
            if (u != anchor) del.push_back(u);
        }
        if (!del.empty() && rep % 7 == 0) del.push_back(del.front());
        compare(g, del, anchor);
    }
    Graph cycle(8);
    for (int i = 0; i < 8; ++i) link(cycle, i, (i + 1) % 8);
    const auto a = retained_route::maximum_stretch(cycle, std::vector{0}, 1);
    assert(a.max_extra == 4 && a.before == 2 && a.after == 6);
    std::cout << "4500 oracle checks and an 8-cycle passed\n";
}
