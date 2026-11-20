#pragma GCC optimize("O3")
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <vector>
#include "../../library/orthogonal-slide-grid.hpp"

using Clock=std::chrono::steady_clock;
struct Answer {
    double expected=0;
    std::vector<int> placements;
};

static uint32_t ordering(int v,int left) {
    uint32_t key=uint32_t(v)*0x85ebca6bU+uint32_t(left)*0xc2b2ae35U;
    key^=key>>16;key*=0x7feb352dU;
    key^=key>>15;key*=0x846ca68bU;
    return key^(key>>16);
}
static Answer build(int n,const std::vector<unsigned char>& wall,
                    int depth,Clock::time_point stop) {
    const int cells=n*n;
    OrthogonalSlideGrid rink(n,n,wall);
    std::vector<int> free;
    for(int v=0;v<cells;++v) if(!wall[v]) free.push_back(v);
    std::vector<double> p(cells),q(cells),first(cells),second(cells);
    for(int v:free) p[v]=1./free.size();
    double living=1.;
    Answer result;
    result.placements.reserve(free.size());
    std::vector<int> list;
    auto vulnerable=[&](const std::vector<double>& dist,int absent) {
        int best=-1;
        for(int v:free) if(v!=absent && (best<0 || dist[v]<dist[best]))
            best=v;
        return best;
    };
    while(!free.empty()) {
        if(depth && Clock::now()>=stop) return {};
        rink.redistribute(p,q);
        int picked=vulnerable(q,-1);
        if(depth && free.size()>=3 && living>1e-15) {
            list=free;
            const int shortlist=std::min<int>(14,list.size());
            auto compare=[&](int a,int b) {
                return std::pair(q[a],ordering(a,free.size()))<
                       std::pair(q[b],ordering(b,free.size()));
            };
            std::partial_sort(list.begin(),list.begin()+shortlist,list.end(),compare);
            double best=std::numeric_limits<double>::infinity(),shape_best=-1.;
            for(int j=0;j<shortlist;++j) {
                int a=list[j];
                if(3*q[a]>best+1e-16) break;
                rink.set_blocked(a,true);
                rink.redistribute(q,first);
                int b=vulnerable(first,a);
                rink.set_blocked(b,true);
                rink.redistribute(first,second);
                double c_risk=1.;
                double concentration=0.;
                for(int c:free) if(c!=a && c!=b) {
                    c_risk=std::min(c_risk,second[c]);
                    concentration+=second[c]*second[c];
                }
                double cost=3*q[a]+2*first[b]+c_risk;
                rink.set_blocked(b,false);
                rink.set_blocked(a,false);
                if(cost<best-1e-16 || (std::abs(cost-best)<=1e-16 && concentration>shape_best)) {
                    best=cost;shape_best=concentration;picked=a;
                }
            }
        }
        living-=q[picked];
        result.expected+=living;
        q[picked]=0.;
        p.swap(q);
        rink.set_blocked(picked,true);
        result.placements.push_back(picked);
        free.erase(std::find(free.begin(),free.end(),picked));
    }
    return result;
}

int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n,m;
    if(!(std::cin>>n>>m)) return 1;
    std::vector<unsigned char> wall(n*n);
    for(int r=0;r<n;++r) {
        std::string s;std::cin>>s;
        for(int c=0;c<n;++c) wall[r*n+c]=s[c]=='#';
    }
    const auto deadline=Clock::now()+std::chrono::milliseconds(1700);
    auto answer=build(n,wall,0,deadline);
#ifndef AHC050_GREEDY_ONLY
    auto candidate=build(n,wall,1,deadline);
#ifdef AHC050_LOOKAHEAD_ONLY
    if(!candidate.placements.empty()) answer=std::move(candidate);
#else
    if(candidate.placements.size()==answer.placements.size() && candidate.expected>answer.expected)
        answer=std::move(candidate);
#endif
#endif
    for(int v:answer.placements) std::cout<<v/n<<' '<<v%n<<'\n';
    std::cerr<<"Model = "<<std::llround(1e6*answer.expected/(n*n-m-1))<<'\n';
}
