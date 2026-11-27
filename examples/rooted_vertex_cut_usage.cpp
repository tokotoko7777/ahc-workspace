#include "library/rooted-vertex-cut.hpp"
#include <array>
#include <iostream>

int main() {
    const std::array<std::array<int,2>,4> adjacency{{{1,-1},{0,2},{1,3},{2,-1}}};
    // Marker 1: worker on vertex 0; marker 2: target on vertex 3.
    const std::array<std::uint64_t,4> objects{1,0,0,2};
    auto neighbors=[&](int p)->const auto&{return adjacency[p];};
    ahc::RootedVertexCut<4> cut;
    cut.build(0,neighbors,[](int){return true;},objects);
    auto current=cut.without(2);
    if(current && current->labels==1)
        std::cout<<"Target separated; shared area "<<current->size<<'\n';
}
