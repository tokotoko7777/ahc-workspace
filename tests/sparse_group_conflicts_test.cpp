#include "../library/sparse-group-conflicts.hpp"
#include <cassert>
#include <random>
#include <tuple>

int main() {
    std::mt19937 rng(549877);
    for(int trial=0;trial<800;++trial) {
        const int n=1+rng()%20, groups=1+rng()%7;
        SparseGroupConflicts g(n);
        std::vector<std::tuple<int,int,int>> terms;
        std::vector<int> labels(n);
        for(int& v:labels) v=rng()%groups;
        for(int i=0;i<80;++i) {
            int a=rng()%n,b=rng()%n,w=int(rng()%91)-45;
            if(a==b) continue;
            g.add(a,b,w);
            terms.emplace_back(a,b,w);
        }
        auto naive=[&] {
            std::int64_t sum=0;
            for(auto [a,b,w]:terms) if(labels[a]==labels[b]) sum+=w;
            return sum;
        };
        for(int i=0;i<30;++i) {
            int a=rng()%n,b=rng()%n,color=rng()%groups;
            auto prior=naive();
            assert(g.cost(labels)==prior);
            if(i%2) {
                auto delta=g.move_delta(labels,a,color);
                labels[a]=color;
                assert(naive()==prior+delta);
            } else {
                auto delta=g.swap_delta(labels,a,b);
                std::swap(labels[a],labels[b]);
                assert(naive()==prior+delta);
            }
        }
    }
}
