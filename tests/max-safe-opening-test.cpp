#include "../library/max-safe-opening.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>

int main() {
    constexpr int n = 12, width = 4;
    using Mask = std::bitset<n>;
    const auto neighbors = [](int u) {
        return std::array{
            u >= width ? u - width : -1,
            u + width < n ? u + width : -1,
            u % width ? u - 1 : -1,
            u % width + 1 < width ? u + 1 : -1,
        };
    };
    const auto flood = [&](int root, const Mask& wall) {
        Mask seen;
        std::array<int, n> q;
        seen.set(root);
        q[0] = root;
        for (int first = 0, end = 1; first < end; ++first)
            for (int v : neighbors(q[first]))
                if (v >= 0 && !wall[v] && !seen[v]) {
                    seen.set(v);
                    q[end++] = v;
                }
        return seen;
    };
    std::mt19937 gen(748319);
    int feasible = 0, infeasible = 0;
    std::uint64_t checked = 0;
    for (int it = 0; it < 12000; ++it) {
        int root = static_cast<int>(gen() % n);
        Mask wall(gen()); wall.reset(root);
        Mask eligible(gen());
        Mask hazards = Mask(gen() & gen()) & ~wall;
        if (it % 4 == 0) hazards.reset();
        const auto got = ahc::max_safe_opening(root, wall, eligible, hazards, neighbors);
        const unsigned choices = (eligible & wall).to_ulong();
        Mask expected;
        for (unsigned sub = choices;; sub = (sub - 1) & choices) {
            const Mask reached = flood(root, wall & ~Mask(sub));
            if ((reached & hazards).none()) expected |= reached;
            ++checked;
            if (!sub) break;
        }
        const bool valid = expected.any();
        assert(got.feasible == valid);
        if (!valid) { ++infeasible; continue; }
        ++feasible;
        assert((got.opened & ~(wall & eligible)).none());
        assert(got.reachable == expected);
        assert(flood(root, wall & ~got.opened) == expected);
        assert((got.reachable & hazards).none());
    }
    assert(feasible > 0 && infeasible > 0);
    std::cout << feasible << " feasible and " << infeasible << " infeasible; "
              << checked << " subsets checked\n";
}
