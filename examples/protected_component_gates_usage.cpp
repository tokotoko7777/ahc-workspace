#include "../library/protected-component-gates.hpp"
#include <iostream>

int main(){
    // Line 0-1-2-3-4-5, occupied pen at 0, human at 4.
    // Keep gate 1 to hold the pen; open gate 3.
    constexpr std::array<std::array<int,2>,6> neighbor{{
        {-1,1},{0,2},{1,3},{2,4},{3,5},{4,-1}
    }};
    const std::bitset<6> closed{0b001010}, seeds{1};
    const auto need=ahc::required_component_gates(neighbor,closed,closed,seeds);
    std::cout << (closed & ~need) << '\n'; // 001000
}
