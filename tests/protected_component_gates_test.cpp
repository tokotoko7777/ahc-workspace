#include "../library/protected-component-gates.hpp"
#include <array>
#include <bitset>
#include <cassert>
#include <random>

int main(){
    constexpr int N=20, W=5;
    std::array<std::array<int,4>,N> nb;
    for(int p=0;p<N;p++)nb[p]={p>=W?p-W:-1,p+W<N?p+W:-1,p%W?p-1:-1,p%W+1!=W?p+1:-1};
    using Bits=std::bitset<N>;
    auto reach=[&](Bits walls,int seed){
        Bits got;got.set(seed);
        for(bool changed=true;changed;){
            Bits old=got;
            for(int v=0;v<N;++v)if(old[v])
                for(int x:nb[v])if(x>=0&&!walls[x])got.set(x);
            changed=old!=got;
        }
        return got;
    };
    int checked=0;
    std::mt19937 rng(3530041);
    for(int rep=0;rep<2350;++rep){
        Bits wall,open,pets;
        std::array<int,7> choices{};
        int max=0;
        for(int i=0;i<N;++i){
            if(rng()%2==0){
                wall.set(i);
                if(max<(int)choices.size()&&rng()%3!=0){open.set(i);choices[max++]=i;}
            }else if(rng()%5==0)pets.set(i);
        }
        const auto need=ahc::required_component_gates(nb,wall,open,pets);
        std::array<Bits,N> reference{};
        for(int i=0;i<N;++i)if(pets[i])reference[i]=reach(wall,i);
        for(unsigned mask=0;mask<(1u<<max);++mask){
            Bits removed;
            for(int i=0;i<max;++i)if(mask>>i&1)removed.set(choices[i]);
            bool equal=true;
            for(int i=0;i<N&&equal;++i)if(pets[i])equal=reach(wall&~removed,i)==reference[i];
            assert(equal==(removed&need).none());
            ++checked;
        }
    }
    assert(checked>100000);
}
