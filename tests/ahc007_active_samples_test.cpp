#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include "../library/dsu.hpp"
#include "../problems/ahc007/active_samples.hpp"
struct E {int from,to,cost,id;};
int main() {
    constexpr int N=14, M=N*(N-1)/2, Samples=9;
    std::mt19937 rng(33144139);
    for(int t=0;t<150;++t){
        std::vector<E> e;
        for(int u=0;u<N;++u)for(int v=u+1;v<N;++v)e.push_back({u,v,0,0});
        std::shuffle(e.begin(),e.end(),rng);
        for(int i=0;i<M;++i)e[i].id=i;
        std::array<std::vector<E>,Samples> orders;
        for(auto& o:orders){
            o=e;
            for(auto& ed:o)ed.cost=1+rng()%31;
            std::sort(o.begin(),o.end(),[](auto a,auto b) {
                return std::pair(a.cost,a.id)<std::pair(b.cost,b.id);
            });
        }
        ActiveKruskal<N,M> a(e,orders);
        Dsu accepted(N);
        for(int i=0;i<M;++i) {
            a.erase(i);
            const auto [u,v,w,id]=e[i];
            if(accepted.same(u,v))continue;
            for(int k=0;k<Samples;++k) {
                Dsu test=accepted;
                int expected=-1;
                for(auto ed:orders[k]) if(ed.id>i && test.unite(ed.from,ed.to) && test.same(u,v)){
                    expected=(ed.cost<<16)|ed.id;break;
                }
                assert(expected==a.simulations[k].cutoff<true>(a.parent,u,v));
            }
            if(rng()%4==0)accepted.unite(u,v),a.merge(u,v,i);
        }
    }
    std::cout<<"ok\n";
}
