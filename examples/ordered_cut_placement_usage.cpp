#include "../library/ordered-cut-placement.hpp"
#include <iostream>
int main() {
    std::vector<int> reward(11);
    reward[3]=1;reward[6]=2;
    auto picked=maximize_ordered_cut_reward(10,{2,3,2},reward);
    std::cout << picked.reward << ':';
    for(int cut:picked.cuts) std::cout<<' '<<cut;
    std::cout<<'\n';
}
