#include "../library/max-flow.hpp"
#include "../library/bounded-residual-cut.hpp"
#include <cassert>
#include <random>
#include <tuple>

int main() {
    std::mt19937 rng(897215);
    int total=0;
    for (int i=0;i<3500;++i) {
        const int n=2+int(rng()%6), s=0, t=n-1;
        MaxFlow<int> net(n);
        std::vector<std::tuple<int,int,int>> edges;
        for (int u=0;u<n;++u)
            for (int v=0;v<n;++v) if (rng()%4==0) {
                int cap=rng()%9;
                net.add_edge(u,v,cap);
                edges.emplace_back(u,v,cap);
            }
        const int base=net.flow(s,t);
        for(int u=0;u<7;++u) {
            std::vector<std::pair<int,int>> imp;
            for (int k=rng()%6;k>0;--k) imp.emplace_back(rng()%n,rng()%n);
            const int budget=rng()%9;
            int mn=1<<20, small=0, large=0;
            for (int cut=1;cut<(1<<t);cut+=2) {
                bool ok=true;
                for(auto [a,b]:imp) if((cut>>a&1)&&!(cut>>b&1))ok=false;
                if(!ok)continue;
                int cap=0;
                for(auto [a,b,c]:edges) if((cut>>a&1)&&!(cut>>b&1))cap+=c;
                if(cap<mn){mn=cap;small=large=cut;}
                else if(cap==mn){small&=cut;large|=cut;}
            }
            const auto v=bounded_residual_cut::query(net,s,t,budget,imp,true);
            assert(bool(v)==(mn-base<=budget));
            if(v){
                assert(v->extra==mn-base);
                for(int j=0;j<n;++j){
                    assert(v->smallest_source[j]==(small>>j&1));
                    assert(v->largest_source[j]==(large>>j&1));
                }
            }
            ++total;
        }
    }
    assert(total==24500);
}
