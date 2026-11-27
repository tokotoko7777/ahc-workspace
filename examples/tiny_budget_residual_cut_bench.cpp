#include "../library/max-flow.hpp"
#include "../library/tiny-budget-residual-cut.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>

int main(){
    using Clock=std::chrono::steady_clock;
    constexpr int D=30,N=D*D,s=2*N,t=s+1;
    MaxFlow<int> graph(t+1);
    std::mt19937 rng(109731);
    for(int p=0;p<N;++p){
        graph.add_edge(2*p,2*p+1,rng()%5?5:0);
        if(p/D==0)graph.add_edge(s,2*p,1);
        if(p/D==D-1)graph.add_edge(2*p+1,t,3);
        for(auto v:{p>=D?p-D:-1,p+D<N?p+D:-1,p%D?p-1:-1,p%D<D-1?p+1:-1})
            if(v>=0)graph.add_edge(2*p+1,2*v,1<<20);
    }
    graph.flow(s,t);
    std::vector<std::vector<std::pair<int,int>>> tests(180);
    for(int i=0;i<(int)tests.size();++i)for(int j=i%4;j>0;--j){
        int p=rng()%N;
        tests[i].emplace_back(2*p,2*p+1);
    }
    for(int r=0;r<8;++r){
        std::uint64_t sums[2]{};
        double time[2]{};
        for(int arm:{r%2,1-r%2}){
            auto start=Clock::now();
            bounded_residual_cut::TinyBudgetWorkspace<int> ws;
            if(arm)ws.prepare(graph);
            for(auto& edges:tests){
                auto a=arm?ws.query(s,t,1,edges,true):bounded_residual_cut::query(graph,s,t,1,edges,true);
                if(a){
                    sums[arm]+=a->extra+1;
                    for(unsigned i=0;i<a->smallest_source.size();++i)
                        sums[arm]+=a->smallest_source[i]*(i+1)+a->largest_source[i]*(i+3);
                }
            }
            time[arm]=std::chrono::duration<double,std::milli>(Clock::now()-start).count();
        }
        assert(sums[0]==sums[1]);
        std::cout<<time[0]<<' '<<time[1]<<' '<<sums[0]<<'\n';
    }
}
