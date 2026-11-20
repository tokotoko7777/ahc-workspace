#pragma once
#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>

template<class Cost>
struct ClassPairInsertion {
    Cost delta;
    int first_gap,second_gap;
};

// As with route-edge insertion, gap g is between route[g-1] and route[g].
// A consecutive insertion in any one gap is legal. A separated insertion
// requires equal class_of_gap at the two gaps. first and second are ordered.
// The route has both boundary vertices. Assign distinct class IDs per gap
// if only consecutive positions are allowed. O(route size + class_count).
template<class Route,class Classes,class Point,class Distance>
auto best_class_constrained_pair_insertion(const Route& route,const Classes& class_of_gap,
                                            int class_count,const Point& first,const Point& second,
                                            Distance distance) {
    using Cost=std::decay_t<decltype(distance(route[0],first))>;
    assert(route.size()>=2 && class_of_gap.size()==route.size());
    struct BestLeft {Cost delta{};int pos=-1;};
    std::vector<BestLeft> hist(class_count);
    ClassPairInsertion<Cost> best{std::numeric_limits<Cost>::max(),-1,-1};
    auto check=[&](Cost delta,int l,int r) {
        if(best.first_gap<0 || delta<best.delta || (delta==best.delta &&
           (l<best.first_gap || (l==best.first_gap && r<best.second_gap)))) best={delta,l,r};
    };
    const Cost pair=distance(first,second);
    for(int g=1;g<int(route.size());++g) {
        const auto id=class_of_gap[g];
        assert(0<=id && id<class_count);
        auto& old=hist[id];
        const auto& a=route[g-1];const auto& b=route[g];
        const Cost edge=distance(a,b);
        const Cost together=distance(a,first)+pair+distance(second,b)-edge;
        check(together,g,g);
        if(old.pos>=0) check(old.delta+distance(a,second)+distance(second,b)-edge,old.pos,g);
        const Cost value=distance(a,first)+distance(first,b)-edge;
        if(old.pos<0 || value<old.delta) old={value,g};
    }
    return best;
}
