#include "../library/greedy-path-forest.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <set>

int main() {
    std::mt19937 rng(3651583);
    for(int trial=0;trial<1200;++trial) {
        int n=1+rng()%14;
        std::vector<std::pair<int,int>> edges;
        for(int i=0,k=rng()%80;i<k;++i) {
            int a=rng()%n,b=rng()%n;
            edges.emplace_back(std::min(a,b),std::max(a,b));
        }
        std::vector<std::vector<int>> forest(n);
        std::set<std::pair<int,int>> want,actual;
        for(auto [a,b]:edges) {
            if(forest[a].size()==2||forest[b].size()==2) continue;
            std::vector<bool> seen(n);seen[a]=true;
            std::vector<int> q{a};
            for(int i=0;i<int(q.size());++i)
                for(int u:forest[q[i]]) if(!seen[u]) q.push_back(u),seen[u]=true;
            if(seen[b]) continue;
            forest[a].push_back(b);forest[b].push_back(a);
            want.emplace(a,b);
        }
        std::vector<int> appears(n);
        for(auto& path:greedy_path_forest(n,edges)) {
            assert(!path.empty());
            for(auto v:path) assert(!appears[v]++);
            for(int j=1;j<int(path.size());++j)
                actual.emplace(std::min(path[j-1],path[j]),std::max(path[j-1],path[j]));
        }
        assert(actual==want);
        assert(std::count(appears.begin(),appears.end(),1)==n);
    }
}
