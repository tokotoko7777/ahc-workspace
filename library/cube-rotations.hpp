#pragma once
#include <algorithm>
#include <array>

namespace cube_rotations {
using Point=std::array<int,3>;
struct Rotation {
    Point perm,sign;
    constexpr Point operator()(Point p) const {
        return {sign[0]*p[perm[0]],sign[1]*p[perm[1]],sign[2]*p[perm[2]]};
    }
};
constexpr auto proper() {
    std::array<Rotation,24> result{};
    Point axes{0,1,2};
    int i=0;
    do {
        const int inv=(axes[0]>axes[1])+(axes[0]>axes[2])+(axes[1]>axes[2]);
        const int parity=inv%2?-1:1;
        for(int x:{-1,1}) for(int y:{-1,1})
            result[i++]={axes,{x,y,parity*x*y}};
    } while(std::next_permutation(axes.begin(),axes.end()));
    return result;
}
inline constexpr auto all=proper();
}
