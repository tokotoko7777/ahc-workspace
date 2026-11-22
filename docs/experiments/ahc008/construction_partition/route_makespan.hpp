#pragma once
#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace route_makespan {
using Cost = std::int64_t;
struct Segment { int worker, first, last; }; // [first,last)
struct Plan { Cost makespan; std::vector<Segment> segments; };

// A fixed directed walk visits every task in order. travel[h][l] is the
// time for worker h to reach the stand for task l before construction.
// Require travel[h][l+1] <= travel[h][l] + next[l], e.g. distances in
// the same open graph in which the prescribed walk is valid. Then a worker
// who could cover [l,r) can start at any later available prefix <=r
// without finishing later, so one furthest reachable prefix per mask suffices.
// No waiting, blocking pets, or dynamic walls are modeled.
inline Plan solve(std::span<const Cost> work, std::span<const Cost> next,
                  const std::vector<std::vector<Cost>>& travel) {
    constexpr auto inf = std::numeric_limits<Cost>::max() / 8;
    int n = static_cast<int>(work.size()), m = static_cast<int>(travel.size());
    if (n == 0) return {0, {}};
    if (next.size()!=work.size() || m<=0 || m>20) throw std::invalid_argument("invalid dimensions");
    std::vector<Cost> before(n+1), through(n);
    for (int j=0;j<n;++j) {
        if (work[j]<0 || next[j]<0 || work[j]>=inf || next[j]>=inf || before[j]>=inf)
            throw std::invalid_argument("invalid or overflowing walk cost");
        through[j]=before[j]+work[j];
        before[j+1]=through[j]+next[j];
    }
    for (const auto& row:travel) {
        if (static_cast<int>(row.size())!=n) throw std::invalid_argument("bad approach size");
        for (int j=0;j<n;++j) {
            if (row[j]<0 || row[j]>=inf ||
                (j && row[j]>row[j-1]+next[j-1]))
                throw std::invalid_argument("approach violates route triangle bound");
        }
    }
    const int states=1<<m;
    auto reachable = [&](Cost budget, bool rebuild) -> std::vector<Segment> {
        std::vector<std::vector<int>> advance(m, std::vector<int>(n));
        for (int h=0;h<m;++h) {
            // The right endpoint cannot decrease as l increases.
            int end=0;
            for (int l=0;l<n;++l) {
                end=std::max(end,l);
                while(end<n && through[end]-before[l]+travel[h][l]<=budget)++end;
                advance[h][l]=end;
            }
        }
        std::vector<int> far(states,-1), prev(states,-1);
        far[0]=0;
        for (int mask=0;mask<states;++mask) {
            if (far[mask]<0) continue;
            if (far[mask]==n) {
                if(!rebuild) return {{-1,0,n}};
                std::vector<Segment> plan;
                while(mask) {
                    int p=prev[mask];
                    plan.push_back({std::countr_zero(static_cast<unsigned>(p^mask)),far[p],far[mask]});
                    mask=p;
                }
                std::reverse(plan.begin(),plan.end());
                return plan;
            }
            const int l=far[mask];
            for (int h=0;h<m;++h) if(!(mask>>h&1)) {
                int nxt=mask|1<<h,r=advance[h][l];
                if (r>l && r>far[nxt]) { far[nxt]=r; prev[nxt]=mask; }
            }
        }
        return {};
    };
    Cost lo=-1, hi=inf+through.back();
    for (auto &row:travel) hi=std::min(hi,row[0]+through.back());
    while(hi-lo>1) {
        Cost mid=lo+(hi-lo)/2;
        if(reachable(mid,false).empty())lo=mid;else hi=mid;
    }
    return {hi,reachable(hi,true)};
}
}
