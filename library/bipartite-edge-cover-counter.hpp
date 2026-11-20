#pragma once
#include <array>
#include <cassert>
#include <utility>
#include <vector>

// Dynamic incidence coverage: an edge is covered iff one or both endpoints
// have positive activity. Parallel edges are counted separately.
class BipartiteEdgeCoverCounter {
    std::array<std::vector<std::vector<int>>,2> adjacent_;
    std::array<std::vector<int>,2> active_;
    int uncovered_;
public:
    BipartiteEdgeCoverCounter(int left_size,int right_size,
                              const std::vector<std::pair<int,int>>& edges)
        :uncovered_(static_cast<int>(edges.size())) {
        assert(left_size>=0&&right_size>=0);
        adjacent_[0].resize(left_size);adjacent_[1].resize(right_size);
        active_[0].resize(left_size);active_[1].resize(right_size);
        for(auto [a,b]:edges){
            assert(0<=a&&a<left_size&&0<=b&&b<right_size);
            adjacent_[0][a].push_back(b);adjacent_[1][b].push_back(a);
        }
    }
    void add(int side,int vertex,int delta) {
        assert(side==0||side==1);
        assert(0<=vertex&&vertex<static_cast<int>(active_[side].size()));
        int& v=active_[side][vertex];
        assert(v+delta>=0);
        const bool before=v>0;v+=delta;const bool after=v>0;
        if(before==after)return;
        for(int other:adjacent_[side][vertex])if(active_[side^1][other]==0)
            uncovered_+=before?1:-1;
    }
    int activity(int side,int vertex) const {return active_[side][vertex];}
    int uncovered() const {return uncovered_;}
    bool all_covered() const {return !uncovered_;}
};
