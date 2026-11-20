#pragma once
#include <cassert>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

// Undirected additive cost when both ends of an interaction share a group.
// Parallel edges are allowed, as are signed weights. No self edges.
class SparseGroupConflicts {
public:
    using Weight=std::int64_t;
private:
    std::vector<std::vector<std::pair<int,Weight>>> adj_;
public:
    explicit SparseGroupConflicts(int n):adj_(n) { assert(n>=0); }
    void add(int a,int b,Weight weight) {
        assert(a!=b);
        adj_.at(a).emplace_back(b,weight);
        adj_.at(b).emplace_back(a,weight);
    }
    Weight cost(std::span<const int> label) const {
        Weight result=0;
        for(int a=0;a<int(adj_.size());++a) for(auto [b,w]:adj_[a])
            if(a<b && label[a]==label[b]) result+=w;
        return result;
    }
    Weight move_delta(std::span<const int> label,int a,int group) const {
        return move_delta_except(label,a,group,-1);
    }
    Weight swap_delta(std::span<const int> label,int a,int b) const {
        if(label[a]==label[b]) return 0;
        return move_delta_except(label,a,label[b],b)+move_delta_except(label,b,label[a],a);
    }
private:
    Weight move_delta_except(std::span<const int> label,int a,int group,int except) const {
        Weight result=0;
        for(auto [b,w]:adj_[a]) if(b!=except)
            result+=w*((label[b]==group)-(label[b]==label[a]));
        return result;
    }
};
