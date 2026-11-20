#include "../library/dsu.hpp"
#include "../library/kruskal-bottleneck.hpp"
#include <array>
#include <iostream>
struct Edge { int from, to, cost; };
int main() {
    Dsu forest(4);
    forest.unite(0,1);
    const std::array<Edge,3> ordered{{{1,2,2},{2,3,5},{0,3,9}}};
    auto cut = kruskal_path_bottleneck(forest, ordered, 0, 3, [](const Edge&) { return true; });
    if (!cut || *cut != 5) return 1;
    std::cout << *cut << '\n';
}
