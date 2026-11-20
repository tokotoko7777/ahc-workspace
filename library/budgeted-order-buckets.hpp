#pragma once
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

// Partial comparison ordering. All values in earlier buckets are <= all
// values in later buckets. A split is atomic with respect to the budget.
template<class Item,class Compare,class Rng>
auto budgeted_order_buckets(std::vector<Item> items,int budget,Compare compare,Rng& rng) {
    struct Bucket {std::vector<Item> items;bool settled;};
    std::vector<Bucket> groups;
    if(!items.empty()) groups.push_back({std::move(items),false});
    while(budget>0) {
        int best=-1;std::size_t largest=1;
        for(int i=0;i<int(groups.size());++i) {
            const auto& g=groups[i];
            if(!g.settled && g.items.size()>largest && g.items.size()<=std::size_t(budget)+1)
                best=i,largest=g.items.size();
        }
        if(best<0) break;
        auto chosen=std::move(groups[best].items);
        const int pi=rng()%chosen.size();
        Bucket low{{},false}, equal{{chosen[pi]},true}, high{{},false};
        for(int i=0;i<int(chosen.size());++i) if(i!=pi) {
            const auto sign=compare(chosen[i],chosen[pi]);
            --budget;
            (sign<0?low:sign>0?high:equal).items.push_back(std::move(chosen[i]));
        }
        auto at=groups.erase(groups.begin()+best);
        if(!high.items.empty()) at=groups.insert(at,std::move(high));
        at=groups.insert(at,std::move(equal));
        if(!low.items.empty()) groups.insert(at,std::move(low));
    }
    std::vector<std::vector<Item>> out;
    out.reserve(groups.size());
    for(auto& g:groups) out.push_back(std::move(g.items));
    return out;
}
