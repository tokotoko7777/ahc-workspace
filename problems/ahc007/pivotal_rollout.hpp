#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>
#include <utility>

// A subset of "pivotal" uncertain future edges stay hidden. The rest of the
// future is fixed per scenario and available as an offline completion forest.
// The terminal forest preserves cost differences induced by zero-cost
// contraction of terminal pairs at all thresholds.
template<int K, int N, int M>
struct PivotalRollout {
    static constexpr int T=2*(K+1), S=1<<K;
    using ProbabilityRow=std::array<double,S>;
    struct NodeEdge { int u,v,w; };
    struct UF {
        std::array<int,T> p;
        UF(){p.fill(-1);}
        int root(int x){return p[x]<0 ? x : p[x]=root(p[x]);}
        bool join(int x,int y) {
            x=root(x); y=root(y);
            if(x==y)return false;
            if(p[x]>p[y])std::swap(x,y);
            p[x]+=p[y];p[y]=x;
            return true;
        }
    };

    std::array<int,T> vertex{};
    std::array<int,K> critical{},lo{},hi{};
    std::vector<ProbabilityRow> tail;
    int tmax;
    template<class Edge>
    PivotalRollout(const std::vector<Edge>& edges,int query,std::array<int,K> choose,int max_weight)
        : critical(choose), tail(max_weight+1), tmax(max_weight) {
        std::sort(critical.begin(),critical.end());
        vertex[0]=edges[query].from;vertex[1]=edges[query].to;
        for(int j=0;j<K;++j) {
            const auto& e=edges[critical[j]];
            vertex[2*j+2]=e.from;vertex[2*j+3]=e.to;
            lo[j]=e.cost;hi[j]=3*e.cost;
        }
        tail.back().fill(0);
        for(int t=tmax-1;t>=0;--t) {
            ProbabilityRow p;
            p[0]=1;
            for(int j=0;j<K;++j) {
                double q=double(std::clamp(t-lo[j]+1,0,hi[j]-lo[j]+1))/(hi[j]-lo[j]+1);
                for(int mask=0;mask<(1<<j);++mask) {
                    p[mask|(1<<j)]=p[mask]*q;
                    p[mask]*=1-q;
                }
            }
            for(int mask=0;mask<S;++mask) tail[t][mask]=tail[t+1][mask]+p[mask];
        }
    }

    // Expected min(no, yes+X) for X uniform over integers [a,b].
    static double reveal(double no,double yes,int a,int b) {
        constexpr double inf=1e12;
        if(yes>=inf) return no;
        if(no>=inf) return yes+(a+b)*0.5;
        const double gap=no-yes;
        if(gap<=a) return no;
        if(gap>=b) return yes+(a+b)*0.5;
        const int last=int(gap);
        const double count=last-a+1;
        return no-count*(gap-(a+last)*0.5)/(b-a+1);
    }

    template<class DSU,class Edge>
    std::pair<double,double> conditional_values(DSU dsu,const std::vector<Edge>& sorted,int i) const {
        std::array<int,N> labels;
        labels.fill(-1);
        std::array<NodeEdge,T-1> forest;
        int n=0;
        for(int j=0;j<T;++j){
            int r=dsu.leader(vertex[j]);
            if(labels[r]<0) labels[r]=j;
            else forest[n++]={labels[r],j,0};
        }
        const std::array<int,K> roots=critical;
        for(const auto& e:sorted){
            if(n==T-1)break;
            if(e.id<=i)continue;
            bool special=false;
            for(int id:roots) special|=id==e.id;
            if(special)continue;
            int a=dsu.leader(e.from),b=dsu.leader(e.to);
            if(a==b)continue;
            if(dsu.parent_or_size[a]>dsu.parent_or_size[b]) std::swap(a,b);
            dsu.parent_or_size[a]+=dsu.parent_or_size[b];dsu.parent_or_size[b]=a;
            if(labels[a]<0) labels[a]=labels[b];
            else if(labels[b]>=0) forest[n++]={labels[a],labels[b],e.cost};
        }
        std::array<double,S> no,yes;
        double sum_cdf=0;
        for(int mask=0;mask<S;++mask) {
            UF d,extended;
            int count=0,count_ext=1;
            extended.join(0,1);
            for(int j=0;j<K;++j) if(mask>>j&1){
                count+=d.join(2*j+2,2*j+3);
                count_ext+=extended.join(2*j+2,2*j+3);
            }
            int need=d.root(0)==d.root(1)?0:tmax;
            int cn=0,cy=0;
            for(int k=0;k<n;++k){
                auto [u,v,w]=forest[k];
                if(d.join(u,v)) {
                    ++count;cn+=w;
                    if(need==tmax && d.root(0)==d.root(1)) need=w;
                }
                if(extended.join(u,v)) ++count_ext,cy+=w;
            }
            no[mask]=count==T-1?double(cn):1e12;
            yes[mask]=count_ext==T-1?double(cy):1e12;
            assert(0<=need && need<=tmax);
            sum_cdf+=tail[need][mask];
        }
        for(int j=K-1;j>=0;--j) {
            for(int mask=0;mask<(1<<j);++mask) {
                no[mask]=reveal(no[mask],no[mask|(1<<j)],lo[j],hi[j]);
                yes[mask]=reveal(yes[mask],yes[mask|(1<<j)],lo[j],hi[j]);
            }
        }
        return {no[0]-yes[0],tmax-sum_cdf};
    }
    template<class DSU,class Edge>
    double correction(DSU dsu,const std::vector<Edge>& sorted,int i) const {
        auto [online,offline]=conditional_values(dsu,sorted,i);
        return online-offline;
    }
};
