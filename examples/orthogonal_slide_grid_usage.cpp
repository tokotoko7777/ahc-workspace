#include "../library/orthogonal-slide-grid.hpp"
#include <iostream>
int main() {
    OrthogonalSlideGrid grid(4,4,std::vector<unsigned char>(16));
    grid.set_blocked(7,true);
    auto stops=grid.destinations(5);  // left:4, right:6, up:1, down:13
    for(int v:stops) std::cout<<v<<' ';
    std::cout<<'\n';
}
