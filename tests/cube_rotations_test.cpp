#include "../library/cube-rotations.hpp"
#include <cassert>
#include <set>
#include <vector>

using cube_rotations::Point;
using Shape = std::vector<Point>;

Shape normalized(Shape a) {
    Point origin=a[0];
    for(const auto& p:a) for(int j=0;j<3;++j) origin[j]=std::min(origin[j],p[j]);
    for(auto& p:a) for(int j=0;j<3;++j) p[j]-=origin[j];
    std::sort(a.begin(),a.end());
    return a;
}

int main() {
    std::set<Point> images;
    Point x{1,0,0},y{0,1,0},z{0,0,1};
    const Shape chiral{{0,0,0},{1,0,0},{1,1,0},{1,1,1}};
    auto mirror=chiral;
    for(auto& p:mirror) p[0]=-p[0];
    const auto mirrored=normalized(mirror);
    for(auto rot:cube_rotations::all) {
        images.insert(rot({1,2,3}));
        auto a=rot(x),b=rot(y),c=rot(z);
        Point cross{a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};
        assert(cross==c);
        auto s=chiral;
        for(auto& p:s) p=rot(p);
        assert(normalized(s)!=mirrored);
    }
    assert(images.size()==24);
    for(auto a:cube_rotations::all) for(auto b:cube_rotations::all)
        assert(images.contains(a(b({1,2,3}))));
}
