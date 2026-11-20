#pragma once
#include <array>
#include <cassert>
#include <utility>
#include <vector>
#include "dsu.hpp"

// Select edges in the supplied priority order, accepting any that neither
// closes a cycle nor creates degree >2. Returns disjoint paths (isolates too).
// Greedy and inclusion-wise maximal, not maximum total edge weight.
inline std::vector<std::vector<int>> greedy_path_forest(
    int n,const std::vector<std::pair<int,int>>& edges) {
    assert(n>=0);
    Dsu components(n);
    std::vector<int> degree(n);
    std::vector<std::array<int,2>> links(n);
    for(auto [a,b]:edges) {
        assert(0<=a && a<n && 0<=b && b<n);
        if(degree[a]==2 || degree[b]==2 || !components.unite(a,b)) continue;
        links[a][degree[a]++]=b;
        links[b][degree[b]++]=a;
    }
    std::vector<std::vector<int>> paths;
    std::vector<bool> seen(n);
    for(int v=0;v<n;++v) if(!seen[v] && degree[v]<=1) {
        auto& path=paths.emplace_back();
        int prev=-1,cur=v;
        do {
            path.push_back(cur);
            seen[cur]=true;
            int next=-1;
            for(int i=0;i<degree[cur];++i)
                if(links[cur][i]!=prev) next=links[cur][i];
            prev=cur;cur=next;
        } while(cur>=0);
    }
    return paths;
}
