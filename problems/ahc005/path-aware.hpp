#pragma once
#include "../../library/bipartite-edge-cover-counter.hpp"

// Dynamic coverage of the exact recorded shortest walk for each directed
// terminal edge. In particular, reversing an edge does not assume the same
// visibility on a tied shortest path.
struct DirectedPathCoverage {
    using Leg=std::pair<int,int>;
    const Solver& map;
    BipartiteEdgeCoverCounter cells;
    std::unordered_map<uint64_t,std::vector<int>> edge_segments;
    int h;
    static vector<pair<int,int>> incidences(const Solver& map) {
        vector<pair<int,int>> result;
        for(int i=0;i<int(map.roads.size());++i)
            result.emplace_back(map.horizontal_segment[i],map.vertical_segment[i]);
        return result;
    }
    explicit DirectedPathCoverage(const Solver& map_)
        :map(map_),cells(int(map.horizontal_members.size()),int(map.vertical_members.size()),incidences(map)),
         h(int(map.horizontal_members.size())) {}
    const vector<int>& segments(Leg leg) {
        auto [a,b]=leg;
        uint64_t key=(uint64_t(a)<<32)|uint32_t(b);
        auto [it,created]=edge_segments.try_emplace(key);
        if(created) {
            auto& vals=it->second;
            int source=map.terminals[a],x=map.terminals[b];
            for(;;) {
                vals.push_back(map.horizontal_segment[x]);
                vals.push_back(h+map.vertical_segment[x]);
                if(x==source)break;
                x=map.shortest_path_parent[a][x];
                if(x<0)throw std::logic_error("invalid shortest-path entry");
            }
            std::sort(vals.begin(),vals.end());
            vals.erase(std::unique(vals.begin(),vals.end()),vals.end());
        }
        return it->second;
    }
    void change(Leg leg,int delta) {
        for(int x:segments(leg))cells.add(x>=h,x>=h?x-h:x,delta);
    }
    int length(Leg leg) const { return map.distance_between_terminals[leg.first][leg.second]; }
};

static vector<int> path_aware_polish(const Solver& map, vector<int> order,
          chrono::steady_clock::time_point deadline,bool extended,int& accepts) {
    using Leg=DirectedPathCoverage::Leg;
    DirectedPathCoverage coverage(map);
    auto at=[&](int i){return order[i%int(order.size())];};
    for(int i=0;i<int(order.size());++i)coverage.change({at(i),at(i+1)},1);
    if(!coverage.cells.all_covered())return order;
    auto consider=[&](const vector<Leg>& remove,const vector<Leg>& append) {
        int delta=0;
        for(auto p:remove)delta-=coverage.length(p);
        for(auto p:append)delta+=coverage.length(p);
        if(delta>=0)return false;
        for(auto p:remove)coverage.change(p,-1);
        for(auto p:append)coverage.change(p,1);
        if(!coverage.cells.all_covered()){
            for(auto p:append)coverage.change(p,-1);
            for(auto p:remove)coverage.change(p,1);
            return false;
        }
        ++accepts;
        return true;
    };
    mt19937 rng(629911);
    // The default leaves a strictly decreasing integer cost as a simple
    // invariant; this is a short finishing optimization, not an anneal.
    for(int iter=0;iter<12000&&order.size()>2;++iter){
        if(chrono::steady_clock::now()>=deadline)break;
        int z=int(order.size());
        int i=1+int(rng()%(z-1)),k=extended?int(rng()%3):0;
        if(k<=1) {
            int last=i+(k==1&&i+1<z);
            if(last-i+1>=z-1)continue;
            vector<Leg> gone;
            for(int t=i-1;t<=last;++t)gone.push_back({at(t),at(t+1)});
            if(consider(gone,{{at(i-1),at(last+1)}}))
                order.erase(order.begin()+i,order.begin()+last+1);
        } else {
            int j=int(rng()%z);
            if(j==i||j==i-1)continue;
            int a=at(i-1),x=at(i),b=at(i+1),u=at(j),v=at(j+1);
            if(consider({{a,x},{x,b},{u,v}},{{a,b},{u,x},{x,v}})){
                order.erase(order.begin()+i);
                order.insert(order.begin()+j+1-(j>i),x);
            }
        }
    }
    return order;
}
