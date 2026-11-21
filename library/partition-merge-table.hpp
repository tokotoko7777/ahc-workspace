#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

// Dense key and join transitions for all set partitions of K vertices.
// Labels are canonical restricted-growth strings; the initial partition has
// K singleton blocks. Construct once per K, then reuse in many small DPs.
template<int K>
struct PartitionMergeTable {
    static_assert(1<=K && K<=7);
    static constexpr int PairCount=K*(K-1)/2;
    using Labels=std::array<std::uint8_t,K>;
    std::vector<Labels> labels;
    std::vector<std::array<std::uint16_t,PairCount>> next;
    std::vector<std::uint8_t> connected;
    int singletons=-1;
    static constexpr int code_space=1<<(3*K);
    static int code(const Labels& key) {
        int x=0;
        for(int v:key) x=(x<<3)|v;
        return x;
    }
    static int pair_id(int a,int b) {
        if(a>b)std::swap(a,b);
        assert(0<=a&&a<b&&b<K);
        return b*(b-1)/2+a;
    }
    PartitionMergeTable() {
        Labels v{};
        auto enumerate=[&](auto&& self,int i,int max_label)->void{
            if(i==K) {
                labels.push_back(v);
                connected.push_back(max_label==0);
                if(max_label==K-1)singletons=int(labels.size())-1;
                return;
            }
            for(int l=0;l<=max_label+1;++l) {
                v[i]=l;
                self(self,i+1,std::max(max_label,l));
            }
        };
        enumerate(enumerate,1,0);
        std::vector<std::int16_t> to_index(code_space,-1);
        for(int i=0;i<int(labels.size());++i) to_index[code(labels[i])]=i;
        next.resize(labels.size());
        for(int state=0;state<int(labels.size());++state) {
            for(int b=1;b<K;++b)for(int a=0;a<b;++a) {
                auto key=labels[state];
                int from=key[b],to=key[a];
                for(auto& value:key) if(value==from)value=to;
                std::array<int,K> compact;
                compact.fill(-1);
                int label=0;
                for(auto& value:key) {
                    if(compact[value]<0)compact[value]=label++;
                    value=compact[value];
                }
                next[state][pair_id(a,b)]=to_index[code(key)];
            }
        }
    }
};
