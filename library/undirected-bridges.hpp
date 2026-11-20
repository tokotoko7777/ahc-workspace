#pragma once
#include <algorithm>
#include <utility>
#include <vector>

// Return one flag per edge. Works for disconnected graphs, parallel edges
// and self-loops. Vertices must be in [0, n); uses recursive DFS.
inline std::vector<bool> undirected_bridges(int n,const std::vector<std::pair<int,int>>& edges) {
    std::vector<std::vector<std::pair<int,int>>> adj(n);
    for(int id=0;id<int(edges.size());++id) {
        auto [a,b]=edges[id];
        adj[a].push_back({b,id});
        adj[b].push_back({a,id});
    }
    std::vector<int> discovery(n,-1),low(n);
    std::vector<bool> bridge(edges.size());
    int time=0;
    auto dfs=[&](auto&& self,int a,int parent)->void {
        discovery[a]=low[a]=time++;
        for(auto [b,id]:adj[a]) {
            if(id==parent) continue;
            if(discovery[b]<0) {
                self(self,b,id);
                low[a]=std::min(low[a],low[b]);
                bridge[id]=low[b]>discovery[a];
            } else low[a]=std::min(low[a],discovery[b]);
        }
    };
    for(int a=0;a<n;++a) if(discovery[a]<0) dfs(dfs,a,-1);
    return bridge;
}
