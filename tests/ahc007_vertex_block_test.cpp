#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <numeric>
#include <random>
#include <vector>
#include "../library/dsu.hpp"
#include "../problems/ahc007/vertex_block.hpp"
struct E {int from,to,cost,id;};
int main() {
    constexpr int N=5,M=9;
    constexpr double INF=1e12;
    std::mt19937 rng(37168193);
    int complete_cases=0;
    for(int t=0;t<1100;++t) {
        std::vector<E> g;
        for(int v=1;v<N;++v)g.push_back({v,int(rng()%v),int(1+rng()%5),0});
        while(int(g.size())<M) {
            int a=rng()%N,b=rng()%N;
            if(a!=b)g.push_back({a,b,int(1+rng()%5),0});
        }
        std::shuffle(g.begin(),g.end(),rng);
        for(int i=0;i<M;++i)g[i].id=i;
        const int query=rng()%4;
        Dsu tree(N);
        for(int j=0;j<query;++j)
            if(rng()%2)tree.unite(g[j].from,g[j].to);
        if(tree.same(g[query].from,g[query].to))continue;
        Dsu check=tree;
        for(int j=query;j<M;++j)check.unite(g[j].from,g[j].to);
        if(check.component_count()!=1)continue;
        std::function<double(Dsu,int)> oracle=[&](Dsu s,int j) {
            if(s.component_count()==1)return 0.;
            if(j==M)return INF;
            double no=oracle(s,j+1);
            auto e=g[j];
            if(!s.unite(e.from,e.to))return no;
            double yes=oracle(s,j+1),total=0;
            for(int w=e.cost;w<=3*e.cost;++w)total+=std::min(no,yes+w);
            return total/(2*e.cost+1);
        };
        const double no=oracle(tree,query+1);
        auto with=tree;
        with.unite(g[query].from,g[query].to);
        double yes=oracle(with,query+1);
        auto a=VertexBlock<N,M>::limit<5>(tree,g,query);
        assert(a);
        if(no>=INF/2)assert(*a>=INF/2);
        else assert(std::abs(no-yes-*a)<1e-7);
        ++complete_cases;
    }
    assert(complete_cases>=900);
}
