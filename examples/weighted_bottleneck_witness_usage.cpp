#include <array>
#include <iostream>
#include <vector>
#include "../library/dsu.hpp"
#include "../library/weighted-bottleneck-witness.hpp"
int main() {
    struct Edge {int from,to,id;};
    const std::vector<Edge> g{{0,1,0},{1,2,1},{0,2,2}};
    const std::array<int,3> weights{2,6,9};
    Dsu ds(3);
    BidirBottleneck<3,3> f(g);
    f.set_groups(ds);
    int witness;
    const int threshold=f.cutoff(weights,-1,0,2,&witness);
    std::cout<<threshold<<' '<<witness<<'\n'; // 6 1
}
