#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <vector>

// Bidirectional bottleneck Dijkstra on the future graph. Accepted vertices
// are supplied as disjoint zero-cost groups.
// Edge.from/to in [0,N), distinct IDs in [0,M); edge weights in [0,MAX_W].
// Call set_groups once (and again after the contraction changes). Passing a
// prefix index i disables IDs <= i. Unreachable: -1; same group: 0.
// The optional witness is a maximum-weight edge on one optimal path (ties
// are arbitrary), or -1 when the answer is unreachable or within a group.
template<int N,int M,int MAX_W=4095>
struct BidirBottleneck {
    static constexpr int W=(MAX_W+64)/64;
    struct Arc { std::uint16_t id, dest; };
    std::array<int,N+1> offsets{};
    std::array<Arc,2*M> arcs{};
    std::array<int,N> comp{},head{},next{};

    template<class Edge>
    explicit BidirBottleneck(const std::vector<Edge>& e) {
        for(auto x:e) ++offsets[x.from+1],++offsets[x.to+1];
        for(int i=0;i<N;++i) offsets[i+1]+=offsets[i];
        auto pos=offsets;
        for(auto x:e) {
            arcs[pos[x.from]++]={std::uint16_t(x.id),std::uint16_t(x.to)};
            arcs[pos[x.to]++]={std::uint16_t(x.id),std::uint16_t(x.from)};
        }
    }
    template<class DSU>
    void set_groups(DSU& tree) {
        head.fill(-1);
        for(int v=0;v<N;++v) {
            int x=comp[v]=tree.leader(v);
            next[v]=head[x]; head[x]=v;
        }
    }

    struct Queue {
        std::array<std::uint64_t,W> mask{};
        std::uint64_t words=0;
        std::array<std::int16_t,MAX_W+1> first;
        std::array<std::int16_t,N> dist,prev,next,highest;
        static constexpr int inf=MAX_W+1;
        Queue() { dist.fill(inf); }
        void update(int v,int w,int mark=-1) {
            int old=dist[v];
            if(old<=w) return;
            if(old!=inf) {
                const int p=prev[v],n=next[v];
                if(p<0) first[old]=n; else next[p]=n;
                if(n>=0) prev[n]=p;
                if(first[old]<0 && !(mask[old/64]&=~(1ull<<(old%64)))) words&=~(1ull<<(old/64));
            }
            auto& flags=mask[w/64];
            const auto bit=1ull<<(w%64);
            int p=(flags&bit)?first[w]:-1;
            flags|=bit; words|=1ull<<(w/64);
            next[v]=p; prev[v]=-1; first[w]=v; dist[v]=w;highest[v]=mark;
            if(p>=0) prev[p]=v;
        }
        int min() const {
            if(!words) return inf;
            int a=std::countr_zero(words);
            return a*64+std::countr_zero(mask[a]);
        }
        int pop(int w) {
            int v=first[w],n=next[v];
            first[w]=n;
            if(n>=0) prev[n]=-1;
            else if(!(mask[w/64]&=~(1ull<<(w%64)))) words&=~(1ull<<(w/64));
            // Leave distance at its settled value. No later strict decrease
            // is possible, which also keeps update() off removed nodes.
            return v;
        }
    };
    static_assert(0<N && N<32768 && 0<M && M<32768 && 0<=MAX_W && W<=64);

    template<class Weights>
    int cutoff(const Weights& weights,int i,int u,int v,int* marginal=nullptr) const {
        if(comp[u]==comp[v]) {if(marginal)*marginal=-1; return 0;}
        Queue queues[2];
        queues[0].update(comp[u],0);
        queues[1].update(comp[v],0);
        int best=Queue::inf, chosen=-1;
        unsigned iter=0;
        for (;;) {
            const int a=queues[0].min(),b=queues[1].min();
            if(a==Queue::inf || b==Queue::inf || std::max(a,b)>=best)
                {if(marginal) *marginal=chosen; return best==Queue::inf ? -1 : best;}
            const int side=a==b ? ((iter++)&1) : a>b;
            const int w=side==0?a:b;
            auto& mine=queues[side];
            const auto& other=queues[side^1];
            int root=mine.pop(w);
            for(int x=head[root];x>=0;x=next[x]) {
                for(int j=offsets[x];j<offsets[x+1];++j) {
                    const auto [id,dest]=arcs[j];
                    if(id<=i) continue;
                    const int y=comp[dest];
                    if(mine.dist[y]<=w) continue;
                    const int cand=std::max(w,int(weights[id]));
                    if(cand>=mine.dist[y] || cand>=best) continue;
                    const int c=std::max(cand,int(other.dist[y]));
                    const int critical = int(weights[id])>=w ? id:mine.highest[root];
                    if(c<best) {
                        best=c;
                        chosen=cand>=other.dist[y] ? critical:other.highest[y];
                    }
                    if(cand<best) mine.update(y,cand,critical);
                }
            }
        }
    }
};
