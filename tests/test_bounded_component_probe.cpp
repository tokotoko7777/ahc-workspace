#include "../library/bounded-component-probe.hpp"

#include <array>
#include <cassert>
#include <bitset>
#include <queue>
#include <random>
#include <iostream>

int main() {
    constexpr int V = 45, cap = 12;
    using Mask = std::bitset<V>;
    using Status = ahc::ComponentProbe;
    std::mt19937 g(12907);
    std::size_t disjoint = 0, intersects = 0, limited = 0, invalid = 0;
    for (int it = 0; it < 11000; ++it) {
        std::array<std::array<int, 5>, V> edges;
        Mask blocked, marked, cache;
        for (int i = 0; i < V; ++i) {
            blocked[i] = g()%7 == 0;
            marked[i] = g()%(it%19+3) == 0;
            for (int& q : edges[i]) q = (g()%7 < unsigned(it%7)) ? int(g()%(V+2))-1 : -1;
        }
        auto next = [&](int v) -> const auto& { return edges[v]; };
        auto allowed = [&](int v) { return !blocked[v]; };
        auto focus = [&](int v) { return bool(marked[v]); };
        for (int k = 0; k < 30; ++k) {
            const int root = int(g()%(V+2))-1;
            const auto before = cache;
            const auto r = ahc::bounded_component_probe<V, cap>(root, next, allowed, focus, cache);
            if (root < 0 || root >= V || blocked[root]) {
                assert(r == Status::invalid_start && before == cache);
                ++invalid; continue;
            }
            Mask entire; std::queue<int> q;
            q.push(root); entire.set(root);
            while (!q.empty()) {
                const int u=q.front(); q.pop();
                for (int v : next(u)) {
                    if (v < 0 || v >= V || blocked[v] || entire[v]) continue;
                    entire.set(v); q.push(v);
                }
            }
            const bool has_mark = (entire&marked).any();
            if (r == Status::disjoint) {
                assert(!has_mark && cache == (before|entire));
                ++disjoint;
            } else if (r == Status::intersects) {
                assert(has_mark && cache == before);
                ++intersects;
            } else {
                assert(r == Status::over_limit && entire.count() > cap && cache == before);
                ++limited;
            }
            if (entire.count() <= cap) assert(r == (has_mark ? Status::intersects : Status::disjoint));
        }
    }
    assert(disjoint && intersects && limited && invalid);
    std::cout << "330000 queries passed; known clear " << disjoint
              << ", intersects " << intersects << ", over budget " << limited
              << ", invalid " << invalid << '\n';
}
