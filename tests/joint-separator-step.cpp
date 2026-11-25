#include "../library/joint-separator-step.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <vector>

namespace {
constexpr int W = 5, V = W * W;
using Mask = std::bitset<V>;
std::array<int, 4> adj(int p) {
    return {p >= W ? p-W : -1, p%W < W-1 ? p+1 : -1,
            p+W < V ? p+W : -1, p%W ? p-1 : -1};
}
int brute(Mask wall, Mask legal, Mask excluded, const std::vector<int>& people) {
    std::vector<int> pos=people;
    Mask put;
    auto explore=[&](auto&& dfs, size_t i)->int {
        if(i==people.size()) {
            for(int p:pos) if(put[p]) return 0;
            Mask cells; std::vector<int> q{pos[0]}; cells.set(pos[0]);
            for(size_t k=0;k<q.size();++k) for(int v:adj(q[k])) {
                if(v<0||wall[v]||put[v]||cells[v]) continue;
                cells.set(v);q.push_back(v);
            }
            if((cells&excluded).any()) return 0;
            for(int p:pos) if(!cells[p]) return 0;
            return q.size();
        }
        const int p=people[i];
        pos[i]=p;
        int best=dfs(dfs,i+1);
        for(int v:adj(p)) if(v>=0&&!wall[v]) {
            pos[i]=v;
            best=std::max(best,dfs(dfs,i+1));
        }
        pos[i]=p;
        for(int v:adj(p)) if(v>=0&&legal[v]&&!put[v]) {
            put.set(v);
            best=std::max(best,dfs(dfs,i+1));
            put.reset(v);
        }
        return best;
    };
    return explore(explore,0);
}
}
int main() {
    std::mt19937 g(42189);
    for(int t=0;t<600;++t) {
        Mask walls,legal,excluded;std::vector<int> open;
        for(int p=0;p<V;++p) if(g()%6==0)walls.set(p);else open.push_back(p);
        const int m=1+g()%3;
        if(int(open.size())<m+2){--t;continue;}
        std::shuffle(open.begin(),open.end(),g);
        const std::vector<int> people(open.begin(),open.begin()+m);
        excluded.set(open[m]); if(t%2)excluded.set(open[m+1]);
        legal=~(walls|excluded);
        for(int p:people)legal.reset(p);
        for(int p=0;p<V;++p) if(excluded[p])for(int v:adj(p))if(v>=0)legal.reset(v);
        const auto z=ahc::joint_separator_step(walls,legal,excluded,people,adj);
        assert(z.area==brute(walls,legal,excluded,people));
        assert(z.commands.empty()==(z.area==0));
    }
}
