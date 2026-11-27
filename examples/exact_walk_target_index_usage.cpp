#include "../library/exact-walk-target-index.hpp"
#include <array>
#include <bitset>
#include <iostream>
int main() {
    std::array<std::array<int,2>,4> graph{{{{1,-1}}, {{0,2}}, {{1,3}}, {{2,-1}}}};
    std::bitset<4> walls, targets;
    targets.set(3);
    ahc::ExactWalkTargetIndex<4> index(graph, walls, targets);
    for (int steps : {2,3,4,5}) std::cout << index.can_reach(0,steps) << ' ';
    std::cout << '\n'; // 0 1 0 1
}
