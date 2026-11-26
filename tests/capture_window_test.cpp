#include "../library/capture-window.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

int main(){
    std::mt19937_64 rng(167);
    int tested=0;
    using G=std::vector<std::vector<int>>;
    auto test=[&](int n,int speed,int first,int last,int src,const G& adj,
                  const std::vector<bool>& mask,ahc::CaptureWindow& work){
        long double want=0;
        auto reference=[&](auto&& f,int p,int micro,long double mass)->void{
            if(micro%speed==0&&micro/speed>=first&&mask[p]){want+=mass;return;}
            if(micro==last*speed)return;
            if(adj[p].empty())f(f,p,micro+1,mass);
            else for(int u:adj[p])f(f,u,micro+1,mass/adj[p].size());
        };
        if(speed==0)want=mask[src];
        else reference(reference,src,0,1.L);
        auto g=[&](int i)->const auto&{return adj[i];};
        auto s=[&](int i){return mask[i];};
        const auto got=work.probability(src,speed,first,last,g,s);
        assert(std::abs(got-want)<1e-10);
        ++tested;
    };
    for(int rep=0;rep<500;++rep){
        const int n=1+rng()%9;
        ahc::CaptureWindow work(n);
        G g(n);
        for(auto& row:g)for(int i=0,m=rng()%4;i<m;++i)row.push_back(rng()%n);
        for(int k=0;k<12;++k){
            std::vector<bool> safe(n);
            for(int i=0;i<n;++i)safe[i]=rng()%4==0;
            const int speed=rng()%4,last=rng()%5,begin=rng()%(last+1),src=rng()%n;
            test(n,speed,begin,last,src,g,safe,work);
        }
    }
    std::vector<std::vector<int>> corridor(9);
    for(int i=0;i<9;++i){
        if(i)corridor[i].push_back(i-1);
        if(i+1<9)corridor[i].push_back(i+1);
    }
    ahc::CaptureWindow work(9);
    std::vector<bool> mask(9); mask[2]=mask[4]=true;
    test(9,2,3,8,6,corridor,mask,work);
    std::cout<<tested<<" exhaustive trajectory checks\n";
}
