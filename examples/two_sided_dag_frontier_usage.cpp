#include "../library/two-sided-dag-frontier.hpp"
#include <iostream>

int main() {
    TwoSidedDagFrontier front({{1},{2},{}});
    if(!front.ready(0,false) || !front.ready(2,true)) return 1;
    front.remove(2,true);
    std::cout<<front.ready(1,true)<<'\n';
    return front.ready(1,true)?0:1;
}
