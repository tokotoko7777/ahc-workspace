#pragma once
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

// Traverse a fixed chronological swap list backwards. If a swap at that
// point is omitted, its original two endpoints become a swap on the final
// state, transported by the retained suffix permutation. The callback must
// test such a final-state swap, commit it only on true, and otherwise leave
// the final state unchanged. Returns the number of removed swaps.
template<class AcceptFinalSwap>
std::size_t prune_swaps_by_suffix(int n,std::vector<std::pair<int,int>>& moves,
                                 AcceptFinalSwap accept_final_swap) {
    std::vector<int> at(n);
    std::iota(at.begin(),at.end(),0);
    std::size_t write=moves.size();
    for(std::size_t read=moves.size();read>0;) {
        const auto [a,b]=moves[--read];
        if(accept_final_swap(at[a],at[b])) continue;
        std::swap(at[a],at[b]);
        moves[--write]={a,b};
    }
    moves.erase(moves.begin(),moves.begin()+write);
    return write;
}
