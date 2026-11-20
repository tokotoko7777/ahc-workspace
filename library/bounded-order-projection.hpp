#pragma once
#include <algorithm>
#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>

// Cyclic pair projections for a bounded ordinal fit. Each pair (a,b)
// requests value[a] >= value[b]. This is a bounded-work approximation,
// not a certificate of global feasibility or a least-squares optimum.
template<class Real>
void project_bounded_order(std::vector<Real>& value,const std::vector<Real>& lower,
                           const std::vector<Real>& upper,
                           const std::vector<std::pair<int,int>>& constraints,
                           int sweeps) {
    static_assert(std::is_floating_point_v<Real>);
    assert(value.size()==lower.size() && value.size()==upper.size() && sweeps>=0);
    for(int pass=0;pass<sweeps;++pass) for(auto [a,b]:constraints) {
        assert(0<=a && a<int(value.size()) && 0<=b && b<int(value.size()));
        if(value[a]>=value[b]) continue;
        const Real lo=std::max(lower[a],lower[b]);
        const Real hi=std::min(upper[a],upper[b]);
        if(lo<=hi) value[a]=value[b]=std::clamp((value[a]+value[b])*Real(.5),lo,hi);
    }
}
