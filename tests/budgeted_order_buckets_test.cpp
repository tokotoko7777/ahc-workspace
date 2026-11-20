#include "../library/budgeted-order-buckets.hpp"
#include <cassert>
#include <numeric>
#include <random>

int main() {
    std::mt19937 rng(267151);
    for(int trial=0;trial<1200;++trial) {
        int n=rng()%90,budget=trial%9==0?n*n:int(rng()%400);
        std::vector<int> vals(n),ids(n);
        for(int& value:vals) value=rng()%50;
        std::iota(ids.begin(),ids.end(),0);
        std::shuffle(ids.begin(),ids.end(),rng);
        int queries=0;
        auto buckets=budgeted_order_buckets(ids,budget,[&](int a,int b) {
            ++queries; return (vals[a]>vals[b])-(vals[a]<vals[b]);
        },rng);
        int last=-1;
        std::vector<int> seen(n);
        for(const auto& group:buckets) {
            int high=last;
            for(int id:group) {
                assert(id>=0 && id<n && !seen[id]);
                seen[id]=1;
                assert(vals[id]>=last);
                high=std::max(high,vals[id]);
            }
            last=high;
        }
        assert(std::count(seen.begin(),seen.end(),1)==n && queries<=budget);
        if(trial%9==0)
            for(const auto& group:buckets) for(int v:group) assert(vals[v]==vals[group[0]]);
    }
}
