#include "../library/dsu.hpp"
#include "../library/kruskal-bottleneck.hpp"
#include <cassert>
#include <random>

struct Edge { int from, to, cost, id; };
int main() {
    std::mt19937 r(7317);
    constexpr int inf=1000;
    for (int trial=0; trial<1500; ++trial) {
        int n=2+r()%9;
        std::vector<Edge> edges;
        std::vector<std::vector<int>> value(n, std::vector<int>(n,inf));
        Dsu d(n);
        for (int u=0; u<n; ++u) for (int v=u+1; v<n; ++v)
            if (r()%8==0) { d.unite(u,v); value[u][v]=value[v][u]=0; }
        for (int u=0;u<n;++u) {
            value[u][u]=0;
            for(int v=u+1;v<n;++v) {
                if(r()%2) continue;
                const int cost=1+r()%50, id=r()%3;
                edges.push_back({u,v,cost,id});
                if(id) value[u][v]=value[v][u]=std::min(value[u][v],cost);
            }
        }
        std::sort(edges.begin(),edges.end(),[](auto a,auto b){return a.cost<b.cost;});
        for(int k=0;k<n;++k) for(int i=0;i<n;++i) for(int j=0;j<n;++j)
            value[i][j]=std::min(value[i][j],std::max(value[i][k],value[k][j]));
        for(int i=0;i<n;++i) for(int j=i+1;j<n;++j) {
            if(d.same(i,j)) continue;
            Dsu scratch=d;
            auto result=kruskal_path_bottleneck(scratch,edges,i,j,[](auto e){return e.id>0;});
            assert(result.value_or(inf)==value[i][j]);
        }
    }
}
