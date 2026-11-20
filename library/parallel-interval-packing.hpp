#pragma once
#include <algorithm>
#include <cassert>
#include <vector>
#include "min-cost-flow.hpp"

// Maximum-weight fixed, inclusive intervals on identical interchangeable lanes.
// Returns indices into jobs organized by lane. Each job is used at most once.
// Weights are nonnegative and must not exceed the inclusive interval length.
struct ParallelIntervalJob { int start,end,weight; };
inline std::vector<std::vector<int>> pack_parallel_intervals(
    int horizon,int lanes,const std::vector<ParallelIntervalJob>& jobs) {
    assert(horizon>0 && lanes>0);
    MinCostFlow<int,long long> network(horizon+2);
    for(int day=1;day<=horizon;++day) network.add_edge(day,day+1,lanes,1);
    for(auto [s,e,w]:jobs) {
        assert(1<=s && s<=e && e<=horizon && 0<=w && w<=e-s+1);
        network.add_edge(s,e+1,1,e-s+1-w);
    }
    auto [sent,cost]=network.flow(1,horizon+1,lanes);
    assert(sent==lanes); // idle edges can always carry all lanes
    std::vector<std::vector<int>> result(lanes);
    std::vector<int> last(lanes);
    std::vector<int> chosen;
    for(int j=0;j<int(jobs.size());++j)
        if(network.get_edge(horizon+j).flow) chosen.push_back(j);
    std::sort(chosen.begin(),chosen.end(),[&](int a,int b){
        return jobs[a].start<jobs[b].start;
    });
    // Every cut is crossed by exactly lanes units; therefore the selected
    // intervals have maximum overlap <= lanes and first available lane works.
    long long benefit=0;
    for(int j:chosen) {
        auto [s,e,w]=jobs[j];
        int k=0;
        while(k<lanes && last[k]>=s) ++k;
        assert(k<lanes);
        result[k].push_back(j);
        last[k]=e;
        benefit+=w;
    }
    assert(benefit==1LL*horizon*lanes-cost);
    return result;
}
