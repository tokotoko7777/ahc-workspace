#include "../library/exact-walk-target-index.hpp"
#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
int main() {
    constexpr int N = 13;
    std::mt19937 rng(573581);
    int checked = 0;
    for (int k = 0; k < 4800; ++k) {
        std::array<std::vector<int>,N> graph;
        std::bitset<N> walls, targets;
        for (int u = 0; u < N; ++u) {
            walls[u] = rng() % 5 == 0;
            targets[u] = rng() % 7 == 0;
            for (int v = u; v < N; ++v) if (rng() % 40 < unsigned(1+k%17)) {
                graph[u].push_back(v);
                if (u != v) graph[v].push_back(u);
            }
            graph[u].push_back(-1);
        }
        for (bool stay : {false,true}) {
            ahc::ExactWalkTargetIndex<N> index(graph,walls,targets,stay);
            for (int s = 0; s < N; ++s) {
                std::bitset<N> cur;
                if (!walls[s]) cur.set(s);
                for (int steps = 0; steps <= 24; ++steps) {
                    assert(index.can_reach(s,steps) == (cur & targets).any());
                    ++checked;
                    std::bitset<N> next;
                    for (int v = 0; v < N; ++v) if (cur[v]) {
                        bool moved = false;
                        for (int u: graph[v]) if (u >= 0 && !walls[u]) {
                            next.set(u);
                            moved = true;
                        }
                        if (!moved && stay) next.set(v);
                    }
                    cur = next;
                }
            }
        }
    }
    std::cout << checked << " DP comparisons\n";
}
