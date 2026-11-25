#include "../library/preserve-reachability.hpp"
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

int main() {
    constexpr int N = 18;
    using Mask = std::bitset<N>;
    std::mt19937 gen(123214);
    for (int rep = 0; rep < 8000; ++rep) {
        std::array<std::vector<int>,N> graph{};
        for (int i=0; i<N; ++i) for(int j=i+1; j<N; ++j) if(gen()%9==0) {
            graph[i].push_back(j); graph[j].push_back(i);
        }
        const std::vector<int> roots{0,1,2};
        std::vector<int> proposed;
        Mask old, terminal;
        for(int i=0; i<N; ++i) {
            if(gen()%3==0)terminal.set(i);
            if(i<3)continue;
            if(gen()%5==0)old.set(i);
            else if(gen()%2 && proposed.size()<7)proposed.push_back(i);
        }
        const auto adj=[&](int p)->const auto&{return graph[p];};
        const auto got=ahc::restore_cuts_preserving_terminals(old,proposed,roots,terminal,adj);
        const auto reach=[&](Mask closed,Mask start){
            // Slow graph-wide relaxation independent of the proposed DFS.
            for(int pass=0;pass<N;++pass)
                for(int v=0;v<N;++v)if(!closed[v]&&!start[v])
                    for(int u:graph[v])if(start[u]){start.set(v);break;}
            return start;
        };
        Mask full=old, sources, first;
        for(int q:proposed)full.set(q);
        for(int r:roots)sources.set(r);
        first.set(roots[0]);const Mask target=reach(full,sources)&terminal;
        int best=-1,area=-1;
        Mask opt;
        for(int b=0;b<(1<<proposed.size());++b){
            Mask w=old;for(unsigned i=0;i<proposed.size();++i)if(b>>i&1)w.set(proposed[i]);
            const Mask z=reach(w,first);
            if((z&sources)!=sources||(z&terminal)!=target)continue;
            if(int(z.count())>area){area=z.count();best=b;opt=z;}
        }
        assert(bool(got)==(best>=0));
        if(got){assert(int(got->keep)==best);assert(got->root_region==opt);}
    }
    std::cout<<"8000 independent exhaustive checks passed\n";
}
