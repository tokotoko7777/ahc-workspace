#include <array>
#include <cassert>
#include <vector>
#include "../library/fixed-walk-schedule.hpp"
int main() {
    // From 1, one uniform step either reaches 0 or 2. Closing 3
    // forbids an adjacent walker at 2. No rescaling: mass is 1/2.
    const std::vector<int> graph[] = {{1},{0,2},{1,3},{2}};
    const std::array kill{2}, close{3};
    const ahc::WalkSchedulePhase phases[] = {{{},{},1}, {kill,close,0}};
    ahc::WalkScheduleWorkspace work;
    const auto p = work.probability(4, [&](int i) -> const auto& { return graph[i]; },
        1, {}, phases, [](int goal) { return goal == 0; });
    assert(p == .5);
}
