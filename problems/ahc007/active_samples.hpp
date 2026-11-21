#pragma once
#include <array>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <vector>
#include "../../library/monotone-ranked-set.hpp"

template<int N,int M>
struct ActiveKruskal {
    using Parents=std::array<int,N>;
    using Node=std::uint16_t;
    struct Record {
        Node u,v,w,id;
    };
    struct Scenario {
        std::array<Record,M> sorted;
        std::array<Node,M> pos;
        MonotoneRankedSet<M> active;
        template<class E>
        explicit Scenario(const std::vector<E>& e) {
            static_assert(N<65536 && M<65536);
            for(int j=0;j<M;++j) {
                sorted[j]={Node(e[j].from),Node(e[j].to),Node(e[j].cost),Node(e[j].id)};
                pos[e[j].id]=j;
            }
        }
        void erase(int id) {
            active.erase(pos[id]);
        }
        static int root(Parents& p,int x){
            while(p[x]>=0) {
                int a=p[x];
                if(p[a]>=0)p[x]=p[a];
                x=a;
            }
            return x;
        }
        template<bool Witness>
        int cutoff(Parents p,int a,int b) const {
            a=root(p,a);b=root(p,b);
            if(a==b)return 0;
            int answer=-1;
            active.visit_until([&](auto rank) {
                const auto& e=sorted[rank];
                int u=root(p,e.u),v=root(p,e.v);
                if(u==v)return false;
                if(p[u]>p[v])std::swap(u,v);
                p[u]+=p[v];p[v]=u;
                if(a==v)a=u;
                if(b==v)b=u;
                if(a!=b)return false;
                answer=Witness ? ((int(e.w)<<16) | e.id) : int(e.w);
                return true;
            });
            return answer;
        }
    };

    struct Arc { Node v,id; };
    std::array<int,N+1> adj{};
    std::array<Arc,2*M> arcs;
    Parents parent;
    std::array<int,N> head,next,tail;
    std::vector<Scenario> simulations;
    template<class E,class Orders>
    ActiveKruskal(const std::vector<E>& edges,const Orders& orders) {
        for(auto e:edges)++adj[e.from+1],++adj[e.to+1];
        for(int v=0;v<N;++v)adj[v+1]+=adj[v];
        auto end=adj;
        for(auto e:edges) {
            arcs[end[e.from]++]={Node(e.to),Node(e.id)};
            arcs[end[e.to]++]={Node(e.from),Node(e.id)};
        }
        for(int i=0;i<N;++i)head[i]=tail[i]=i;
        parent.fill(-1);next.fill(-1);
        simulations.reserve(orders.size());
        for(auto& cand:orders)simulations.emplace_back(cand);
    }
    void erase(int id) {
        for(auto& s:simulations)s.erase(id);
    }
    void merge(int a,int b,int i) {
        a=Scenario::root(parent,a); b=Scenario::root(parent,b);
        if(a==b)return;
        if(parent[a]>parent[b])std::swap(a,b);
        for(int x=head[b];x>=0;x=next[x])
            for(int j=adj[x];j<adj[x+1];++j)
                if(arcs[j].id>i && Scenario::root(parent,arcs[j].v)==a) erase(arcs[j].id);
        next[tail[a]]=head[b];tail[a]=tail[b];
        parent[a]+=parent[b];parent[b]=a;
    }
};
