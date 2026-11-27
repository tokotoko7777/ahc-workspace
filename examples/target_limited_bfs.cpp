#include <array>
#include <bitset>
#include <cassert>
#include "../library/target-limited-bfs.hpp"

int main() {
    // Directed graph. Only distances to 2 and 4 serve subsequent logic.
    constexpr std::array<std::array<int, 2>, 5> graph{{
        {1, 3}, {2, -1}, {-1, -1}, {4, -1}, {-1, -1},
    }};
    std::bitset<5> targets;
    targets.set(2).set(4);
    std::array<int, 5> d;
    ahc::TargetBfsStats stat;
    ahc::target_limited_bfs(0, 5, 99, targets, d, [&](int i) -> const auto& {
        return graph[i];
    }, [](int i) { return i != 3; }, &stat);
    assert(d[2] == 2 && d[4] == 99 && !stat.all_targets_found);
}
