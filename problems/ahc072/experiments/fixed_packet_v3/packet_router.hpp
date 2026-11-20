#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <optional>
#include <vector>

// AHC072 fixed-support whole-packet shortest path. Supports at nests
// must already be stable (and remain immutable when uncovered).
namespace ahc072_packet {
struct Support { int height=0,top=-1; };
struct Jump { int from,base,direction,length; };
template<class Ray>
std::optional<std::vector<Jump>> shortest(int source,const std::vector<int>& word,
        const std::vector<Support>& support,const std::vector<int>& home,Ray ray){
    const int q=word.size(),n=support.size();
    assert(0<q && q<=8 && n==int(home.size()) && source>=0 && source<n);
    for(int i=0;i<n;++i)assert(support[i].height>=0 && support[i].height<=8
        && !(support[i].height && support[i].top==home[i]));
    assert(q+support[source].height<=8);
    assert(word.back()!=home[source]); // packet already stable at construction
    struct Slice {int lo,hi,flip;};
    std::array<std::array<std::array<int,2>,9>,9> ids{};
    std::vector<Slice> slices;
    for(int lo=0;lo<q;++lo)for(int hi=lo+1;hi<=q;++hi)for(int f=0;f<2;++f){
        ids[lo][hi][f]=slices.size();slices.push_back({lo,hi,f});
    }
    const int width=slices.size(),start=source*width+ids[0][q][0];
    std::vector<int> parent(n*width,-1),queue{start};
    std::vector<Jump> via(n*width);
    parent[start]=start;
    for(size_t head=0;head<queue.size();++head){
        const int state=queue[head],cell=state/width;
        const auto slice=slices[state%width];
        const int early_base=support[cell].height;
        for(int d=0;d<4;++d)for(int length=1;length<=early_base+1;++length){
            const int dest=ray(cell,d,length);
            if(dest<0)break;
            if(support[dest].height+slice.hi-slice.lo>8)continue;
            auto [lo,hi,flip]=slice;
            flip^=1;
            // At source, the exposed frozen support is already stable.
            // At destination, reverse the packet and pop matching top.
            while(lo<hi && word[flip?lo:hi-1]==home[dest]){
                if(flip)++lo;else --hi;
            }
            Jump last{cell,early_base,d,length};
            if(lo==hi){
                std::vector<Jump> path{last};
                for(int k=state;k!=start;k=parent[k])path.push_back(via[k]);
                std::reverse(path.begin(),path.end());
                return path;
            }
            const int next=dest*width+ids[lo][hi][flip];
            if(parent[next]>=0)continue;
            parent[next]=state;via[next]=last;queue.push_back(next);
        }
    }
    return std::nullopt;
}
}
