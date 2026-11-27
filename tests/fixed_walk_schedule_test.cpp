#include <array>
#include <cassert>
#include <cmath>
#include <random>
#include <numeric>
#include "../library/fixed-walk-schedule.hpp"
int main() {
    ahc::WalkScheduleWorkspace w;
    std::mt19937 rng(3813471);
    for (int tc=0; tc<4000; ++tc) {
        const int n=1+rng()%9, start=rng()%n, len=rng()%5;
        std::vector<std::vector<int>> adj(n);
        for (int i=0;i<n;++i) for (int j=0;j<n;++j)
            if(i!=j&&rng()%5==0) adj[i].push_back(j);
        std::vector<int> closed0;
        int initmask=0;
        for (int i=0;i<n;++i) if(i!=start&&rng()%5==0) {
            closed0.push_back(i); initmask|=1<<i;
        }
        std::vector<std::vector<int>> f(len),c(len);
        std::vector<ahc::WalkSchedulePhase> phases;
        for (int t=0;t<len;++t) {
            for (int i=0;i<n;++i) {if(rng()%5==0) f[t].push_back(i);if(rng()%6==0)c[t].push_back(i);}
            phases.push_back({f[t],c[t],int(rng()%4)});
        }
        const auto mask=int(rng()%(1<<n));
        auto brute = [&](auto&& self,int t,int h,int x,int closed)->long double{
            if(t==len) return (mask>>x)&1;
            if(h<0){
                for(int v:f[t]) if(x==v)return 0;
                for(int v:c[t]) {if(x==v)return 0;closed|=1<<v;}
                return self(self,t,phases[t].steps_after,x,closed);
            }
            if(!h)return self(self,t+1,-1,x,closed);
            long double sum=0;int cnt=0;
            for(int v:adj[x])if(!(closed>>v&1)){sum+=self(self,t,h-1,v,closed);++cnt;}
            if(!cnt)return self(self,t,h-1,x,closed);
            return sum/cnt;
        };
        const auto r=w.probability(n,[&](int v)->const auto&{return adj[v];},
                      start,closed0,phases,[&](int v){return (mask>>v)&1;});
        assert(std::abs((long double)r-brute(brute,0,-1,start,initmask))<1e-12);
    }
}
