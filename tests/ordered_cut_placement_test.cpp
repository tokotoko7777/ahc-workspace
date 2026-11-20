#include "../library/ordered-cut-placement.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <random>

int main() {
    std::mt19937 rng(20310031);
    for(int trial=0;trial<1200;++trial) {
        int size=1+rng()%5,length=rng()%8;
        std::vector<int> gaps(size);
        for(auto& d:gaps) d=1+rng()%3,length+=d;
        std::vector<int> reward(length+1);
        for(int& x:reward) x=int(rng()%9)-4;
        int best=std::numeric_limits<int>::min();
        std::function<void(int,int,int)> visit=[&](int gap,int last,int value) {
            if(gap==size-1) {
                if(length-last>=gaps[gap]) best=std::max(best,value);
                return;
            }
            for(int cut=last+gaps[gap];cut<length;++cut)
                visit(gap+1,cut,value+reward[cut]);
        };
        visit(0,0,0);
        auto result=maximize_ordered_cut_reward(length,gaps,reward);
        assert(int(result.cuts.size())==size-1 && result.reward==best);
        int last=0,value=0;
        for(int i=0;i<size-1;++i) {
            assert(result.cuts[i]-last>=gaps[i]);
            last=result.cuts[i];
            value+=reward[last];
        }
        assert(length-last>=gaps.back() && value==best);
    }
}
