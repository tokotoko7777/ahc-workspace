#pragma once
#include "../../library/bounded-depth-forest.hpp"
#include <algorithm>
#include <cassert>
#include <vector>

// Before lowering the subtree, redirect the few offending child subtrees
// to outside neighbors or to root status. The compound action is atomic.
template<class Accept>
bool try_depth_repair(BoundedDepthForest& forest,const std::vector<std::vector<int>>& adj,
                      int cap,int v,int p,Accept accept) {
    if(p<0) return false;
    const auto& d=forest.depths();
    const auto& parent=forest.parents();
    const int shift=d[p]+1-d[v];
    if(shift<=0 || d[p]>=cap) return false;
    for(int a=p;a>=0;a=parent[a]) if(a==v) return false;
    const int n=adj.size(), ceiling=cap-shift;
    std::vector<int> order{v}, cuts;
    std::vector<unsigned char> belongs(n);
    belongs[v]=1;
    for(int i=0;i<int(order.size());++i) {
        int a=order[i];
        forest.for_each_child(a,[&](int u) {
            belongs[u]=1;
            order.push_back(u);
            if(d[a]<=ceiling && d[u]>ceiling) cuts.push_back(u);
        });
        if(cuts.size()>4) return false;
    }
    if(cuts.empty()) return false;
    std::vector<std::pair<int,int>> undo;
    auto rollback=[&]() {
        for(auto it=undo.rbegin();it!=undo.rend();++it) {
            bool ok=forest.try_reparent(it->first,it->second,[](long long){return true;});
            assert(ok);
            (void)ok;
        }
    };
    const long long before=forest.score();
    for(int u:cuts) {
        int height=0;
        std::vector<int> sub{u};
        for(int i=0;i<int(sub.size());++i) {
            int a=sub[i];height=std::max(height,d[a]-d[u]);
            forest.for_each_child(a,[&](int b){sub.push_back(b);});
        }
        int to=-1;
        for(int x:adj[u]) if(!belongs[x] && d[x]+1+height<=cap && (to<0 || d[x]>d[to]))
            to=x;
        int old=parent[u];
        if(!forest.try_reparent(u,to,[](long long){return true;})) {
            rollback();
            return false;
        }
        undo.emplace_back(u,old);
    }
    int previous=parent[v];
    if(!forest.try_reparent(v,p,[](long long){return true;})) {
        rollback();
        return false;
    }
    undo.emplace_back(v,previous);
    if(!accept(forest.score()-before)) {
        rollback();
        assert(forest.score()==before);
        return false;
    }
    return true;
}
