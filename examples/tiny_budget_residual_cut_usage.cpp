#include "../library/max-flow.hpp"
#include "../library/tiny-budget-residual-cut.hpp"
#include <cassert>
int main() {
    MaxFlow<int> net(4);
    net.add_edge(0,1,3);
    net.add_edge(1,3,4);
    net.add_edge(0,2,2);
    net.add_edge(2,3,4);
    assert(net.flow(0,3)==5);
    bounded_residual_cut::TinyBudgetWorkspace<int> ws;
    ws.prepare(net);
    const std::pair<int,int> keep1[]={{0,1}}, keep2[]={{0,2}};
    assert(ws.query(0,3,1,keep1)->extra==1);
    assert(!ws.query(0,3,1,keep2));
    assert(ws.query(0,3,2,keep2)->extra==2);
    assert(ws.query(0,3,1,keep1)->extra==1);
}
