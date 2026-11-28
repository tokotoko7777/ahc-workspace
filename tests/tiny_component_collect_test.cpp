#pragma GCC optimize("O3")
#include "../library/tiny-component-collect.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>
using ahc::TinyComponentStatus;
int main() {
    constexpr std::size_t V = 19;
    std::mt19937 rng(91677);
    long long cases = 0;
    for (int it = 0; it < 8000; ++it) {
        std::array<std::vector<int>, V> g;
        std::array<bool, V> open;
        for (std::size_t z = 0; z < V; ++z) {
            open[z] = rng() % 3 != 0;
            int deg = rng() % 8;
            for (int j = 0; j < deg; ++j) g[z].push_back(int(rng() % (V + 3)) - 1);
        }
        auto nbs = [&](int p) -> const auto& { return g[p]; };
        auto free = [&](int p) { return open[p]; };
        for (int p = -1; p <= int(V); ++p) {
            std::vector<int> ref;
            if (p >= 0 && p < int(V) && free(p)) {
                ref.push_back(p);
                std::array<bool, V> seen{};
                seen[p] = true;
                for (std::size_t i = 0; i < ref.size(); ++i)
                    for (int q : g[ref[i]])
                        if (q >= 0 && q < int(V) && free(q) && !seen[q]) {
                            seen[q] = true;
                            ref.push_back(q);
                        }
            }
            auto verify = [&]<std::size_t Limit>() {
                auto result = ahc::collect_tiny_component<V, Limit>(p, nbs, free);
                auto set = result.complete_vertices();
                if (ref.empty()) {
                    assert(result.status == TinyComponentStatus::invalid_start && set.empty());
                } else if (ref.size() > Limit) {
                    assert(result.status == TinyComponentStatus::over_limit && set.empty());
                } else {
                    assert(result.status == TinyComponentStatus::complete);
                    assert(std::equal(set.begin(), set.end(), ref.begin(), ref.end()));
                }
                ++cases;
            };
            verify.template operator()<1>();
            verify.template operator()<4>();
            verify.template operator()<8>();
            verify.template operator()<V>();
        }
    }
    std::cout << cases << " differential checks\n";
}
