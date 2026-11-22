#include "csr_dinic.hpp"
#include <cassert>
#include <iostream>
#include <random>
#include <utility>

using D=ahc::CsrDinic;
using C=D::Capacity;
using A=D::Arc;

void brute(int n,int s,int t,const std::vector<A>& arcs) {
    C min=std::numeric_limits<C>::max();
    unsigned meet=(1u<<n)-1;
    for(unsigned side=0;side<(1u<<n);++side) {
        if(!(side>>s&1) || (side>>t&1)) continue;
        C cost=0;
        for(auto [u,v,c]:arcs) if((side>>u&1) && !(side>>v&1)) cost+=c;
        if(cost<min){min=cost;meet=side;}
        else if(cost==min)meet&=side;
    }
    D d(n,arcs);
    assert(d.solve(s,t)==min);
    for(int v=0;v<n;++v)assert(d.on_source_side(v)==bool(meet>>v&1));
}

template<class Error,class Callback> void rejects(Callback call) {
    bool thrown=false;
    try { call(); } catch(const Error&) { thrown=true; }
    assert(thrown);
}

int main() {
    constexpr C max=std::numeric_limits<C>::max();
    {
        const std::vector<A> arcs{{0,0,max},{0,1,max-1},{0,1,1},{1,0,max}};
        D g(2,arcs);
        rejects<std::logic_error>([&]{(void)g.on_source_side(0);});
        rejects<std::out_of_range>([&]{g.solve(-1,1);});
        rejects<std::invalid_argument>([&]{g.solve(0,0);});
        assert(g.solve(0,1)==max);
        assert(g.on_source_side(0) && !g.on_source_side(1));
        rejects<std::logic_error>([&]{g.solve(0,1);});
        rejects<std::out_of_range>([&]{(void)g.on_source_side(2);});
    }
    {
        const std::vector<A> arcs{{0,1,max},{0,1,1}};
        D g(2,arcs);
        rejects<std::overflow_error>([&]{g.solve(0,1);});
        rejects<std::logic_error>([&]{(void)g.on_source_side(0);});
        assert(g.solve(1,0)==0);
    }
    {
        rejects<std::out_of_range>([]{D g(2,std::vector<A>{{-1,0,0}});});
        rejects<std::out_of_range>([]{D g(2,std::vector<A>{{0,2,1}});});
        rejects<std::invalid_argument>([]{D g(1,{});});
        brute(5,0,4,{});
        brute(5,0,4,{{0,1,3},{0,1,2},{1,0,4},{1,2,1},{1,1,8},{2,4,1},{3,0,2},{4,3,0}});
    }
    unsigned exhaust=0;
    for(int n=2;n<=3;++n){
        for(unsigned bits=0;bits<(1u<<(n*n));++bits){
            std::vector<A> a;
            for(int u=0;u<n;++u)for(int v=0;v<n;++v)
                if(bits>>(u*n+v)&1)a.push_back({u,v,1});
            for(int s=0;s<n;++s)for(int t=0;t<n;++t)if(s!=t){brute(n,s,t,a);++exhaust;}
        }
    }
    std::mt19937 rng(0xcdbc037u);
    for(int i=0;i<16000;++i){
        const int n=2+rng()%9, s=rng()%n;
        int t=rng()%(n-1); if(t>=s)++t;
        int k=rng()%(2*n*n+1);
        std::vector<A> a;
        for(int j=0;j<k;++j) a.push_back({int(rng()%n),int(rng()%n),C(rng()%25)});
        brute(n,s,t,a);
        std::reverse(a.begin(),a.end());
        brute(n,s,t,a);
    }
    std::cout<<exhaust<<" exhaustive topology/terminal combinations and 32000 seeded multigraph/order cases passed\n";
}
