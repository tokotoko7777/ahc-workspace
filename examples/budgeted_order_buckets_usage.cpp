#include "../library/budgeted-order-buckets.hpp"
#include <iostream>
#include <random>

int main() {
    std::mt19937 rng(1);
    auto buckets=budgeted_order_buckets(std::vector<int>{8,3,9,2,4,6},5,
        [](int a,int b){return (a>b)-(a<b);},rng);
    for(const auto& group:buckets) {
        for(int value:group) std::cout<<value<<' ';
        std::cout<<'\n';
    }
}
