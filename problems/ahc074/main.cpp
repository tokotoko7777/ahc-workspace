#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

using Mask=uint64_t;
using Clock=std::chrono::steady_clock;
struct Paper {Mask printed,ink;};
struct Problem {
    int N,K,P,R;
    std::vector<Mask> catalogue;
    std::vector<Paper> papers;
    std::vector<std::vector<int>> group;
    std::vector<int> order;
    std::vector<std::pair<int,int>> lifted;
    const Clock::time_point design_deadline=Clock::now()+std::chrono::milliseconds(680);
    static Mask prefix(int n) {return n>=64 ? ~Mask(0) : (Mask(1)<<n)-1;}
    int match(Mask target,Mask code,int end) const {
        const Mask diff=(target^code)&prefix(end+1);
        return diff ? end-(63-std::countl_zero(diff)) : end+1;
    }
    bool read() {
        if(!(std::cin>>N>>K>>P>>R)) return false;
        catalogue.resize(P);group.resize(N);
        for(auto& bits:catalogue) {
            std::string s;std::cin>>s;
            bits=0;
            for(int j=0;j<N;++j) if(s[j]=='1') bits|=Mask(1)<<j;
        }
        return true;
    }
    void make_design() {
        std::vector<std::vector<int>> templates(N);
#ifndef AHC074_BIT_ONLY
        const int from=N/2,len=N-from,extras=K-2*N;
        std::vector<uint8_t> length(size_t(len)*P*P);
        for(int j=from;j<N;++j)
            for(int i=0;i<P;++i) for(int p=0;p<P;++p)
                length[(size_t(j-from)*P+i)*P+p]=match(catalogue[i],catalogue[p],j);
        std::vector<int> covered(P);
        std::vector<std::vector<char>> used(N,std::vector<char>(P));
        for(int step=0;step<extras && Clock::now()<design_deadline;++step) {
            int best=0,anchor=-1,id=-1,offset=0;
            for(int j=from;j<N;++j) if(templates[j].size()<7) {
                int overhead=7+int(templates[j].size());
                for(int i=0;i<P;++i) if(!used[j][i]) {
                    size_t index=(size_t(j-from)*P+i)*P;
                    int v=0;
                    for(int p=0;p<P;++p) v+=std::max<int>(0,length[index+p]-overhead-covered[p]);
                    if(v>best) best=v,anchor=j,id=i,offset=overhead;
                }
            }
            if(anchor<0) break;
            templates[anchor].push_back(id);
            used[anchor][id]=true;
            size_t index=(size_t(anchor-from)*P+id)*P;
            for(int p=0;p<P;++p) covered[p]=std::max<int>(covered[p],length[index+p]-offset);
        }
#endif
        auto add=[&](int j,Mask printed,Mask ink) {
            group[j].push_back(papers.size());papers.push_back({printed,ink});
        };
        for(int j=0;j<N;++j) {
            Mask b=Mask(1)<<j;
            const int white=std::count_if(catalogue.begin(),catalogue.end(),[&](Mask a){return (a&b)==0;});
            const Mask first=2*white>=P ? 0 : b;
            add(j,b,first);add(j,b,first^b);
            for(int item:templates[j]) add(j,prefix(j+1),catalogue[item]&prefix(j+1));
        }
        papers.resize(K,{0,0});
        order.resize(K);
        std::iota(order.begin(),order.end(),0);
    }
    void emit_design() const {
        for(auto& p:papers) {
            for(int j=0;j<N;++j)
                std::cout<<(p.printed>>j&1 ? (p.ink>>j&1 ? '1':'0') : '.');
            std::cout<<'\n';
        }
        std::cout.flush();
    }
    int pos(int p) const {return std::find(order.begin(),order.end(),p)-order.begin();}
    void move(int p,int reference,bool after,std::vector<std::pair<int,int>>& ops) {
        int old=pos(p),target=pos(reference);
        assert(p!=reference && old<K && target<K);
#ifndef NDEBUG
        Mask obstacles=0;
        for(int i=0;i<old;++i) obstacles|=papers[order[i]].printed;
        assert(papers[p].printed & ~obstacles);
#endif
        int place=target-int(old<target)+int(after);
        assert(0<=place && place<K);
        if(place==old) return;
        order.erase(order.begin()+old);
        order.insert(order.begin()+place,p);
        ops.push_back({p,place});
    }
    std::vector<std::pair<int,int>> query(Mask target) {
        std::vector<std::pair<int,int>> ops;
        // Each lifted prefix is still visible through its distinct right anchor.
        for(auto [anchor,k]:lifted) move(k,group[anchor][1],false,ops);
        lifted.clear();
        struct Edge {int lo=-1,id=-1;};
        std::vector<Edge> take(N+1);
        std::vector<int> dp(N+1,1<<25);
        dp[0]=0;
        for(int j=0;j<N;++j) {
            for(int rank=0;rank<int(group[j].size());++rank) {
                int id=group[j][rank];
                const auto& p=papers[id];
                const int extent=std::min(std::popcount(p.printed),match(target,p.ink,j));
                for(int len=1;len<=extent;++len) {
                    const int lo=j-len+1;
                    const int v=dp[lo]+rank+(len>1 ? 2 : 0);
                    if(v<dp[j+1]) dp[j+1]=v,take[j+1]={lo,id};
                }
            }
        }
        for(int j=N-1;j>=0;) {
            auto [lo,id]=take[j+1];
            assert(lo>=0);
            auto& stack=group[j];
            while(stack.front()!=id) {
                int other=stack.front();
                move(other,id,true,ops);
                stack.erase(stack.begin());
                stack.insert(std::find(stack.begin(),stack.end(),id)+1,other);
            }
            // Its exposed suffix becomes the display of [lo,j]. Outside this
            // range, untranslated earlier groups remain above the moved sheet.
            if(lo<j) {
                move(id,group[lo][0],false,ops);
                lifted.push_back({j,id});
            }
            j=lo-1;
        }
#ifndef NDEBUG
        Mask seen=0,bits=0;
        for(int id:order) {
            const auto& p=papers[id];
            bits|=p.ink&~seen;seen|=p.printed;
        }
        assert(bits==target);
        assert(ops.size()<=2*size_t(K));
#endif
        return ops;
    }
    void solve() {
        make_design();
        emit_design();
        long long total=0;
        int last=-1;
        for(int i=0;i<R;++i) {
            int request;
            if(!(std::cin>>request) || request<0) return;
            auto ops=request==last ? std::vector<std::pair<int,int>>{} : query(catalogue[request]);
            last=request;
            total+=ops.size();
            std::cout<<ops.size()<<'\n';
            for(auto [k,d]:ops) std::cout<<k<<' '<<d<<'\n';
            std::cout.flush();
        }
        std::cerr<<"Raw score = "<<total<<'\n';
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Problem p;
    if(p.read()) p.solve();
}
