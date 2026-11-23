#include "hall_small.hpp"
#include <array>
#include <iostream>
int main(){
    using ahc::WorkerMask;
    // Tasks 0 and 2 both depend on worker 0; worker 1 can handle task 1.
    constexpr std::array<WorkerMask,3> candidates{0b01,0b11,0b01};
    if (auto hall=ahc::find_hall_deficiency(2,candidates)) {
        std::cout<<"worker mask: "<<hall->worker_neighbors<<"\ntasks:";
        for(auto i:hall->tasks)std::cout<<' '<<i;
        std::cout<<'\n';
    }
}
