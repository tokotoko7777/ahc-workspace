#include "../library/paired-coverage-reward.hpp"
#include <array>
#include <cassert>
#include <random>
int main() {
    std::mt19937 rng(43043041);
    for(int trial=0;trial<300;++trial) {
        int n=1+rng()%40;
        std::vector<long long> w(n);
        for(auto& v:w) v=int(rng()%100)-20;
        std::vector<std::array<int,2>> covered(n);
        auto full=[&](const auto& state) {
            long long x=0;
            for(int i=0;i<n;++i) if(state[i][0] && state[i][1]) x+=w[i];
            return x;
        };
        PairedCoverageReward state(w);
        for(int step=0;step<100;++step) {
            std::vector<int> events(rng()%21);
            auto preview=covered;
            for(auto& e:events) {
                e=rng()%(2*n);
                ++preview[e/2][e%2];
            }
            assert(state.delta(events)==full(preview)-full(covered));
            assert(state.value()==full(covered));
            if(rng()%3==0) state.add(events),covered=preview;
            assert(state.value()==full(covered));
        }
    }
}
