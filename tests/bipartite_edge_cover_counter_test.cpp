#include "../library/bipartite-edge-cover-counter.hpp"
#include <cassert>
#include <random>

int main(){
    std::mt19937 rng(34289);
    for(int rep=0;rep<300;++rep){
        const int l=1+int(rng()%17),r=1+int(rng()%21);
        std::vector<std::pair<int,int>> edges;
        for(int i=0;i<90;++i)edges.emplace_back(rng()%l,rng()%r);
        BipartiteEdgeCoverCounter cover(l,r,edges);
        std::array<std::vector<int>,2> cnt{std::vector<int>(l),std::vector<int>(r)};
        auto expected=[&](){
            int miss=0;
            for(auto [a,b]:edges)miss+=cnt[0][a]==0&&cnt[1][b]==0;
            assert(miss==cover.uncovered());
            assert(cover.all_covered()==(miss==0));
        };
        expected();
        for(int step=0;step<500;++step){
            int side=rng()%2,id=rng()%cnt[side].size();
            int old=cnt[side][id],delta=(rng()&1)?1:-(old>0);
            cnt[side][id]+=delta;cover.add(side,id,delta);
            assert(cover.activity(side,id)==cnt[side][id]);
            expected();
            if(rng()%3==0){cnt[side][id]-=delta;cover.add(side,id,-delta);expected();}
        }
    }
}
