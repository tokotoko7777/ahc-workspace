#include <array>
#include <bitset>
#include <cassert>
#include "../library/joint-separator-step.hpp"

int main() {
    // 0--1--2--3, worker at 0, excluded terminal at 3.
    // The worker can wall cell 1, leaving a 1-cell clean component.
    auto corridor = [](int p) {
        return std::array{p == 0 ? -1 : p - 1, p == 3 ? -1 : p + 1, -1, -1};
    };
    const std::bitset<4> walls, legal(0b0010), excluded(0b1000);
    const std::array workers{0};
    const auto best = ahc::joint_separator_step(walls, legal, excluded, workers, corridor);
    assert(best.area == 1 && best.commands == "b");
}
