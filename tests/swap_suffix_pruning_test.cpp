#include "../library/swap-suffix-pruning.hpp"
#include <algorithm>
#include <cassert>
#include <random>

using Swaps = std::vector<std::pair<int,int>>;
using Board = std::vector<int>;

Board execute(Board a,const Swaps& steps) {
    for(auto [x,y]:steps) std::swap(a[x],a[y]);
    return a;
}

int penalty(const Board& b) {
    int score=0;
    for(int i=0;i<int(b.size());++i) score+=std::abs(i-b[i]);
    return score;
}

int main() {
    std::mt19937 rng(447291);
    for(int trial=0;trial<1000;++trial) {
        int n=2+rng()%16;
        Board initial(n); std::iota(initial.begin(),initial.end(),0);
        std::shuffle(initial.begin(),initial.end(),rng);
        Swaps source;
        int m=rng()%48;
        for(int i=0;i<m;++i) {
            int a=rng()%n,b=rng()%(n-1);
            source.emplace_back(a,b+(b>=a));
        }
        auto fast=source, slow=source;
        auto final=execute(initial,fast);
        for(int pass=0;pass<5;++pass) {
            const auto count=prune_swaps_by_suffix(n,fast,[&](int a,int b) {
                auto attempt=final;
                std::swap(attempt[a],attempt[b]);
                if(penalty(attempt)>penalty(final)) return false;
                final=std::move(attempt);
                return true;
            });
            std::size_t removed=0;
            for(std::size_t i=slow.size();i>0;) {
                --i;
                auto attempt=slow;
                attempt.erase(attempt.begin()+i);
                if(penalty(execute(initial,attempt))<=penalty(execute(initial,slow))) {
                    slow=std::move(attempt); ++removed;
                }
            }
            assert(count==removed && slow==fast && execute(initial,fast)==final);
            if(!count) break;
        }
    }
}
