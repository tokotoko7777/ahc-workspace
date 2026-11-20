#include "../library/class-constrained-pair-insertion.hpp"
#include <array>
#include <cassert>
#include <random>
#include <tuple>

int main() {
    std::mt19937 rng(41458501);
    for(int test=0;test<10000;++test) {
        const int n=2+rng()%18,classes=1+rng()%7;
        std::vector<int> route(n),group(n);
        for(int i=0;i<n;++i) route[i]=i,group[i]=rng()%classes;
        std::array<std::array<int,22>,22> table;
        for(auto& row:table) for(int& x:row) x=int(rng()%61)-25;
        auto dist=[&](int a,int b){return table[a][b];};
        auto cost=[&](const std::vector<int>& path) {
            int score=0;
            for(int i=1;i<int(path.size());++i) score+=dist(path[i-1],path[i]);
            return score;
        };
        auto fast=best_class_constrained_pair_insertion(route,group,classes,n,n+1,dist);
        auto slow=std::tuple(100000000,-1,-1);
        for(int l=1;l<n;++l) for(int r=l;r<n;++r) if(group[l]==group[r]) {
            auto copy=route;
            copy.insert(copy.begin()+r,n+1);
            copy.insert(copy.begin()+l,n);
            slow=std::min(slow,std::tuple(cost(copy)-cost(route),l,r));
        }
        assert((std::tuple(fast.delta,fast.first_gap,fast.second_gap)==slow));
    }
}
