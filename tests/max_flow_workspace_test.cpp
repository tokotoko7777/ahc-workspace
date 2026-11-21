#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <random>
#include <vector>
#include "../library/max-flow.hpp"

int main() {
    static constexpr int n=7;
    std::mt19937 rng(55281);
    MaxFlow<int> reused(n);
    for (int trial=0;trial<4000;trial++) {
        reused.clear_edges();
        assert(reused.edges().empty());
        int source=rng()%n, sink=(source+1+rng()%(n-1))%n;
        std::vector<std::array<int,3>> e;
        for (int i=0,k=rng()%75;i<k;i++) {
            int a=rng()%n,b=rng()%n,c=rng()%60;
            e.push_back({a,b,c});
            const int id=reused.add_edge(a,b,c);
            const auto info=reused.get_edge(id);
            assert(info.from==a&&info.to==b&&info.capacity==c&&info.flow==0);
        }
        int expect=INT_MAX;
        for(int w=0;w<(1<<n);w++)if((w>>source&1)&&!(w>>sink&1)) {
            int v=0;
            for(auto [a,b,c]:e)if((w>>a&1)&&!(w>>b&1))v+=c;
            expect=std::min(expect,v);
        }
        const int partial=reused.flow(source,sink,10);
        assert(partial==std::min(10,expect));
        assert(partial+reused.flow(source,sink)==expect);
        const auto cut=reused.min_cut(source);
        assert(cut[source]&&!cut[sink]);
        int cut_cost=0;
        for(auto [a,b,c]:e)if(cut[a]&&!cut[b])cut_cost+=c;
        assert(cut_cost==expect);
    }
}
