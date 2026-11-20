#pragma GCC optimize("O3")
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
#include "../../library/bounded-depth-forest.hpp"

using Clock=std::chrono::steady_clock;
struct RNG {
    uint32_t s=134917841;
    uint32_t next() {s^=s<<13;s^=s>>17;s^=s<<5;return s;}
    double unit() {return (next()+0.5)/4294967296.0;}
};
struct Solver {
    int n,h;
    std::vector<std::vector<int>> adj;
    std::vector<long long> weights;
    RNG rng;
    BoundedDepthForest construct() {
        std::vector<int> order(n),cost(n),p(n,-2);
        for(int i=0;i<n;++i) order[i]=i,cost[i]=int(weights[i])*3+rng.next()%120;
        std::sort(order.begin(),order.end(),[&](int a,int b) {
            return std::pair(cost[a],a)<std::pair(cost[b],b);
        });
        auto visit=[&](auto&& self,int v,int d) -> void {
            if(d==h) return;
            std::vector<int> q;
            for(int a:adj[v]) if(p[a]==-2) q.push_back(a);
            std::sort(q.begin(),q.end(),[&](int a,int b) {
                return std::pair(cost[a],a)<std::pair(cost[b],b);
            });
            for(int u:q) if(p[u]==-2) p[u]=v,self(self,u,d+1);
        };
        for(int v:order) if(p[v]==-2) p[v]=-1,visit(visit,v,0);
        return {p,weights,h};
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Solver s;
    int m;
    if(!(std::cin>>s.n>>m>>s.h)) return 1;
    s.adj.resize(s.n);s.weights.resize(s.n);
    for(auto& w:s.weights) std::cin>>w;
    for(int i=0,u,v;i<m;++i) {
        std::cin>>u>>v;s.adj[u].push_back(v);s.adj[v].push_back(u);
    }
    const auto start=Clock::now();
    const auto limit=start+std::chrono::milliseconds(140);
    auto current=s.construct();
    auto parent=current.parents();
    auto best=current.score();
    auto improve=[&](const BoundedDepthForest& a) {
        if(a.score()>best) best=a.score(),parent=a.parents();
    };
    for(int i=0;i<12 && Clock::now()<limit;++i) {
        auto next=s.construct();
        if(next.score()>current.score()) current=std::move(next);
        improve(current);
    }
#ifdef AHC041_RESTARTS_ONLY
    while(Clock::now()<limit) improve(s.construct());
#else

    while(Clock::now()<limit) {
        const double phase=std::min(1.,std::chrono::duration<double,std::milli>(Clock::now()-start).count()/140.);
        const double temp=124.*std::pow(.018,phase);
        for(int rep=0;rep<256;++rep) {
            int v=s.rng.next()%s.n;
            int d=s.adj[v].size();
            unsigned x=s.rng.next()%(d+1);
            int p=int(x)==d ? -1 : s.adj[v][x];
            if(current.try_reparent(v,p,[&](long long change) {
                return change>=0 || s.rng.unit()<std::exp(change/temp);
            })) improve(current);
        }
    }
#endif
    for(int v=0;v<s.n;++v) std::cout<<parent[v]<<(v+1==s.n?'\n':' ');
    std::cerr<<"Model = "<<best+1<<'\n';
}
