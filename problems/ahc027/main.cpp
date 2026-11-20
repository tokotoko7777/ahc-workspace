#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <span>
#include <string>
#include <vector>
#include "../../library/cyclic-visit-energy.hpp"

#if !defined(AHC027_TARGET_GAPS) && !defined(AHC027_EXTRA_SPLICES) && !defined(AHC027_TREE_ONLY) && !defined(AHC027_FIRST_SPLICES)
#define AHC027_TARGET_GAPS
#endif

struct Grid {
    int n,m;
    std::vector<std::array<int,4>> next;
    std::vector<int> weight;
    bool read() {
        if(!(std::cin>>n)) return false;
        m=n*n;
        next.resize(m);
        for(auto& a:next) a.fill(-1);
        for(int r=0;r+1<n;++r) {
            std::string row;std::cin>>row;
            for(int c=0;c<n;++c) if(row[c]=='0') {
                int a=r*n+c;next[a][1]=a+n;next[a+n][0]=a;
            }
        }
        for(int r=0;r<n;++r) {
            std::string row;std::cin>>row;
            for(int c=0;c+1<n;++c) if(row[c]=='0') {
                int a=r*n+c;next[a][3]=a+1;next[a+1][2]=a;
            }
        }
        weight.resize(m);
        for(int& x:weight) std::cin>>x;
        return true;
    }
    bool edge(int a,int b) const {
        return std::find(next[a].begin(),next[a].end(),b)!=next[a].end();
    }
    void dfs(int v,std::vector<unsigned char>& marked,std::vector<int>& path,
             std::mt19937& rng) const {
        marked[v]=1;path.push_back(v);
        auto adj=next[v];std::shuffle(adj.begin(),adj.end(),rng);
        for(int u:adj) if(u>=0&&!marked[u]) {
            dfs(u,marked,path,rng);path.push_back(v);
        }
    }
};


#if defined(AHC027_TARGET_GAPS) || defined(AHC027_EXTRA_SPLICES)
void extend_patrol(const Grid& g,CyclicVisitEnergy& energy,std::vector<int>& best,
                   std::int64_t& best_num,std::mt19937& rng) {
    using Clock=std::chrono::steady_clock;
    const auto deadline=Clock::now()+std::chrono::milliseconds(150);
    auto current=best;
    auto current_num=best_num;
    std::vector<int> work;
    std::vector<std::vector<int>> visits(g.m);
#ifdef AHC027_TARGET_GAPS
    struct Gap {int v,from,len;std::int64_t importance;};
    std::vector<Gap> gaps;
    const int reach=7;
    // For each target, first steps toward it from positions within reach.
    std::vector<std::int16_t> toward(g.m*g.m,-1);
    std::vector<int> fifo(g.m),depth(g.m);
    for(int to=0;to<g.m;++to) {
        auto* p=toward.data()+to*g.m;
        p[to]=to;fifo[0]=to;depth[0]=0;int tail=1;
        for(int i=0;i<tail;++i) {
            int u=fifo[i];
            if(depth[i]==reach) continue;
            for(int v:g.next[u]) if(v>=0&&p[v]<0) {
                p[v]=u;depth[tail]=depth[i]+1;fifo[tail++]=v;
            }
        }
    }
#endif
    auto index=[&] {
        for(auto& a:visits) a.clear();
        const int l=current.size()-1;
        for(int i=0;i<l;++i) visits[current[i]].push_back(i);
#ifdef AHC027_TARGET_GAPS
        gaps.clear();
        for(int v=0;v<g.m;++v) {
            const auto& a=visits[v];
            int from=a.back()-l;
            for(int next:a) {
                const int d=next-from;
                gaps.push_back({v,from<0?from+l:from,d,1LL*d*d*g.weight[v]});
                from=next;
            }
        }
        if(gaps.size()>180) {
            std::nth_element(gaps.begin(),gaps.begin()+180,gaps.end(),
                             [](auto a,auto b){return a.importance>b.importance;});
            gaps.resize(180);
        }
#endif
    };
    index();
    for(int it=0;it<160000;++it) {
        if((it&63)==0&&Clock::now()>=deadline) break;
        const int l=current.size()-1;
#ifdef AHC027_TARGET_GAPS
        if(it%3!=0) {
            if(l>=4*g.m) continue;
            const auto gap=gaps[rng()%gaps.size()];
            int offset=gap.len/2+int(rng()%(gap.len/3+1))-gap.len/6;
            int i=(gap.from+offset)%l, at=current[i];
            auto* step=toward.data()+gap.v*g.m;
            if(step[at]<0||at==gap.v) continue;
            std::array<int,15> excursion{};
            int h=0;
            for(int v=at;v!=gap.v;) {v=step[v];excursion[h++]=v;}
            const int outward=h;
            for(int j=outward-2;j>=0;--j) excursion[h++]=excursion[j];
            excursion[h++]=at;
            work=current;
            work.insert(work.begin()+i+1,excursion.begin(),excursion.begin()+h);
        } else
#endif
        {
            const int i=rng()%l;
            int v=g.next[current[i]][rng()%4],kind=rng()%10;
            if(v<0) continue;
            if(kind<=2) {
                if(l>=4*g.m) continue;
                work=current;
                work.insert(work.begin()+i+1,{v,current[i]});
            } else {
                int j=visits[v][rng()%visits[v].size()];
                const int a=std::min(i,j),b=std::max(i,j);
                if(b-a<2||!g.edge(current[a],current[b])) continue;
                if(kind<=6) {
                    if(b-a>37) continue;
                    work=current;
                    work.erase(work.begin()+a+1,work.begin()+b);
                } else {
                    if(!g.edge(current[a+1],current[b+1])) continue;
                    work=current;
                    std::reverse(work.begin()+a+1,work.begin()+b+1);
                }
            }
        }
        const std::span<const int> route(work.data(),work.size()-1);
        auto v=energy.numerator(route,g.weight);
        if(!v || static_cast<__int128>(*v)*l>=static_cast<__int128>(current_num)*route.size()) continue;
        current.swap(work);
        current_num=*v;
        index();
    }
    best=std::move(current);
    best_num=current_num;
}
#endif

int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Grid g;
    if(!g.read()) return 1;
    using Clock=std::chrono::steady_clock;
    const auto started=Clock::now(),deadline=started+std::chrono::milliseconds(440);
    std::mt19937 rng(719275);
    CyclicVisitEnergy energy(g.m);
    auto numer=[&](const std::vector<int>& p) {
        return energy.numerator(std::span(p).first(p.size()-1),g.weight);
    };
    std::vector<int> best,work;
    work.reserve(4*g.m);
    std::vector<unsigned char> marked(g.m);
    std::int64_t best_num=1;
    auto better=[](std::int64_t a,int sa,std::int64_t b,int sb){
        return static_cast<__int128>(a)*sb<static_cast<__int128>(b)*sa;
    };
    auto random_dfs=[&] {
        work.clear();std::fill(marked.begin(),marked.end(),0);
        g.dfs(0,marked,work,rng);
        auto s=numer(work);
        if(s && (best.empty()||better(*s,work.size()-1,best_num,best.size()-1))) {
            best_num=*s;best=work;
        }
    };
    for(int i=0;i<48;++i) random_dfs();
#ifdef AHC027_TREE_ONLY
    for(int i=48; i<20000&&Clock::now()<deadline;++i) random_dfs();
#else
    std::vector<int> current=best;
    std::int64_t current_num=best_num;
    std::vector<std::vector<int>> positions(g.m);
    auto index=[&] {
        for(auto& list:positions) list.clear();
        for(int i=0;i<int(current.size());++i) positions[current[i]].push_back(i);
    };
    index();
    for(int iter=0;iter<85000;++iter) {
        if((iter&127)==0&&Clock::now()>=deadline) break;
        int length=int(current.size())-1;
        const int i=rng()%length;
        int v=g.next[current[i]][rng()%4];
        if(v<0) continue;
        int kind=rng()%10;
        if(kind<=2) {
            if(length>=4*g.m || length>=99998) continue;
            work=current;
            work.insert(work.begin()+i+1,{v,current[i]});
        } else {
            const auto& list=positions[v];
            int j=list[rng()%list.size()];
            const int a=std::min(i,j),b=std::max(i,j);
            if(b-a<2) continue;
            if(!g.edge(current[a],current[b])) continue;
            if(kind<=6) {
                if(b-a>37) continue;
                work=current;
                work.erase(work.begin()+a+1,work.begin()+b);
            } else {
                if(b>=length||!g.edge(current[a+1],current[b+1])) continue;
                work=current;
                std::reverse(work.begin()+a+1,work.begin()+b+1);
            }
        }
        auto s=numer(work);
        if(!s) continue;
        const int wl=work.size()-1;
        const bool good=better(*s,wl,current_num,length);
        if(!good) {
            const double loss=(double(*s)/wl)/(double(current_num)/length)-1.;
            const double temperature=0.00019+0.00085*(1.0-iter/85000.0);
            if(loss>0. && std::generate_canonical<double,32>(rng)>=std::exp(-loss/temperature)) continue;
        }
        current.swap(work);current_num=*s;index();
        if(better(current_num,current.size()-1,best_num,best.size()-1)) {
            best=current;best_num=current_num;
        }
    }
#endif
#if defined(AHC027_TARGET_GAPS) || defined(AHC027_EXTRA_SPLICES)
    extend_patrol(g,energy,best,best_num,rng);
#endif
    constexpr char directions[]="UDLR";
    for(int i=1;i<int(best.size());++i) {
        auto& options=g.next[best[i-1]];
        const int dir=std::find(options.begin(),options.end(),best[i])-options.begin();
        if(dir==4) return 1;
        std::cout<<directions[dir];
    }
    std::cout<<'\n';
}
