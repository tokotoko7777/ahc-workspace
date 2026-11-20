#pragma GCC optimize ("O3")
#include "../../partial-lines.hpp"
#include "../../observed-cycles.hpp"
#include <chrono>
#include <iostream>
#include <map>
int main() {
    int m,k;std::cin>>m>>k;
    std::map<std::string,int> counts;
    for(int i=0;i<m;++i){std::string w;std::cin>>w;++counts[w];}
    std::vector<std::string> words;std::vector<int> freq;
    for(const auto& [w,f]:counts){words.push_back(w);freq.push_back(f);}
    auto c=ahc004::observed_cycles(20,words,freq,28000);
    auto t=std::chrono::steady_clock::now();
    auto p=ahc004::partial_assembled_lines(20,words,k);
    double dur=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();
    std::cout<<dur<<'\n';
    for(const auto& w:c.cycles)std::cout<<"O "<<w.letters<<'\n';
    for(const auto& w:p)if(w.known==20)std::cout<<"P "<<w.letters<<'\n';
}
