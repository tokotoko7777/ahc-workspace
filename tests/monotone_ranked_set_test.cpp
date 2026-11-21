#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>
#include "../library/monotone-ranked-set.hpp"
template<int N> void run(std::mt19937& rng) {
    for(int t=0;t<20;++t) {
        MonotoneRankedSet<N> s;
        std::vector<int> perm(N);
        std::iota(perm.begin(),perm.end(),0);
        std::shuffle(perm.begin(),perm.end(),rng);
        std::vector<bool> active(N,true);
        for(int i=0;i<=N;++i) {
            if(i) {
                const auto u=perm[i-1];
                s.erase(u);
                s.erase(u);
                active[u]=false;
            }
            std::vector<int> got,expected;
            s.visit_until([&](auto rank){got.push_back(rank);return false;});
            for(int j=0;j<N;++j)if(active[j])expected.push_back(j);
            assert(got==expected);
            int desired=1+rng()%8,k=0;
            got.clear();
            const bool ended=s.visit_until([&](auto rank){
                got.push_back(rank);
                return ++k==desired;
            });
            assert(ended==(int(expected.size())>=desired));
            expected.resize(std::min(desired,int(expected.size())));
            assert(got==expected);
        }
    }
}
int main(){
    std::mt19937 rng(262651);
    run<1>(rng);run<63>(rng);run<64>(rng);run<65>(rng);
    run<200>(rng);run<2048>(rng);run<4096>(rng);
}
