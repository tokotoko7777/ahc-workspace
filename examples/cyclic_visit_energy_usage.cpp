#include "../library/cyclic-visit-energy.hpp"
#include <array>
#include <iostream>

int main() {
    const std::array<int,4> route{0,1,0,2};
    const std::array<int,3> weight{2,1,3};
    CyclicVisitEnergy evaluator(3);
    const auto cost=evaluator.numerator(route,weight);
    if(!cost || *cost!=28) return 1;
    std::cout<<*cost/4.0<<'\n';  // weighted steady-state average
}
