#include "time-layer-joins.hpp"
#include <cassert>
#include <iostream>
int main() {
    constexpr auto blocked = ahc::TimeLayerJoin::infinity;
    auto edges=[](int u,auto visit){
        visit(u);
        if(u>0)visit(u-1);
        if(u<2)visit(u+1);
    };
    auto cost=[](int t,int u,int v)->std::int64_t{
        // Cell 2 is available for arrival at turn 2, but closes on turn 3.
        if(t+1==3&&v==2)return blocked;
        return u==v ? 0 : 1;
    };
    auto r=ahc::time_layer_joins(3,0,2,4,0,0,edges,cost);
    assert(r.via(1)==2);
    assert(r.via(2)==4);
    std::cout<<r.via(1)<<' '<<r.via(2)<<'\n';
}
