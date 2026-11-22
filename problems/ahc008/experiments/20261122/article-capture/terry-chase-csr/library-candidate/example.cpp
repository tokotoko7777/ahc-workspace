#include "csr_dinic.hpp"
#include <array>
#include <iostream>

int main() {
    using Flow=ahc::CsrDinic;
    const std::array edges{Flow::Arc{0,1,3},Flow::Arc{0,2,2},
                           Flow::Arc{1,3,2},Flow::Arc{2,3,2}};
    Flow g(4,edges);
    std::cout<<g.solve(0,3)<<'\n';  // 4
    for(int v=0;v<4;++v)
        if(g.on_source_side(v)) std::cout<<v<<' ';  // 0 1
    std::cout<<'\n';
}
