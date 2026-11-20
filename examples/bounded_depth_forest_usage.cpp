#include "../library/bounded-depth-forest.hpp"
#include <iostream>
int main() {
    BoundedDepthForest forest({-1,0,-1},{2,7,1},2);
    forest.try_reparent(0,2,[](long long gain){return gain>0;});
    std::cout<<forest.score()<<'\n'; // 26
}
