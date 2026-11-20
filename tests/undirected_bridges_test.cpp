#include "../library/undirected-bridges.hpp"
#include <cassert>
#include <random>

int main() {
    std::mt19937 gen(67377151);
    for(int run=0;run<2500;++run) {
        int n=1+gen()%20,m=gen()%65;
        std::vector<std::pair<int,int>> list;
        for(int i=0;i<m;++i) list.emplace_back(gen()%n,gen()%n);
        auto flags=undirected_bridges(n,list);
        for(int absent=0;absent<m;++absent) {
            auto [a,b]=list[absent];
            std::vector<std::vector<int>> adj(n);
            for(int j=0;j<m;++j) if(j!=absent) {
                auto [u,v]=list[j];
                adj[u].push_back(v);adj[v].push_back(u);
            }
            std::vector<int> q{a};
            std::vector<bool> seen(n);
            seen[a]=true;
            for(unsigned i=0;i<q.size();++i)
                for(int v:adj[q[i]]) if(!seen[v]) seen[v]=true,q.push_back(v);
            assert(flags[absent]!=seen[b]);
        }
    }
}
