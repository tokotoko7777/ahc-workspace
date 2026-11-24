#include "shortest-mandatory-arc.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

using Index = ahc::shortest_mandatory_arc;
using Arc = Index::arc;
using Graph = std::vector<Arc>;

std::vector<int> oracle(int n,const Graph& g,int t,int removed){
    std::vector<int>d(n,-1),q{t};
    d[t]=0;
    for(std::size_t i=0;i<q.size();++i){
        int x=q[i];
        // Deliberately use original arcs (not the shortest DAG).
        for(int j=0;j<static_cast<int>(g.size());++j)if(j!=removed){
            auto [u,v]=g[j];
            if(v==x&&d[u]<0){d[u]=d[x]+1;q.push_back(u);}
        }
    }
    return d;
}
std::uint64_t verify(int n,const Graph& g,int t){
    Index a(n,g,t);
    const auto base=oracle(n,g,t,-1);
    for(int s=0;s<n;++s)assert(a.distance(s)==base[s]);
    for(int e=0;e<static_cast<int>(g.size());++e){
        const auto alt=oracle(n,g,t,e);
        for(int s=0;s<n;++s){
            if(base[s]<0)assert(!a.mandatory(s,e));
            else assert(a.mandatory(s,e)==std::optional<bool>(alt[s]!=base[s]));
        }
    }
    return std::uint64_t(n)*(g.size()+1);
}
template<class E,class Fn> void raises(Fn f){
    bool yes=false;try{f();}catch(const E&){yes=true;}assert(yes);
}
int main(){
    std::uint64_t checks=0;
    for(int n=1;n<=4;++n){
        for(unsigned b=0;b<(1u<<(n*n));++b){
            Graph g;
            for(int e=0;e<n*n;++e)if((b>>e)&1)g.push_back({e/n,e%n});
            for(int t=0;t<n;++t)checks+=verify(n,g,t);
        }
    }
    std::mt19937 rng(0x5311753d);
    for(int i=0;i<2200;++i){
        const int n=1+rng()%17,m=rng()%71,t=rng()%n;
        Graph g;
        for(int j=0;j<m;++j){
            if(j>0&&rng()%4==0){const Arc copy=g[rng()%j];g.push_back(copy);}
            else g.push_back({int(rng()%n),int(rng()%n)});
        }
        checks+=verify(n,g,t);
    }
    {
        const Graph g={{0,1},{0,2},{1,3},{2,3},{3,4},{3,4},{4,5},{5,5},{6,6}};
        Index table(7,g,5);
        assert(table.mandatory(0,4)==false&&table.mandatory(0,5)==false);
        assert(table.mandatory(0,6)==true&&table.mandatory(3,6)==true);
        assert(table.mandatory(5,6)==false&&!table.mandatory(6,6));
        raises<std::out_of_range>([&]{(void)table.mandatory(7,0);});
        raises<std::out_of_range>([&]{(void)table.mandatory(0,-1);});
        raises<std::out_of_range>([&]{(void)table.mandatory(0,9);});
        raises<std::out_of_range>([&]{(void)table.distance(-1);});
        raises<std::invalid_argument>([&]{(void)Index(0,g,0);});
        raises<std::invalid_argument>([&]{(void)Index(7,g,7);});
        raises<std::invalid_argument>([&]{(void)Index(6,g,5);});
    }
    {
        constexpr int n=50000;
        Graph g;
        for(int i=0;i<n-1;++i)g.push_back({i,i+1});
        const Index chain(n,g,n-1);
        assert(chain.distance(0)==n-1);
        assert(chain.mandatory(0,n-2)==true);
        assert(chain.mandatory(n-2,0)==false);
    }
    std::cout<<"oracle checks "<<checks<<'\n';
}
