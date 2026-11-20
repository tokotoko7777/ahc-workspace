#include "../library/parallel-interval-packing.hpp"
#include <iostream>
int main() {
    const auto plan=pack_parallel_intervals(8,2,{{1,4,4},{3,6,4},{5,8,4}});
    for(const auto& lane:plan) {
        for(int job:lane) std::cout<<job<<' ';
        std::cout<<'\n';
    }
}
