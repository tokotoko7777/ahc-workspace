#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

// Weighted sum of the age since each vertex was visited, over a whole
// period. route contains one vertex at each tick, without duplicating the
// initial vertex at the end. A missing required visit gives nullopt.
class CyclicVisitEnergy {
    std::vector<int> first_,last_;
    static std::int64_t triangles(std::int64_t gap) {return gap*(gap-1)/2;}
public:
    explicit CyclicVisitEnergy(int n):first_(n),last_(n){}
    std::optional<std::int64_t> numerator(std::span<const int> route,
                                          std::span<const int> weight) {
        std::fill(first_.begin(),first_.end(),-1);
        std::int64_t sum=0;
        for(int i=0;i<int(route.size());++i) {
            int p=route[i];
            if(first_[p]<0) first_[p]=i;
            else sum+=weight[p]*triangles(i-last_[p]);
            last_[p]=i;
        }
        for(int p=0;p<int(first_.size());++p) {
            if(first_[p]<0) return std::nullopt;
            sum+=weight[p]*triangles(static_cast<std::int64_t>(route.size())+first_[p]-last_[p]);
        }
        return sum;
    }
};
