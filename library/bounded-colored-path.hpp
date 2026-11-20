#pragma once
#include <vector>

namespace bounded_colored_path {
// Maximize the sum of values of interior vertices of a start-to-goal walk
// of at most max_edges edges. Each color may occur at most once. The caller
// marks colors unavailable outside the rebuilt segment via blocked(). The
// start and goal colors are always reserved. lower_bound(v) must not
// overestimate the remaining edge distance to the goal. Requires dense,
// nonnegative colors in [0, color_count), and distinct endpoint colors.
// Return -1 when no route exists; assumes nonnegative values. The output
// contains only interior vertices. Supply empty state and reserve prefix/output
// capacity of max_edges to avoid allocations during DFS.
template<class Neighbors, class Color, class Value, class Blocked, class LowerBound>
int maximize(int start, int goal, int max_edges, int color_count,
             const Neighbors& neighbors, const Color& color, const Value& value,
             const Blocked& blocked, const LowerBound& lower_bound,
             std::vector<int>& output, std::vector<int>& prefix,
             std::vector<unsigned char>& on_path) {
    if (static_cast<int>(on_path.size()) != color_count) on_path.assign(color_count, 0);
    output.clear();
    prefix.clear();
    int best = -1;
    on_path[color(start)] = on_path[color(goal)] = 1;
    auto dfs = [&](auto&& self, int v, int left, int score) -> void {
        if (left < 1 || lower_bound(v) > left) return;
        for (int u : neighbors(v)) {
            if (u == goal) {
                if (score > best) { best = score; output = prefix; }
            } else if (left > 1 && lower_bound(u) <= left - 1 &&
                       !on_path[color(u)] && !blocked(color(u))) {
                on_path[color(u)] = 1;
                prefix.push_back(u);
                self(self, u, left - 1, score + value(u));
                prefix.pop_back();
                on_path[color(u)] = 0;
            }
        }
    };
    dfs(dfs, start, max_edges, 0);
    on_path[color(start)] = on_path[color(goal)] = 0;
    return best;
}
}
