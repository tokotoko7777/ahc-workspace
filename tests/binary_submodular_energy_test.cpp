#include "../library/max-flow.hpp"
#include "../library/binary-submodular-energy.hpp"
#include <random>
#include <cassert>
#include <array>
int main(){
    std::mt19937 rng(123);
    for(int k=0;k<1400;++k){
        int n=int(rng()%8)+1;
        BinarySubmodularEnergy<long long> b(n);
        std::vector<std::pair<int,int>> u;
        std::vector<std::array<int,4>> r;
        for(int i=0;i<n;++i){int a=int(rng()%51)-25,c=int(rng()%51)-25;u.emplace_back(a,c);b.unary(i,a,c);}
        for(int j=0;j+1<n;++j){
            std::array<int,4> a;
            a[0]=int(rng()%61)-30;a[1]=int(rng()%61)-30;a[2]=int(rng()%61)-30;
            a[3]=a[1]+a[2]-a[0]-int(rng()%20);
            r.push_back(a);b.pair(j,j+1,a[0],a[1],a[2],a[3]);
        }
        std::vector<std::array<int,3>> implications;
        for(int e=0; e<n+2; ++e) {
            const int a=int(rng()%unsigned(n)),c=int(rng()%unsigned(n)),w=int(rng()%10);
            implications.push_back({a,c,w});
            b.imply(a,c,w);
        }
        auto value=[&](int m){
            int v=0;
            for(int i=0;i<n;++i)v+=((m>>i)&1)?u[i].second:u[i].first;
            for(int j=0;j+1<n;++j)v+=r[j][(((m>>j)&1)*2)+((m>>(j+1))&1)];
            for(auto [a,b,w]:implications) if((m>>a&1) && !(m>>b&1)) v+=w;
            return v;
        };
        int q=value(0);
        for(int i=1;i<(1<<n);++i)q=std::min(q,value(i));
        auto [z,side]=b.solve();
        int bits=0;
        for(int i=0;i<n;++i)bits|=(int(side[i])<<i);
        assert(q==z&&q==value(bits));
    }
    BinarySubmodularEnergy<long long> b(2);
    try {
        b.pair(0,1,3,0,0,3);
        assert(false);
    } catch(const std::invalid_argument&) {
    }
}
