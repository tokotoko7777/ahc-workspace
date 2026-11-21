#pragma GCC optimize("O3")
#include "../library/weighted-bottleneck-witness.hpp"
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <tuple>

namespace {
const int N=25,M=100;
struct Edge {int from,to,id;};
struct Dsu {
    std::array<int,N> p;
    Dsu(){std::iota(p.begin(),p.end(),0);}
    int leader(int x){return p[x]==x?x:p[x]=leader(p[x]);}
    void join(int a,int b){p[leader(a)]=leader(b);}
};
}
int main() {
    std::mt19937 rng(726231);
    int checked=0;
    for(int t=0;t<1600;++t){
        std::array<int,M> w,order;
        std::iota(order.begin(),order.end(),0);
        std::vector<Edge> e;
        for(int i=0;i<M;++i) {
            int u=rng()%N,v=rng()%N;
            while(v==u)v=rng()%N;
            e.push_back({u,v,i});w[i]=t&1 ? rng()%400 : rng()%4096;
        }
        std::sort(order.begin(),order.end(),[&](int a,int b){return std::tie(w[a],a)<std::tie(w[b],b);});
        BidirBottleneck<N,M> query(e);
        Dsu accepted;
        int have=rng()%N;
        for(int i=0;i<have;++i)accepted.join(rng()%N,rng()%N);
        query.set_groups(accepted);
        for(int k=0;k<12;++k){
            int after=rng()%(M+1)-1,a=rng()%N,b=rng()%N;
            if(accepted.leader(a)==accepted.leader(b)) continue;
            auto ref=accepted;
            int want=-1;
            for(int id:order) if(id>after){
                ref.join(e[id].from,e[id].to);
                if(ref.leader(a)==ref.leader(b)){want=w[id];break;}
            }
            int witness=-1;
            int got=query.cutoff(w,after,a,b,&witness);
            assert(got==want);
            if(got>=0){
                assert(witness>after && witness<M && w[witness]==want);
                assert(query.cutoff(w,after,a,b)==want);
            }
            ++checked;
        }
    }
    std::cout<<checked<<" random contracted suffix bottleneck comparisons passed\n";
}
