#include "../library/max-flow.hpp"
#include "../library/bounded-residual-cut.hpp"
#include <cassert>
int main() {
    MaxFlow<int> net(3);
    net.add_edge(0,1,3);
    net.add_edge(1,2,4);
    assert(net.flow(0,2)==3);
    const std::pair<int,int> keep[]={{0,1}};
    assert(!bounded_residual_cut::query(net,0,2,0,keep));
    auto test=bounded_residual_cut::query(net,0,2,1,keep,true);
    assert(test&&test->extra==1&&test->smallest_source[1]);
}
