#pragma once
#include <cassert>
#include <numeric>
#include <vector>

struct OrderedCutPlacement {
    int reward;
    std::vector<int> cuts;
};

// Place ordered cuts between fixed endpoints 0 and length. The number of
// gaps equals min_gap.size(), and each has a positive minimum length. Each
// internal cut collects reward_at[coordinate]. O(gaps * (slack + 1)).
inline OrderedCutPlacement maximize_ordered_cut_reward(
    int length,const std::vector<int>& min_gap,const std::vector<int>& reward_at) {
    assert(!min_gap.empty() && int(reward_at.size())==length+1);
    for(int gap:min_gap) assert(gap>0);
    const int slack=length-std::accumulate(min_gap.begin(),min_gap.end(),0);
    assert(slack>=0);
    const int k=int(min_gap.size())-1;
    if(!k) return {0,{}};
    std::vector<int> previous(slack+1),current(slack+1),positions(k);
    std::vector<int> predecessor(k*(slack+1));
    int minimum=0;
    for(int i=0;i<k;++i) {
        minimum+=min_gap[i];
        positions[i]=minimum;
        int best=0;
        for(int extra=0;extra<=slack;++extra) {
            // Cumulative slack is nondecreasing from one cut to the next.
            if(previous[extra]>previous[best]) best=extra;
            current[extra]=previous[best]+reward_at[minimum+extra];
            predecessor[i*(slack+1)+extra]=best;
        }
        previous.swap(current);
    }
    int extra=0;
    for(int e=1;e<=slack;++e) if(previous[e]>previous[extra]) extra=e;
    OrderedCutPlacement answer{previous[extra],positions};
    for(int i=k-1;i>=0;--i) {
        answer.cuts[i]+=extra;
        extra=predecessor[i*(slack+1)+extra];
    }
    return answer;
}
