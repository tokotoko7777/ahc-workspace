#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <span>

// Posterior for one AHC008 dog. State zero is "no current target"; state
// i+1 is human i. Distances and motion are from AFTER the human phase.
class AHC008DogTargetFilter {
public:
    explicit AHC008DogTargetFilter(int humans) : m_(humans) {
        assert(1 <= m_ && m_ <= 10);
        belief_[0] = 1;
    }
    double probability(int person) const { return belief_[person + 1]; }
    double reset_probability() const { return belief_[0]; }

    // dist(i, cell) is shortest distance from human i or -1 if disconnected.
    // seen: 2 cells for directed+basic motion, 1 for basic-only.
    template<class Distance>
    bool update(int start, std::span<const int> open_neighbors, std::span<const int> seen, Distance dist) {
        assert(!open_neighbors.empty() && (seen.size()==1 || seen.size()==2));
        std::array<double,11> out{}, target{};
        std::array<int,10> at{};
        int allowed = 0;
        double reset = belief_[0];
        for (int i=0;i<m_;++i) {
            at[i]=dist(i,start);
            if (at[i]>0) ++allowed, target[i]=belief_[i+1];
            else reset+=belief_[i+1];
        }
        if (allowed==0) {
            if (seen.size()==1) {
                belief_={};
                belief_[0]=1;
                return true;
            }
            return false;
        }
        if (seen.size()!=2) return false;

        const double random_each=reset/allowed;
        for (int i=0;i<m_;++i) {
            if (at[i]<=0) continue;
            if (dist(i,seen[0])!=at[i]-1) continue;
            int first_choices=0;
            for (int next : open_neighbors) first_choices+=dist(i,next)==at[i]-1;
            assert(first_choices>0);
            // Basic second step is uniform and identical for every hidden
            // state conditional on the observed first step, so it cancels.
            const bool finished=dist(i,seen[0])==0 || dist(i,seen[1])==0;
            out[finished?0:i+1]+=(target[i]+random_each)/first_choices;
        }
        double z=0;
        for (double x : out) z+=x;
        if (z==0) return false;
        for (int i=0;i<=m_;++i) belief_[i]=out[i]/z;
        return true;
    }
private:
    int m_;
    std::array<double,11> belief_{};
};
