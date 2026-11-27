#pragma once

#include <algorithm>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ahc {
struct WalkSchedulePhase {
    // Check these positions before closing vertices. The caller lists any
    // required safety buffer here (e.g. neighbors of the planned wall).
    std::span<const int> forbidden_before;
    // Closed vertices are also implicitly forbidden before this phase.
    std::span<const int> close;
    int steps_after = 0;
};

// Uniform neighbor walk through predetermined closures. No random
// retries/renormalization: the answer is joint survival + final arrival mass.
// The graph's vertex IDs are [0,n). Duplicate neighbors are separate choices.
// At a vertex with no traversable neighbor the walker stays in place.
// A vertex that was already closed may be closed again harmlessly.
class WalkScheduleWorkspace {
public:
    template<class Neighbors, class Accept>
    double probability(int n, Neighbors&& neighbors, int start,
                       std::span<const int> initially_closed,
                       std::span<const WalkSchedulePhase> phases,
                       Accept&& final_accept) {
        if (n <= 0 || start < 0 || start >= n)
            throw std::invalid_argument("invalid walk-schedule start");
        closed_.assign(n, 0);
        forbidden_.assign(n, 0);
        rows_[0].assign(n, 0.);
        rows_[1].assign(n, 0.);
        active_[0].clear();
        active_[1].clear();
        active_[0].reserve(n);
        active_[1].reserve(n);

        auto check = [n](int v) {
            if (v < 0 || v >= n) throw std::out_of_range("walk-schedule vertex");
        };
        for (int v : initially_closed) { check(v); closed_[v] = 1; }
        if (closed_[start]) return 0.;
        int front = 0;
        active_[front].push_back(start);
        rows_[front][start] = 1.;
        for (const auto& phase : phases) {
            if (phase.steps_after < 0)
                throw std::invalid_argument("negative walk-schedule steps");
            for (int v : phase.forbidden_before) { check(v); forbidden_[v] = 1; }
            for (int v : phase.close) { check(v); closed_[v] = 1; }
            auto& here = active_[front];
            here.erase(std::remove_if(here.begin(), here.end(), [&](int v) {
                if (!forbidden_[v] && !closed_[v]) return false;
                rows_[front][v] = 0.;
                return true;
            }), here.end());
            for (int v : phase.forbidden_before) forbidden_[v] = 0;
            for (int t = 0; t < phase.steps_after; ++t) {
                auto& src = rows_[front];
                auto& dst = rows_[front ^ 1];
                auto& next = active_[front ^ 1];
                auto add = [&](int v, double mass) {
                    if (mass == 0.) return;
                    if (dst[v] == 0.) next.push_back(v);
                    dst[v] += mass;
                };
                for (int u : active_[front]) {
                    int deg = 0;
                    for (int v : neighbors(u)) {
                        check(v);
                        if (!closed_[v]) ++deg;
                    }
                    if (!deg) add(u, src[u]);
                    else for (int v : neighbors(u))
                        if (!closed_[v]) add(v, src[u] / deg);
                    src[u] = 0.;
                }
                active_[front].clear();
                front ^= 1;
            }
        }
        double ans = 0.;
        for (int v : active_[front]) if (final_accept(v)) ans += rows_[front][v];
        return ans;
    }
private:
    std::vector<unsigned char> closed_, forbidden_;
    std::vector<double> rows_[2];
    std::vector<int> active_[2];
};
}  // namespace ahc
