#include "../library/two-sided-dag-frontier.hpp"
#include <random>

int main() {
    std::mt19937 rng(173216);
    for(int trial=0;trial<1000;++trial) {
        int n=1+rng()%30;
        std::vector<std::vector<int>> edges(n);
        for(int a=0;a<n;++a) for(int b=a+1;b<n;++b)
            if(rng()%7==0) edges[a].push_back(b);
        TwoSidedDagFrontier f(edges);
        std::vector<bool> live(n,true);
        for(int d=0;d<n;++d) {
            std::vector<std::pair<int,bool>> available;
            for(int v=0;v<n;++v) {
                bool low=live[v],high=live[v];
                for(int p=0;p<n;++p) if(live[p])
                    for(int c:edges[p]) if(c==v) low=false;
                for(int c:edges[v]) if(live[c]) high=false;
                assert(f.live(v)==live[v] && f.ready(v,false)==low && f.ready(v,true)==high);
                if(low) available.emplace_back(v,false);
                if(high) available.emplace_back(v,true);
            }
            assert(!available.empty());
            auto [id,high]=available[rng()%available.size()];
            live[id]=false; f.remove(id,high);
        }
    }
}
