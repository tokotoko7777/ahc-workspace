#include <array>
#include <bitset>
#include <cassert>
#include <queue>
#include <random>
#include <vector>
#include "../library/target-limited-bfs.hpp"

int main(){
    constexpr int N=32, INF=999;
    std::mt19937 rng(670159);
    for(int t=0;t<10000;++t){
        std::array<std::vector<int>,N> adj;
        std::bitset<N> open, targets;
        for(int a=0;a<N;++a){
            open[a]=(rng()%4!=0);
            targets[a]=(rng()%6==0);
            adj[a].push_back(-1);
            for(int b=0;b<N;++b)if(rng()%13==0)adj[a].push_back(b);
        }
        int start=rng()%N, radius=rng()%14;
        std::array<int,N> stock,cut;
        stock.fill(INF);stock[start]=0;
        std::queue<int> q;q.push(start);
        while(!q.empty()){
            int a=q.front();q.pop();
            for(int b:adj[a])if(b>=0&&open[b]&&stock[b]==INF){
                stock[b]=stock[a]+1;q.push(b);
            }
        }
        ahc::TargetBfsStats stats;
        ahc::target_limited_bfs(start,radius,INF,targets,cut,
            [&](int a)->const auto&{return adj[a];},[&](int a){return open[a];},&stats);
        bool all=true;
        for(int a=0;a<N;++a)if(targets[a]){
            assert(cut[a]==(stock[a]<=radius?stock[a]:INF));
            if(cut[a]==INF)all=false;
        }
        assert(all==stats.all_targets_found);
    }
}
