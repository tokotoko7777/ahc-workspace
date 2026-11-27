#pragma once
#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace ahc {
// All vertices reachable after some allowed number of steps, ignoring
// probabilities. Bit k of accepted_steps selects length k (0 through 63).
// At a dead end, optionally permit a stay; otherwise that walk terminates.
// Neighbor slots are nonnegative vertex indices, or -1 for a missing edge.
template<std::size_t N, class Neighbors>
std::bitset<N> exact_step_frontier(const Neighbors& graph,
                                   const std::bitset<N>& walls,
                                   const std::bitset<N>& initial,
                                   std::uint64_t accepted_steps,
                                   bool stay_at_dead_end=false){
    assert((initial & walls).none());
    using Index = std::conditional_t<
        N <= std::numeric_limits<std::uint16_t>::max() + std::size_t{1},
        std::uint16_t,std::size_t>;
    std::array<Index,N> a,b;
    Index* current=a.data(); Index* next=b.data();
    std::size_t n=0;
    for(std::size_t p=0;p<N;++p)if(initial[p])current[n++]=static_cast<Index>(p);
    std::bitset<N> out;
    if(accepted_steps&1)out=initial;
    for(accepted_steps>>=1;accepted_steps&&n;accepted_steps>>=1){
        std::bitset<N> selected;
        std::size_t m=0;
        auto put=[&](std::size_t q){
            if(!selected[q]){selected.set(q);next[m++]=static_cast<Index>(q);}
        };
        for(std::size_t i=0;i<n;++i){
            const auto p=current[i];bool moved=false;
            for(auto raw:graph[p]){
                if(raw<0)continue;
                const auto q=static_cast<std::size_t>(raw);assert(q<N);
                if(!walls[q]){moved=true;put(q);}
            }
            if(!moved&&stay_at_dead_end)put(p);
        }
        if(accepted_steps&1)out|=selected;
        auto saved=current;current=next;next=saved;n=m;
    }
    return out;
}
}
