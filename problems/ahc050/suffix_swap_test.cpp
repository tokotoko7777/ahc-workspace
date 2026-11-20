#include "suffix_swap.hpp"
#include <cassert>
#include <random>

static double brute(int n,std::vector<unsigned char> wall,const std::vector<int>& order) {
    std::vector<double> p(n*n),q(n*n);
    const auto m=order.size();
    for(int x:order) p[x]=1./m;
    double value=0.;
    for(int rock:order) {
        std::fill(q.begin(),q.end(),0.);
        for(int i=0;i<n*n;++i) if(!wall[i]) {
            for(auto [r,c]:std::vector<std::pair<int,int>>{{-1,0},{1,0},{0,-1},{0,1}}) {
                int x=i/n,y=i%n;
                while(0<=x+r && x+r<n && 0<=y+c && y+c<n && !wall[(x+r)*n+y+c])
                    x+=r,y+=c;
                q[x*n+y]+=p[i]*.25;
            }
        }
        q[rock]=0;
        value+=std::accumulate(q.begin(),q.end(),0.);
        p.swap(q);
        wall[rock]=1;
    }
    return value;
}
int main() {
    std::mt19937 rng(5050050);
    for(int rep=0;rep<250;++rep) {
        const int n=2+rng()%4;
        std::vector<unsigned char> walls(n*n);
        std::vector<int> ordering;
        for(int v=0;v<n*n;++v) {
            walls[v]=rng()%4==0;
            if(!walls[v]) ordering.push_back(v);
        }
        if(ordering.size()<3) continue;
        std::shuffle(ordering.begin(),ordering.end(),rng);
        auto copy=ordering;
        const int parity=rep%2;
        for(int t=parity;t+1<int(ordering.size());t+=2) {
            auto trial=copy;
            std::swap(trial[t],trial[t+1]);
            if(brute(n,walls,trial)>brute(n,walls,copy)+1e-10) copy.swap(trial);
        }
        auto result=suffix_swap::sweep(n,walls,ordering,parity,true,suffix_swap::Clock::time_point::max());
        const auto expect=brute(n,walls,copy);
        assert(std::abs(result.expected-expect)<1e-8);
        assert(std::abs(result.expected-brute(n,walls,result.path))<1e-8);
    }
}
