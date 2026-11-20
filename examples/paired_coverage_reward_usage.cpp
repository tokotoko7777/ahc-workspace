#include "../library/paired-coverage-reward.hpp"
#include <iostream>
int main() {
    PairedCoverageReward score({4,12});
    score.add({0,2});
    std::cout << score.delta({1,1,3}) << '\n'; // 16
}
