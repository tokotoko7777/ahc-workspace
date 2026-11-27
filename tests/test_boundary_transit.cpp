#include "library/boundary-transit.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <queue>
#include <random>
#include <utility>
#include <vector>

int main() {
    constexpr int R=8,C=8,V=R*C,INF=1'000'000;
    using BT=ahc::BoundaryTransit<V>;
    auto grid=[](int a){
        const int r=a/C,c=a%C;
        return std::array{r>0?a-C:-1,c+1<C?a+1:-1,r+1<R?a+C:-1,c>0?a-1:-1};
    };
    // Third exterior port cannot be used as a patch-interior hop.
    {
        const BT bt(BT::Mask{}.set(9).set(11),grid);
        auto sig=bt.signature({},grid);
        auto port=[&](int c){return int(std::find(bt.ports().begin(),bt.ports().end(),c)-bt.ports().begin());};
        const int a=port(1),b=port(3),mid=port(10);
        assert(sig[b*(b-1)/2+a]==BT::unreachable);
        assert(sig[mid*(mid-1)/2+a]==2 && sig[mid*(mid-1)/2+b]==2);
        assert(bt.signature(BT::Mask{}.set(10),grid)==sig);
    }
    std::mt19937 rng(23161);
    int comparisons=0, nested=0, removed=0;
    for (int it=0;it<1250;++it) {
        BT::Mask patch,first,second;
        // Connected rectangles and occasionally ragged patches.
        int l=rng()%C,d=rng()%R,w=1+rng()%(C-l),h=1+rng()%(R-d);
        for (int r=d;r<d+h;++r) for(int c=l;c<l+w;++c)
            if(it%3 || rng()%4)patch.set(r*C+c);
        for (int i=0;i<V;++i) if(patch[i]) {
            if(rng()%5==0)first.set(i);
            if(rng()%5==0)second.set(i);
        }
        second |= first;
        const BT bt(patch,grid);
        const auto f=bt.signature(first,grid),s=bt.signature(second,grid);
        assert(f.size()==s.size());
        for(std::size_t k=0;k<f.size();++k){assert(f[k]<=s[k]);++nested;}
        for (int pass=0;pass<2;++pass) {
            BT::Mask outside,inside=pass?second:first;
            const auto& sig=pass?s:f;
            if(pass)for(int x=0;x<V;++x)if(!patch[x]&&rng()%4==0)outside.set(x);
            for(int p:bt.ports())if(outside[p])++removed;
            assert(bt.signature(inside|outside,grid)==sig);
            std::array<std::vector<std::pair<int,int>>,V> graph;
            for (int i=0;i<V;++i) if(!patch[i]&&!outside[i])
                for(int v:grid(i))if(v>=0&&!patch[v]&&!outside[v])graph[i].push_back({v,1});
            const auto& ports=bt.ports();
            for(std::size_t i=1;i<ports.size();++i)for(std::size_t j=0;j<i;++j){
                const int a=ports[i],b=ports[j],cost=sig[i*(i-1)/2+j];
                if(!outside[a]&&!outside[b]&&cost<BT::unreachable){
                    graph[a].push_back({b,cost});graph[b].push_back({a,cost});
                }
            }
            const auto walls=outside|inside;
            for (int src=0;src<V;src+=3)if(!patch[src]&&!outside[src]) {
                std::array<int,V> expected,got;
                expected.fill(INF);got.fill(INF);
                std::queue<int> q;q.push(src);expected[src]=0;
                while(!q.empty()){
                    int p=q.front();q.pop();
                    for(int v:grid(p))if(v>=0&&!walls[v]&&expected[v]==INF){
                        expected[v]=expected[p]+1;q.push(v);
                    }
                }
                using Entry=std::pair<int,int>;
                std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> pq;
                got[src]=0;pq.push({0,src});
                while(!pq.empty()){
                    auto [d,p]=pq.top();pq.pop();
                    if(d!=got[p])continue;
                    for(auto [v,w]:graph[p])if(got[v]>d+w){got[v]=d+w;pq.push({got[v],v});}
                }
                for(int j=0;j<V;++j)if(!patch[j]&&!outside[j]){assert(expected[j]==got[j]);++comparisons;}
            }
        }
    }
    std::cout<<"full BFS checks: "<<comparisons<<"; monotone port pairs: "<<nested
             <<"; port deletions: "<<removed<<'\n';
}
