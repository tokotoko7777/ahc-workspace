#pragma once

#include "bounded-residual-cut.hpp"
#include <algorithm>
#include <cstddef>

namespace bounded_residual_cut {

// Repeated small-integer-budget queries on one solved residual network.
// Flatten once, copy contiguous capacities per query, and augment only
// until budget+1 units have moved. For large budgets use query() instead.
template<std::integral Capacity>
class TinyBudgetWorkspace {
    struct Edge {int to, rev, next;};
    int n_ = 0, base_size_ = 0;
    std::vector<int> offset_, head_, parent_, queue_;
    std::vector<Edge> edges_;
    std::vector<Capacity> base_, cap_;

    void add(int u,int v,Capacity capacity) {
        if(u==v)return;
        const int e=edges_.size();
        edges_.push_back({v,e+1,head_[u]});
        edges_.push_back({u,e,head_[v]});
        cap_.push_back(capacity);
        cap_.push_back(0);
        head_[u]=e;
        head_[v]=e+1;
    }
    bool visit(int start,int finish,bool backward) {
        std::fill(parent_.begin(),parent_.end(),-1);
        queue_.clear();
        parent_[start]=-2;
        queue_.push_back(start);
        for(std::size_t i=0;i<queue_.size();++i){
            const int u=queue_[i];
            auto edge=[&](int e) {
                int v=edges_[e].to;
                if(cap_[backward?edges_[e].rev:e]>0&&parent_[v]==-1){
                    parent_[v]=e;
                    queue_.push_back(v);
                    return v==finish;
                }
                return false;
            };
            for(int e=offset_[u];e<offset_[u+1];++e)if(edge(e))return true;
            for(int e=head_[u];e!=-1;e=edges_[e].next)if(edge(e))return true;
        }
        return false;
    }

  public:
    // net must represent a completed, feasible s-t flow.
    template<class Network>
    void prepare(const Network& net) {
        n_=net.n;
        offset_.assign(n_+1,0);
        for(int u=0;u<n_;++u)offset_[u+1]=offset_[u]+net.graph[u].size();
        base_size_=offset_.back();
        edges_.clear();base_.clear();
        edges_.reserve(base_size_);base_.reserve(base_size_);
        for(int u=0;u<n_;++u)for(const auto& e:net.graph[u]){
            edges_.push_back({e.to,offset_[e.to]+e.reverse_index,-1});
            base_.push_back(e.capacity);
        }
        head_.resize(n_);
        parent_.resize(n_);
        queue_.reserve(n_);
    }

    std::optional<Result<Capacity>> query(
        int s,int t,Capacity budget,std::span<const std::pair<int,int>> implications,
        bool need_largest=false) {
        if(budget<0||budget==std::numeric_limits<Capacity>::max()||
           s<0||t<0||s>=n_||t>=n_||s==t)
            throw std::invalid_argument("tiny residual cut: invalid budget or terminals");
        const Capacity limit=static_cast<Capacity>(budget+1);
        edges_.resize(base_size_);
        cap_=base_;
        std::fill(head_.begin(),head_.end(),-1);
        for(auto [a,b]:implications) {
            if(a<0||b<0||a>=n_||b>=n_)
                throw std::invalid_argument("tiny residual cut: invalid implication");
            add(a,b,limit);
        }
        Capacity extra=0;
        while(visit(s,t,false)){
            Capacity move=limit-extra;
            for(int v=t;v!=s;v=edges_[edges_[parent_[v]].rev].to)
                move=std::min(move,cap_[parent_[v]]);
            for(int v=t;v!=s;v=edges_[edges_[parent_[v]].rev].to){
                const int e=parent_[v];
                cap_[e]-=move;
                cap_[edges_[e].rev]+=move;
            }
            extra+=move;
            if(extra==limit)return std::nullopt;
        }
        Result<Capacity> r{extra,std::vector<char>(n_),{}};
        for(int v=0;v<n_;++v)r.smallest_source[v]=parent_[v]!=-1;
        if(need_largest){
            visit(t,-1,true);
            r.largest_source.resize(n_);
            for(int v=0;v<n_;++v)r.largest_source[v]=parent_[v]==-1;
        }
        return r;
    }
};

} // namespace bounded_residual_cut
