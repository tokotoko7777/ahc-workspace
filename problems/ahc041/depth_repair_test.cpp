#include "depth_repair.hpp"
#include <algorithm>
#include <cassert>
#include <random>

static long long brute(const BoundedDepthForest& state,
                       const std::vector<std::vector<int>>& adj,
                       const std::vector<long long>& weight,int cap) {
    const auto& parent=state.parents();
    const auto& depth=state.depths();
    const int n=parent.size();
    long long total=0;
    for(int v=0;v<n;++v) {
        int x=v,steps=0;
        while(parent[x]>=0) {
            int y=parent[x];
            assert(y<n && ++steps<=cap);
            assert(std::find(adj[x].begin(),adj[x].end(),y)!=adj[x].end());
            x=y;
        }
        assert(steps==depth[v]);
        total+=weight[v]*(steps+1);
    }
    return total;
}
int main() {
    {
        std::vector<int> p{-1,0,1,2,-1,4,-1};
        std::vector<long long> weights{2,10,10,1,1,1,1};
        std::vector<std::vector<int>> adj{{1},{0,2,5},{1,3},{2,6},{5},{4,1},{3}};
        BoundedDepthForest f(p,weights,3);
        auto score=f.score();
        assert(!f.try_reparent(1,5,[](long long){return true;}));
        assert(!try_depth_repair(f,adj,3,1,5,[](long long gain){ assert(gain==18);return false;}));
        assert(f.parents()==p && f.score()==score);
        assert(try_depth_repair(f,adj,3,1,5,[](long long gain){return gain==18;}));
        assert(f.score()==score+18 && f.score()==brute(f,adj,weights,3));
    }
    std::mt19937 rng(4558401);
    int repaired=0;
    for(int trial=0;trial<150;++trial) {
        const int n=25,cap=2+rng()%7;
        std::vector<int> p(n,-1);
        std::vector<long long> weights(n);
        std::vector<std::vector<int>> adj(n);
        for(int i=0;i<n;++i) {
            weights[i]=1+rng()%130;
            for(int j=i+1;j<n;++j) if(rng()%3==0)
                adj[i].push_back(j),adj[j].push_back(i);
        }
        BoundedDepthForest f(p,weights,cap);
        for(int turn=0;turn<300;++turn) {
            int v=rng()%n;
            if(adj[v].empty()) continue;
            int q=adj[v][rng()%adj[v].size()];
            if(turn%2==0) {
                f.try_reparent(v,q,[](long long){return true;});
            } else {
                const auto before=f.parents(), depths=f.depths();
                const auto old=f.score();
                bool call=false,want=rng()%3>0;
                bool got=try_depth_repair(f,adj,cap,v,q,[&](long long delta) {
                    call=true;assert(delta==brute(f,adj,weights,cap)-old);
                    return want;
                });
                assert(got==(call&&want));
                if(!got) assert(f.parents()==before && f.depths()==depths && f.score()==old);
                repaired+=got;
            }
            assert(f.score()==brute(f,adj,weights,cap));
        }
    }
    assert(repaired>100);
}
