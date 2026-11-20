#include "../library/swap-suffix-pruning.hpp"
#include <algorithm>
#include <iostream>

int main() {
    std::vector<int> final_board{1,3,2};
    std::vector<std::pair<int,int>> swaps{{0,1},{0,1},{1,2}};
    const auto removed=prune_swaps_by_suffix(3,swaps,[&](int a,int b) {
        auto copy=final_board;
        std::swap(copy[a],copy[b]);
        // The application cares only about keeping its minimum at position 0.
        if(copy[0]!=1) return false;
        final_board=std::move(copy);
        return true;
    });
    std::cout<<removed<<" of 3 swaps removed\n";
    return removed==1 ? 0 : 1;
}
