#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <random>
#include <vector>

// Forest certificate: the MSF of any edge subset can replace that subset
// when computing connectivity at all weight thresholds, even after adding
// arbitrary additional edges (including zero-cost contractions).
template<int N, int M, int B, class Edge>
struct SuffixCertificates {
    struct Record {
        std::uint16_t u, v, weight, id;
    };
    static_assert(M < 65536 && N < 32768);
    using Parents = std::array<std::int16_t,N>;
    std::array<std::vector<Record>, (M + B - 1) / B> blocks;

    static int root(Parents& p, int a) {
        while (p[a]>=0) {
            int b=p[a];
            if(p[b]>=0) p[a]=p[b];
            a=b;
        }
        return a;
    }
    static bool join(Parents& p, int a, int b) {
        a=root(p,a); b=root(p,b);
        if(a==b) return false;
        if(p[a]>p[b]) std::swap(a,b);
        p[a]+=p[b]; p[b]=a;
        return true;
    }
    explicit SuffixCertificates(const std::vector<Edge>& sorted) {
        for(int k=0;k<int(blocks.size());++k) {
            int begin=k*B, end=begin+B;
            Parents parent;
            parent.fill(-1);
            auto& cert=blocks[k];
            cert.reserve(B+N-1);
            for(const auto& e:sorted) {
                if(e.id<begin) continue;
                if(e.id<end || join(parent,e.from,e.to)) {
                    cert.push_back({std::uint16_t(e.from), std::uint16_t(e.to),
                                    std::uint16_t(e.cost),std::uint16_t(e.id)});
                }
            }
        }
    }
    int cutoff(Parents parent, int i, int u, int v) const {
        int a=root(parent,u), b=root(parent,v);
        if(a==b) return 0;
        for(const auto& r:blocks[i/B]) {
            if(r.id<=i) continue;
            int x=root(parent,r.u), y=root(parent,r.v);
            if(x==y) continue;
            if(parent[x]>parent[y]) std::swap(x,y);
            parent[x]+=parent[y]; parent[y]=x;
            if(a==y) a=x;
            if(b==y) b=x;
            if(a==b) return r.weight;
        }
        return -1;
    }
};
