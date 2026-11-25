#include "../library/preserve-reachability.hpp"
#include <cassert>

int main() {
    // Main path 0--1--2--3--4, loose terminal 5 attached to vertex 2.
    const std::array<std::array<int, 3>, 6> graph{{
        {{1, -1, -1}}, {{0, 2, -1}}, {{1, 3, 5}},
        {{2, 4, -1}}, {{3, -1, -1}}, {{2, -1, -1}}
    }};
    const std::array cuts{1, 5}, roots{0, 4};
    std::bitset<6> terminals;
    terminals.set(5);
    const auto opt = ahc::restore_cuts_preserving_terminals(
        std::bitset<6>{}, cuts, roots, terminals,
        [&](int v) -> const auto& { return graph[v]; });
    assert(opt && opt->keep == 0b10 && opt->root_region.count() == 5);
}
