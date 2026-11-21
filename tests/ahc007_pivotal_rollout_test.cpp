#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include "../library/dsu.hpp"
#include "../problems/ahc007/pivotal_rollout.hpp"

struct E {int from,to,cost,id;};
constexpr int N=6, M=15;
constexpr double INF=1e12;
double complete(Dsu d,const std::vector<E>& edges) {
    int total=0;
    auto copy=edges;
    std::sort(copy.begin(),copy.end(),[](auto a,auto b){return a.cost<b.cost;});
    for(auto e:copy)if(d.unite(e.from,e.to))total+=e.cost;
    return d.component_count()==1?double(total):INF;
}
template<int K>
void run(std::mt19937& rng) {
    std::vector<E> all;
    for(int u=0;u<N;++u) for(int v=u+1;v<N;++v)
        all.push_back({u,v,int(1+rng()%4),0});
    std::shuffle(all.begin(),all.end(),rng);
    for(int i=0;i<M;++i) all[i].id=i;
    constexpr int Query=3;
    Dsu accepted(N);
    for(int i=0;i<Query;++i) if(rng()%2) accepted.unite(all[i].from,all[i].to);
    auto current=all[Query];
    if(accepted.same(current.from,current.to))return;
    Dsu plus=accepted;
    plus.unite(current.from,current.to);
    auto future=accepted;
    for(auto e:all)if(e.id>Query)future.unite(e.from,e.to);
    if(future.component_count()!=1)return;
    std::array<int,K> crit;
    std::vector<int> ids;
    for(int j=Query+1;j<M;++j)ids.push_back(j);
    std::shuffle(ids.begin(),ids.end(),rng);
    for(int j=0;j<K;++j)crit[j]=ids[j];
    std::sort(crit.begin(),crit.end());
    std::vector<E> backup;
    for(auto e:all) {
        e.cost=1+rng()%30;
        if(e.id>Query && !std::binary_search(crit.begin(),crit.end(),e.id))
            backup.push_back(e);
    }
    auto sorted=backup;
    std::sort(sorted.begin(),sorted.end(),[](auto a,auto b){return a.cost < b.cost;});
    std::function<double(Dsu,int)> online=[&](Dsu t,int j){
        if (j==K) return complete(t,backup);
        E e=all[crit[j]];
        double no=online(t,j+1);
        t.unite(e.from,e.to);
        double yes=online(t,j+1);
        double tot=0;
        for(int w=e.cost;w<=3*e.cost;++w)tot+=std::min(no,w+yes);
        return tot/(2*e.cost+1);
    };
    auto observed=backup;
    std::function<double(int)> offline=[&](int j){
        if(j==K)return complete(accepted,observed)-complete(plus,observed);
        E e=all[crit[j]];
        double x=0;
        for(int w=e.cost;w<=3*e.cost;++w){
            auto em=e;em.cost=w;
            observed.push_back(em);
            x+=offline(j+1);
            observed.pop_back();
        }
        return x/(2*e.cost+1);
    };
    PivotalRollout<K,N,M> tested(all,Query,crit,30);
    double exact=online(accepted,0)-online(plus,0)-offline(0), ans=tested.correction(accepted,sorted,Query);
    assert(std::abs(exact-ans)<1e-6);
}
int main() {
    std::mt19937 rng(78884019);
    for(int i=0;i<1500;++i)run<2>(rng);
    for(int i=0;i<800;++i)run<3>(rng);
    for(int i=0;i<200;++i)run<4>(rng);
    std::cout<<"ok\n";
}
