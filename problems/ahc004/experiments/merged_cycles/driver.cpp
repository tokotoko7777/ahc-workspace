#pragma GCC optimize ("O3")
#include "../../partial-lines.hpp"
#include <iostream>
int main(){
    int n,m,k;std::cin>>n>>m>>k;
    std::vector<std::string> read(m);
    for(auto& s:read)std::cin>>s;
    for(const auto& s:ahc004::partial_assembled_lines(n,read,k))std::cout<<s.letters<<'\n';
}
