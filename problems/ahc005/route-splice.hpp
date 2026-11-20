#pragma once
#include "../../library/bipartite-edge-cover-counter.hpp"

// Defined after the existing user's Solver declaration. This uses already
// built road IDs and visibility segments, and edits the realized road walk.
static std::string splice_realized_walk(const Solver& map, const std::string& original,
        std::chrono::steady_clock::time_point deadline) {
    std::vector<int> path{map.road_id[map.start_row][map.start_column]};
    int row=map.start_row,col=map.start_column;
    for(char c:original){
        row += (c=='D')-(c=='U');
        col += (c=='R')-(c=='L');
        path.push_back(map.road_id[row][col]);
    }
    int m=int(map.roads.size());
    std::vector<std::pair<int,int>> geometry(m);
    for(int i=0;i<m;++i)geometry[i]={map.horizontal_segment[i],map.vertical_segment[i]};
    BipartiteEdgeCoverCounter coverage(int(map.horizontal_members.size()),int(map.vertical_members.size()),geometry);
    auto visit=[&](int at,int delta){
        auto [h,v]=geometry[at];coverage.add(0,h,delta);coverage.add(1,v,delta);
    };
    for(int x:path)visit(x,1);
    if(!coverage.all_covered())return original;
    std::vector<int> prefix;
    auto rebuild=[&](){
        prefix.assign(path.size(),0);
        for(int i=1;i<int(path.size());++i)prefix[i]=prefix[i-1]+map.enter_cost[path[i]];
    };
    rebuild();
    auto replace=[&](int a,int b,const std::vector<int>& extra,int new_cost){
        if(new_cost>=prefix[b]-prefix[a])return false;
        // For a<b, keep endpoint a. Delete b only if both endpoints are the
        // same cell and the inserted list is empty; otherwise keep endpoint b.
        const bool close=path[a]==path[b]&&extra.empty();
        for(int z=a+1;z<b+close;++z)visit(path[z],-1);
        for(int w:extra)visit(w,1);
        if(!coverage.all_covered()){
            for(int w:extra)visit(w,-1);
            for(int z=a+1;z<b+close;++z)visit(path[z],1);
            return false;
        }
        path.erase(path.begin()+a+1,path.begin()+b+close);
        path.insert(path.begin()+a+1,extra.begin(),extra.end());
        rebuild();
        return true;
    };
    std::vector<int> last(m),dist(m),parent(m);
    std::array<std::vector<int>,10> buckets;
    std::mt19937 rng(0x20aa9e2u);
    // First remove revisiting loops that add no required visibility.
    for(int round=0;round<6;++round) {
        bool changed=false;
        std::fill(last.begin(),last.end(),-1);
        for(int i=0;i<int(path.size());++i) {
            if((i&31)==0&&std::chrono::steady_clock::now()>=deadline)break;
            int prev=last[path[i]];
            if(prev>=0&&replace(prev,i,{},0)){changed=true;break;}
            last[path[i]]=i;
        }
        if(!changed)break;
    }
    // Length-limited random windows can cross arbitrary old waypoint boundaries.
    for(int attempt=0;attempt<4500&&path.size()>=4;++attempt){
        if(std::chrono::steady_clock::now()>=deadline)break;
        int a=int(rng()%(path.size()-2));
        int b=std::min(int(path.size())-1,a+2+int(rng()%(2u<<(rng()%6))));
        int source=path[a],target=path[b],budget=prefix[b]-prefix[a];
        if(source==target){replace(a,b,{},0);continue;}
        const auto& s=map.roads[source]; const auto& t=map.roads[target];
        if(5*(std::abs(t.row-s.row)+std::abs(t.column-s.column))>=budget)continue;
        std::fill(dist.begin(),dist.end(),Solver::INF);
        for(auto& q:buckets)q.clear();
        dist[source]=0;buckets[0].push_back(source);int queued=1;
        for(int d=0;queued&&d<budget;++d){
            auto& q=buckets[d%10];
            while(!q.empty()){
                int u=q.back();q.pop_back();--queued;
                if(dist[u]!=d)continue;
                if(u==target){queued=0;break;}
                for(int v:map.next_road[u]){
                    int den=d+map.enter_cost[v];
                    if(den>=budget||dist[v]<=den)continue;
                    dist[v]=den;parent[v]=u;buckets[den%10].push_back(v);++queued;
                }
            }
        }
        if(dist[target]>=budget)continue;
        std::vector<int> inside;
        for(int u=parent[target];u!=source;u=parent[u])inside.push_back(u);
        std::reverse(inside.begin(),inside.end());
        replace(a,b,inside,dist[target]);
    }
    std::string result;
    for(int i=1;i<int(path.size());++i)result+=map.move_letter(path[i-1],path[i]);
    if(result.empty() || !map.answer_is_valid(result))return original;
    return result;
}
