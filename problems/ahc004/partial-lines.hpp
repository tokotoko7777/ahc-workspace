#pragma once
#include <algorithm>
#include <bit>
#include <cstdint>
#include <functional>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ahc004 {
struct PartialLine {
    std::string letters; // canonical toroidal rotation, '.' for uncovered cells
    int known = 0;
};

inline std::vector<PartialLine> partial_assembled_lines(
    int n, const std::vector<std::string>& reads, int overlap=5,
    const std::function<bool()>& deadline=[] { return false; }) {
    if (n != 20 || overlap < 4 || overlap > 7 || deadline()) return {};
    constexpr uint64_t P=(uint64_t(1)<<60)-1;
    std::set<std::string> us;
    for (const auto& s: reads)
        if (int(s.size()) >= overlap && int(s.size()) <= n) us.insert(s);
    for (int k: {4,5}) {
        if(deadline()) return {};
        const int mask=(1<<(3*k))-1,sz=mask+1;
        std::vector<uint8_t> in(sz),out(sz),seen(sz);
        for (const auto& s:reads) {
            int w=0;
            for (int i=0;i<int(s.size());++i) {
                int c=s[i]-'A';
                if(i>=k) {
                    out[w]|=1<<c;
                    in[((w<<3)|c)&mask]|=1<<(w>>(3*(k-1)));
                }
                w=((w<<3)|c)&mask;
            }
        }
        auto walk=[&](int a,int c) {
            const int start=a;
            std::string s(k,'A');
            for(int i=0;i<k;++i)s[i]+= (a>>(3*(k-i-1)))&7;
            for(;;) {
                if(seen[a]>>c&1)break;
                seen[a]|=1<<c;
                s+=char('A'+c);
                a=((a<<3)|c)&mask;
                if(int(s.size())==20+k) {
                    if(a!=start)return;
                    s.resize(20);
                    break;
                }
                if(std::popcount(in[a])!=1 || std::popcount(out[a])!=1)break;
                c=std::countr_zero(out[a]);
            }
            if(int(s.size())>=overlap && int(s.size())<=20)us.insert(std::move(s));
        };
        for(int pass=0;pass<2;++pass)
            for(int a=0;a<sz;++a) {
                if(!(a&1023)&&deadline())return {};
                if(pass==0 && std::popcount(in[a])==1 && std::popcount(out[a])==1)continue;
                unsigned cs=out[a]&~seen[a];
                while(cs) {
                    int c=std::countr_zero(cs); cs&=cs-1; walk(a,c);
                }
            }
    }
    std::vector<std::string> s(us.begin(),us.end());
    std::sort(s.begin(),s.end(),[](const auto& a,const auto& b){
        return a.size()!=b.size()?a.size()>b.size():a<b;
    });
    std::unordered_map<int,std::vector<std::pair<int,int>>> bucket;
    const int K=(1<<(3*overlap))-1,N=s.size();
    for(int a=0;a<N;++a) {
        int w=0;
        for(int i=0;i<int(s[a].size());++i) {
            w=((w<<3)|(s[a][i]-'A'))&K;
            if(i+1>=overlap)bucket[w].emplace_back(a,i+1-overlap);
        }
    }
    std::unordered_map<int,int> cnt;
    for(const auto& [w,v]:bucket) {
        if(v.size()>32)continue;
        for(int i=0;i<int(v.size());++i)
            for(int j=i+1;j<int(v.size());++j)
                if(v[i].first!=v[j].first) {
                    int a=v[i].first,b=v[j].first;
                    int d=(v[i].second-v[j].second+20)%20;
                    ++cnt[(a*N+b)*20+d];
                }
    }
    std::vector<std::pair<int,int>> edges;
    for(const auto& [e,c]:cnt)edges.emplace_back(-c,e);
    std::sort(edges.begin(),edges.end());
    std::vector<int> par(N),off(N);
    std::iota(par.begin(),par.end(),0);
    std::vector<uint64_t> line(N),known(N);
    for(int a=0;a<N;++a) {
        known[a]=(uint64_t(1)<<(3*s[a].size()))-1;
        for(int i=0;i<int(s[a].size());++i)line[a]|=uint64_t(s[a][i]-'A')<<(3*i);
    }
    auto root=[&](int a) {
        int d=0;
        while(a!=par[a]){d+=off[a];a=par[a];}
        return std::pair(a,d%20);
    };
    auto shift=[&](uint64_t w,int d) {
        return d?((w<<(3*d))|(w>>(60-3*d)))&P:w;
    };
    int checks=0;
    for(auto [neg,e]:edges) {
        if((checks++&255)==0&&deadline())return {};
        int a=(e/20)/N,b=(e/20)%N,d=e%20;
        const auto [ra,pa]=root(a);
        const auto [rb,pb]=root(b);
        if(ra==rb)continue;
        d=(d+pa-pb+20)%20;
        const uint64_t w=shift(line[rb],d),m=shift(known[rb],d);
        if((line[ra]^w)&known[ra]&m)continue;
        line[ra]|=w;known[ra]|=m;
        par[rb]=ra;off[rb]=d;
    }
    std::set<std::string> cand;
    for(int a=0;a<N;++a)if(a==par[a]) {
        std::string w(20,'.');
        for(int i=0;i<20;++i)
            if((known[a]>>(3*i))&7)w[i]='A'+((line[a]>>(3*i))&7);
        std::string best=w;
        for(int i=1;i<20;++i) best=std::min(best,w.substr(i)+w.substr(0,i));
        cand.insert(std::move(best));
    }
    std::vector<PartialLine> result;
    for(const auto& w:cand)result.push_back({w,20-int(std::count(w.begin(),w.end(),'.'))});
    std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){
        return a.known!=b.known?a.known>b.known:a.letters<b.letters;
    });
    return result;
}
} // namespace ahc004
