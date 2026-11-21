#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <optional>
#include <vector>
#include "../../library/partition-merge-table.hpp"

template<int N,int M>
struct VertexBlock {
    struct Link { int u,v,d,id; };
    struct Arc { int to,edge,next; };

    // The online spanning-tree cost decomposes across vertex-biconnected
    // blocks of the remaining multigraph. Parallel edges retain distinct IDs.
    template<class Edge,class DSU>
    static std::vector<Link> edges_for(DSU dsu,const std::vector<Edge>& graph,int now) {
        std::array<Arc,2*M> arcs;
        std::array<int,N> head,time,low;
        head.fill(-1);time.fill(-1);
        int count=0,timer=0;
        std::vector<Link> links;
        links.reserve(M-now);
        for(int j=now;j<M;++j) {
            const auto e=graph[j];
            int u=dsu.leader(e.from),v=dsu.leader(e.to);
            if(u==v)continue;
            const int id=int(links.size());
            links.push_back({u,v,e.cost,j});
            arcs[count]={v,id,head[u]};head[u]=count++;
            arcs[count]={u,id,head[v]};head[v]=count++;
        }
        assert(links.size()>0 && links.front().id==now);
        std::vector<int> stack,block;
        stack.reserve(links.size());
        auto dfs=[&](auto&& self,int a,int parent)->void {
            time[a]=low[a]=timer++;
            for(int k=head[a];k>=0;k=arcs[k].next) {
                const auto [b,id,next]=arcs[k];
                if(id==parent)continue;
                if(time[b]<0) {
                    stack.push_back(id);
                    self(self,b,id);
                    low[a]=std::min(low[a],low[b]);
                    if(low[b]>=time[a]) {
                        int hi=int(stack.size())-1,lo=hi;
                        while(stack[lo]!=id) --lo;
                        if(std::find(stack.begin()+lo,stack.end(),0)!=stack.end())
                            block.assign(stack.begin()+lo,stack.end());
                        stack.resize(lo);
                    }
                } else if(time[b]<time[a]) {
                    stack.push_back(id);
                    low[a]=std::min(low[a],time[b]);
                }
            }
        };
        dfs(dfs,links.front().u,-1);
        std::sort(block.begin(),block.end());
        std::vector<Link> result;
        result.reserve(block.size());
        for(int id:block)result.push_back(links[id]);
        return result;
    }

    static double reveal(double no,double yes,int d) {
        const double gap=no-yes;
        if(gap<=d)return no;
        if(gap>=3*d)return yes+2*d;
        const int k=int(gap);
        return no-(k-d+1)*(gap-0.5*(d+k))/(2*d+1);
    }
    template<int K>
    static double value(const std::vector<Link>& block) {
        static const PartitionMergeTable<K> table;
        constexpr double INF=1e12;
        std::vector<double> dp(table.labels.size(),INF);
        dp[0]=0;
        for(int j=int(block.size())-1;j>0;--j) {
            const auto e=block[j];
            int pair=table.pair_id(e.u,e.v);
            // Join moves to a lexicographically earlier canonical partition.
            // Reverse update order reads the unchanged next-stage state.
            for(int s=int(dp.size())-1;s>=0;--s) {
                int t=table.next[s][pair];
                assert(t<=s);
                if(t!=s) dp[s]=reveal(dp[s],dp[t],e.d);
            }
        }
        const int pair=table.pair_id(block.front().u,block.front().v);
        const auto no=dp[table.singletons];
        if(no>=INF/2)return INF;
        return no-dp[table.next[table.singletons][pair]];
    }
    template<int MaxVerts,class Edge,class DSU>
    static std::optional<double> limit(DSU tree,const std::vector<Edge>& graph,int i) {
        auto block=edges_for(tree,graph,i);
        if(block.size()==1) return 1e12;
        std::array<int,N> map;
        map.fill(-1);
        int count=0;
        for(auto& edge:block) {
            for(int* v:{&edge.u,&edge.v}) {
                if(map[*v]<0) {
                    if(count==MaxVerts)return std::nullopt;
                    map[*v]=count++;
                }
                *v=map[*v];
            }
        }
        switch(count) {
            case 2: return value<2>(block);
            case 3: return value<3>(block);
            case 4: return value<4>(block);
            case 5: if constexpr(MaxVerts>=5)return value<5>(block); else break;
            case 6: if constexpr(MaxVerts>=6)return value<6>(block); else break;
            case 7: if constexpr(MaxVerts>=7)return value<7>(block); else break;
        }
        return std::nullopt;
    }
};
