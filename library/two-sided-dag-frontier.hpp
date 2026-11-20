#pragma once
#include <cassert>
#include <vector>

// Remove one vertex at a time from a DAG, either with no live incoming edge
// (low end) or with no live outgoing edge (high end).
class TwoSidedDagFrontier {
    std::vector<std::vector<int>> parents_,children_;
    std::vector<int> in_,out_;
    std::vector<unsigned char> live_;
public:
    explicit TwoSidedDagFrontier(const std::vector<std::vector<int>>& edges)
        : parents_(edges.size()),children_(edges),in_(edges.size()),
          out_(edges.size()),live_(edges.size(),1) {
        for(int a=0;a<int(edges.size());++a) for(int b:edges[a]) {
            assert(a!=b && 0<=b && b<int(edges.size()));
            parents_[b].push_back(a); ++in_[b]; ++out_[a];
        }
    }
    bool live(int v) const {return live_[v];}
    bool ready(int v,bool high) const {
        return live_[v] && (high ? out_[v] : in_[v])==0;
    }
    void remove(int v,bool high) {
        assert(ready(v,high));
        live_[v]=0;
        for(int p:parents_[v]) --out_[p];
        for(int c:children_[v]) --in_[c];
    }
};
