#pragma once
#include <cassert>
#include <utility>
#include <vector>

// Each weighted item pays once as soon as both its sides are covered.
// An event is 2*item+side, where side is 0 or 1. Coverage only increases.
class PairedCoverageReward {
    std::vector<long long> weight_;
    std::vector<unsigned char> live_,staged_;
    std::vector<int> touched_;
    long long value_=0;
public:
    explicit PairedCoverageReward(std::vector<long long> weights)
        : weight_(std::move(weights)),live_(weight_.size()),staged_(weight_.size()) {
        touched_.reserve(weight_.size());
    }
    long long value() const {return value_;}
    void add(const std::vector<int>& events) {
        for(int e:events) {
            assert(0<=e && e/2<int(live_.size()));
            auto& x=live_[e/2];
            if(x==3) continue;
            x|=1<<(e&1);
            if(x==3) value_+=weight_[e/2];
        }
    }
    long long delta(const std::vector<int>& events) {
        long long result=0;
        touched_.clear();
        for(int e:events) {
            assert(0<=e && e/2<int(live_.size()));
            int j=e/2;
            if(live_[j]==3) continue;
            if(!staged_[j]) touched_.push_back(j);
            staged_[j]|=1<<(e&1);
        }
        for(int i:touched_) {
            if((live_[i]|staged_[i])==3) result+=weight_[i];
            staged_[i]=0;
        }
        return result;
    }
};
