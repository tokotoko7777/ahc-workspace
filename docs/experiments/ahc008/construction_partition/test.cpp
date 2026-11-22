#pragma GCC optimize("O3")
#include "route_makespan.hpp"
#include <cassert>
#include <functional>
#include <random>
#include <iostream>
using namespace route_makespan;
int main(){
    std::mt19937 random(7713251);
    for (int tt=0;tt<3000;++tt) {
        int n=1+random()%8,m=1+random()%5;
        std::vector<Cost> work(n), walk(n);std::vector<int>x(n), y(n);
        for(int j=0;j<n;++j) {
            x[j]=static_cast<int>(random()%11);y[j]=static_cast<int>(random()%11);
            work[j]=random()%4;
            if(j)walk[j-1]=std::abs(x[j]-x[j-1])+std::abs(y[j]-y[j-1])+random()%3;
        }
        std::vector<std::vector<Cost>> t(m,std::vector<Cost>(n));
        for(auto &row:t){
            int a=random()%11,b=random()%11;
            for(int i=0;i<n;++i) row[i]=std::abs(a-x[i])+std::abs(b-y[i]);
        }
        std::vector<std::vector<Cost>> memo(1<<m,std::vector<Cost>(n,-1));
        auto brute=[&](auto&& self,int k,int i)->Cost{
            if(i==n)return 0;
            auto &v=memo[k][i]; if(v!=-1)return v;
            v=10000000;
            for(int a=0;a<m;++a)if(!(k>>a&1)){
                Cost dur=t[a][i];
                for(int j=i;j<n;++j){
                    dur+=work[j];
                    v=std::min(v,std::max(dur,self(self,k|1<<a,j+1)));
                    dur+=walk[j];
                }
            }
            return v;
        };
        auto answer=solve(work,walk,t);
        assert(answer.makespan==brute(brute,0,0));
        int l=0,used=0;Cost longest=0;
        for(auto [h,a,b]:answer.segments){
            assert(a==l && b>a && b<=n && !(used>>h&1));
            used|=1<<h;l=b;
            Cost d=t[h][a];
            for(int k=a;k<b;++k)d+=work[k]+(k+1<b?walk[k]:0);
            longest=std::max(longest,d);
        }
        assert(l==n && longest==answer.makespan);
    }
    std::cout<<"3000 exhaustive small-state comparisons passed\n";
}
