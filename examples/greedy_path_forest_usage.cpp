#include "../library/greedy-path-forest.hpp"
#include <iostream>
int main() {
    auto paths=greedy_path_forest(5,{{1,2},{1,3},{1,4},{2,3},{0,3}});
    for(const auto& path:paths) {
        for(int v:path) std::cout<<v<<' ';
        std::cout<<'\n';
    }
}
