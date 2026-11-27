#include "library/boundary-transit.hpp"
#include <array>
#include <iostream>

int main() {
    // Exterior endpoints 0 and 4. Vertex 2 is a side branch, not a shortcut.
    constexpr std::array graph{
        std::array{1,-1,-1}, std::array{0,2,3}, std::array{1,-1,-1},
        std::array{1,4,-1}, std::array{3,-1,-1}};
    auto next=[&](int p){return graph[p];};
    using Transit=ahc::BoundaryTransit<graph.size()>;
    const Transit patch(Transit::Mask{}.set(1).set(2).set(3),next);
    const auto before=patch.signature({},next);
    const auto after=patch.signature(Transit::Mask{}.set(2),next);
    std::cout << (before==after ? "Outside distances preserved\n" : "Routes changed\n");
}
