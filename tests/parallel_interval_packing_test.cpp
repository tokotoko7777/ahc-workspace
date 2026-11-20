#include "../library/parallel-interval-packing.hpp"
#include <cassert>
#include <random>
int main() {
    // Even the best tie-breaking in sequential per-lane optimization gives
    // 13, while selecting both lanes jointly can collect 14.
    const std::vector<ParallelIntervalJob> tricky{{2,4,3},{3,4,2},{3,7,5},
                                                  {5,8,4},{7,7,1},{1,2,2}};
    int joint=0;
    for(const auto& lane:pack_parallel_intervals(8,2,tricky))
        for(int j:lane) joint+=tricky[j].weight;
    assert(joint==14);
    std::mt19937 rng(104019);
    for(int trial=0;trial<1200;++trial) {
        int days=1+rng()%7,lanes=1+rng()%4,n=rng()%11;
        std::vector<ParallelIntervalJob> jobs(n);
        for(auto& j:jobs) {
            j.start=1+rng()%days;
            j.end=j.start+rng()%(days-j.start+1);
            j.weight=1+rng()%(j.end-j.start+1);
        }
        int optimum=0;
        for(int mask=0;mask<(1<<n);++mask) {
            std::vector<int> depth(days+1);
            int total=0;
            for(int j=0;j<n;++j) if(mask>>j&1) {
                auto [s,e,w]=jobs[j];
                for(int d=s;d<=e;++d) ++depth[d];
                total+=w;
            }
            if(*std::max_element(depth.begin(),depth.end())<=lanes)
                optimum=std::max(optimum,total);
        }
        auto plan=pack_parallel_intervals(days,lanes,jobs);
        assert(int(plan.size())==lanes);
        std::vector<int> used(n);
        int actual=0;
        for(auto lane:plan) {
            int last=0;
            for(int j:lane) {
                assert(!used[j]++);
                assert(last<jobs[j].start);
                last=jobs[j].end;
                actual+=jobs[j].weight;
            }
        }
        assert(actual==optimum);
    }
}
