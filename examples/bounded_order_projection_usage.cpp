#include "../library/bounded-order-projection.hpp"
#include <iostream>
int main() {
    std::vector<double> lengths{80,120};
    project_bounded_order(lengths,{60,100},{150,150},{{0,1}},1);
    std::cout<<lengths[0]<<' '<<lengths[1]<<'\n'; // 100 100
}
