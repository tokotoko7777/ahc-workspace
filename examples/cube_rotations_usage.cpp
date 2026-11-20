#include "../library/cube-rotations.hpp"
#include <iostream>

int main() {
    // Rotate both coordinates and offset vectors with the same transform.
    const cube_rotations::Point p{1,2,3};
    for(const auto& rotate:cube_rotations::all) {
        const auto [x,y,z] = rotate(p);
        std::cout<<x<<' '<<y<<' '<<z<<'\n';
    }
}
