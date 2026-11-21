#include <array>
#include <cassert>
#include <cmath>
#include <random>
#include <vector>
#include "../problems/ahc008/dog-target-filter.hpp"

int main() {
    // 2x3 layout. Humans share positions sometimes; walls are rebuilt here
    // across test epochs to exercise changing reachability.
    std::mt19937 rng(822);
    for (int test=0;test<1000;++test) {
        const int m=1+rng()%5;
        AHC008DogTargetFilter filter(m);
        int dog=rng()%6, truth=-1;
        for (int turn=0;turn<100;++turn) {
            std::array<std::vector<int>,6> g;
            for (int i=0;i<6;++i) for (int j=0;j<i;++j) {
                bool adj=std::abs(i/3-j/3)+std::abs(i%3-j%3)==1;
                if (adj && rng()%5 !=0) g[i].push_back(j),g[j].push_back(i);
            }
            if (g[dog].empty()) {
                const int q=(dog+3)%6;
                g[dog].push_back(q); g[q].push_back(dog);
            }
            std::array<int,5> where{};
            std::array<std::array<int,6>,5> d;
            for (int h=0;h<m;++h) {
                where[h]=rng()%6;
                d[h].fill(-1);d[h][where[h]]=0;
                std::vector<int> q{where[h]};
                for (std::size_t i=0;i<q.size();++i) for (int u:g[q[i]]) if (d[h][u]<0)
                    d[h][u]=d[h][q[i]]+1,q.push_back(u);
            }
            if (truth<0||d[truth][dog]<=0) {
                std::vector<int> eligible;
                for (int h=0;h<m;++h) if (d[h][dog]>0) eligible.push_back(h);
                truth=eligible.empty()?-1:eligible[rng()%eligible.size()];
            }
            std::vector<int> move;
            int from=dog;
            if (truth>=0) {
                std::vector<int> options;
                for (int u:g[dog]) if (d[truth][u]==d[truth][dog]-1) options.push_back(u);
                assert(!options.empty());
                dog=options[rng()%options.size()];move.push_back(dog);
                if (dog==where[truth]) truth=-1;
            }
            dog=g[dog][rng()%g[dog].size()];move.push_back(dog);
            if (truth>=0 && dog==where[truth]) truth=-1;
            assert(filter.update(from,g[from],move,[&](int h,int u){return d[h][u];}));
            double total=filter.reset_probability();
            for (int h=0;h<m;++h) total+=filter.probability(h);
            assert(std::abs(total-1)<1e-10);
            assert(filter.probability(truth)>0);
        }
    }
}
