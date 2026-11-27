#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace ahc {
// Minimum combined cost of a forward path reaching a specified vertex at
// layer meet, and a reverse-swept path returning to goal at layer through.
// for_each_from(u, visit) enumerates possible outgoing vertices. Staying is
// not added implicitly. Cost(t,u,v) must return a nonnegative int64_t
// strictly less than infinity, or infinity for a forbidden edge.
// Caller can enforce time-varying obstacles and mandatory actions with cost.
struct TimeLayerJoin {
    static constexpr std::int64_t infinity = std::numeric_limits<std::int64_t>::max()/4;
    std::vector<std::int64_t> arrive, onward;
    [[nodiscard]] std::int64_t via(int vertex) const {
        auto a=arrive.at(vertex), b=onward.at(vertex);
        return a>=infinity-b ? infinity : a+b;
    }
};
template <class ForEachFrom, class Cost>
TimeLayerJoin time_layer_joins(int vertices,int from,int meet,int through,int source,int goal,
                             ForEachFrom&& for_each_from,Cost&& cost) {
    using I=std::int64_t;
    constexpr I forbidden=TimeLayerJoin::infinity;
    if(vertices<=0||source<0||source>=vertices||goal<0||goal>=vertices||!(0<=from&&from<=meet&&meet<=through))
        throw std::invalid_argument("invalid time layer bounds");
    auto plus=[](I a,I b){return a>=forbidden-b ? forbidden : a+b;};
    TimeLayerJoin r{std::vector<I>(vertices,forbidden),std::vector<I>(vertices,forbidden)};
    std::vector<I> tmp(vertices,forbidden);
    r.arrive[source]=0;
    for(int t=from;t<meet;++t){
        std::fill(tmp.begin(),tmp.end(),forbidden);
        for(int u=0;u<vertices;++u)if(r.arrive[u]!=forbidden)
            for_each_from(u,[&](int v){
                const I c=cost(t,u,v);
                tmp[v]=std::min(tmp[v],plus(r.arrive[u],c));
            });
        tmp.swap(r.arrive);
    }
    r.onward[goal]=0;
    for(int t=through;t>meet;--t){
        std::fill(tmp.begin(),tmp.end(),forbidden);
        for(int u=0;u<vertices;++u)
            for_each_from(u,[&](int v){
                if(r.onward[v]!=forbidden){
                    const I c=cost(t-1,u,v);
                    tmp[u]=std::min(tmp[u],plus(c,r.onward[v]));
                }
            });
        tmp.swap(r.onward);
    }
    return r;
}
}
