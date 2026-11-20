#pragma once
#include "../../library/orthogonal-slide-grid.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>
#include <vector>

namespace suffix_swap {
using Clock=std::chrono::steady_clock;

struct Improvement {
    std::vector<int> path;
    double expected=0.;
};

inline std::vector<std::vector<double>> continuation(
    int n,const std::vector<int>& path) {
    const int m=n*n,k=path.size();
    std::vector<unsigned char> all_walls(m,true);
    OrthogonalSlideGrid board(n,n,all_walls);
    std::vector<std::vector<double>> future(k+1,std::vector<double>(m));
    for(int t=k-1;t>=0;--t) {
        const int rock=path[t];
        board.set_blocked(rock,false);
        const auto& after=future[t+1];
        auto& value=future[t];
        for(int u=0;u<m;++u) if(!board.blocked(u)) {
            double sum=0;
            for(int v:board.destinations(u)) if(v!=rock) sum+=1.+after[v];
            value[u]=sum*.25;
        }
    }
    return future;
}

inline Improvement sweep(int n,const std::vector<unsigned char>& initial,
                         const std::vector<int>& order,int parity,bool exact,
                         Clock::time_point deadline) {
    const int k=order.size(),m=n*n;
    Improvement result{order,0.};
    auto future=continuation(n,order);
    OrthogonalSlideGrid board(n,n,initial);
    std::vector<double> mass(m),step(m),ab(m),ba(m);
    for(int v:order) mass[v]=1./k;
    double survival=1.;
    int t=0;
    auto one=[&](int v) {
        board.redistribute(mass,step);
        survival-=step[v];
        result.expected+=survival;
        step[v]=0.;
        mass.swap(step);
        board.set_blocked(v,true);
    };
    if(parity && k) one(order[t++]);
    for(;t+1<k;t+=2) {
        if((t&31)<2 && Clock::now()>=deadline) return {};
        const int a=order[t],b=order[t+1];
        board.redistribute(mass,step);
        auto pair=[&](int first,int second,std::vector<double>& after) {
            board.set_blocked(first,true);
            board.redistribute(step,after); // dropping the first blocked cell
            board.set_blocked(first,false);
            const double r1=survival-step[first],r2=r1-after[second];
            after[second]=0.;
            return std::pair(r1+r2,r2);
        };
        auto [score1,remaining1]=pair(a,b,ab);
        auto [score2,remaining2]=pair(b,a,ba);
        const auto& later=future[t+2];
        const double with1=score1+std::inner_product(ab.begin(),ab.end(),later.begin(),0.);
        const double with2=score2+std::inner_product(ba.begin(),ba.end(),later.begin(),0.);
        const bool change=exact ? with2>with1+1e-11 : score2>score1+1e-11;
        board.set_blocked(a,true);
        board.set_blocked(b,true);
        if(change) {
            std::swap(result.path[t],result.path[t+1]);
            mass.swap(ba);
            result.expected+=score2;
            survival=remaining2;
        } else {
            mass.swap(ab);
            result.expected+=score1;
            survival=remaining1;
        }
    }
    if(t<k) one(order[t]);
    return result;
}
}
