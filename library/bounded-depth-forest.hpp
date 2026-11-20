#pragma once
#include <cassert>
#include <vector>

// Incrementally reparent subtrees of a forest with a depth bound. Depth of
// roots is zero. Objective is sum(weight[v] * (1 + depth[v])).
// Does not store the underlying graph: the caller decides which edges exist.
class BoundedDepthForest {
    int cap_;
    std::vector<int> parent_,depth_,first_,next_,previous_,buffer_;
    std::vector<long long> weights_;
    long long score_=0;
    void attach(int v,int p) {
        parent_[v]=p;
        previous_[v]=-1;
        next_[v]=p<0 ? -1 : first_[p];
        if(next_[v]>=0) previous_[next_[v]]=v;
        if(p>=0) first_[p]=v;
    }
    void detach(int v) {
        const int l=previous_[v],r=next_[v],p=parent_[v];
        if(l>=0) next_[l]=r;
        else if(p>=0) first_[p]=r;
        if(r>=0) previous_[r]=l;
    }
public:
    BoundedDepthForest(const std::vector<int>& parents,const std::vector<long long>& weights,
                       int cap)
        : cap_(cap),parent_(parents.size(),-1),depth_(parents.size()),
          first_(parents.size(),-1),next_(parents.size(),-1),previous_(parents.size(),-1),
          weights_(weights) {
        const int n=parents.size();
        assert(cap>=0 && int(weights.size())==n);
        buffer_.reserve(n);
        for(int i=0;i<n;++i) {
            const int p=parents[i];
            assert(-1<=p && p<n && p!=i);
            attach(i,p);
            if(p<0) buffer_.push_back(i);
        }
        for(int i=0;i<int(buffer_.size());++i) {
            const int v=buffer_[i];
            assert(depth_[v]<=cap_);
            score_+=weights_[v]*(depth_[v]+1);
            for(int u=first_[v];u>=0;u=next_[u]) {
                depth_[u]=depth_[v]+1;
                buffer_.push_back(u);
            }
        }
        assert(int(buffer_.size())==n);
    }
    const std::vector<int>& parents() const {return parent_;}
    const std::vector<int>& depths() const {return depth_;}
    long long score() const {return score_;}

    // O(size of moved subtree); rejects no-ops, cycles and excessive depth.
    // Accept receives the exact signed change only if this move is feasible.
    template<class Accept>
    bool try_reparent(int v,int p,Accept accept) {
        const int n=parent_.size();
        assert(0<=v && v<n && -1<=p && p<n);
        if(p==v || parent_[v]==p) return false;
        const int delta=(p<0 ? 0 : depth_[p]+1)-depth_[v];
        buffer_.clear();
        buffer_.push_back(v);
        long long weight=0;
        for(int i=0;i<int(buffer_.size());++i) {
            const int u=buffer_[i];
            if(u==p || depth_[u]+delta>cap_) return false;
            weight+=weights_[u];
            for(int child=first_[u];child>=0;child=next_[child]) buffer_.push_back(child);
        }
        const long long gain=weight*delta;
        if(!accept(gain)) return false;
        detach(v);attach(v,p);
        for(int u:buffer_) depth_[u]+=delta;
        score_+=gain;
        return true;
    }
};
