#include "hall_small.hpp"
#include <cassert>
#include <iostream>
#include <random>
#include <utility>

using ahc::WorkerMask;
using W=WorkerMask;

static bool brute_assignment(std::span<const W> masks,W used=0) {
    if(masks.empty())return true;
    for(W bits=masks.front()&~used;bits;bits&=bits-1) {
        const W one=bits&(W(0)-bits);
        if(brute_assignment(masks.subspan(1),used|one))return true;
    }
    return false;
}
static void check(const std::vector<W>& a,int m) {
    auto p=ahc::find_hall_deficiency(m,a);
    assert(p.has_value() != brute_assignment(a));
    if(!p)return;

    // Separately enumerate worker subsets to check deterministic selection.
    W win=0;int k=m+1;
    for(W s=0;s<(W{1}<<m);++s){
        int inside=0,pc=std::popcount(s);
        for(W x:a) inside+=(x&~s)==0;
        if(inside>pc&&pc<k){win=s;k=pc;}
    }
    assert(k<=m);
    std::vector<std::size_t> expected;
    W scope=0;
    for(std::size_t i=0;i<a.size()&&expected.size()<std::size_t(k+1);++i)
        if((a[i]&~win)==0){expected.push_back(i);scope|=a[i];}
    assert(expected==p->tasks&&scope==p->worker_neighbors);
    assert(std::size_t(std::popcount(scope))<p->tasks.size());
}
template<class E,class F> void reject(F f){
    bool got=false;try{f();}catch(const E&){got=true;}assert(got);
}
int main(){
    using ahc::find_hall_deficiency;
    int count=0;
    for(int m=1;m<=3;++m)for(int g=0;g<=4;++g)
        for(int x=0;x<(1<<(m*g));++x){
            std::vector<W> a(g);
            for(int i=0;i<g;++i)a[i]=W((x>>(i*m))&((1<<m)-1));
            check(a,m);++count;
        }
    std::mt19937 rng(0xec9a9101u);
    for(int i=0;i<8000;++i){
        int m=1+rng()%8,g=rng()%11;
        std::vector<W> a(g);
        for(W& x:a)x=rng()%(1<<m);
        check(a,m);++count;
    }
    check({},0);check({0},0);check({0,0},0);check({1,2,1,2},2);
    reject<std::out_of_range>([]{find_hall_deficiency(-1,{});});
    reject<std::out_of_range>([]{find_hall_deficiency(17,{});});
    reject<std::invalid_argument>([]{find_hall_deficiency(0,std::vector<W>{1});});
    reject<std::invalid_argument>([]{find_hall_deficiency(2,std::vector<W>{0,4});});
    reject<std::invalid_argument>([]{find_hall_deficiency(16,std::vector<W>{W{1}<<31});});
    std::vector<W> disjoint(16);
    for(unsigned i=0;i<16;++i)disjoint[i]=W{1}<<i;
    assert(!find_hall_deficiency(16,disjoint));
    disjoint.push_back(W{1}<<15);
    auto cap=find_hall_deficiency(16,disjoint);
    assert(cap && (cap->worker_neighbors==(W{1}<<15)) &&
           (cap->tasks==std::vector<std::size_t>{15,16}));
    auto huge=find_hall_deficiency(1,std::vector<W>(100000,1));
    assert(huge&&(huge->tasks==std::vector<std::size_t>{0,1}));
    std::cout<<count<<" exhaustive/random graph cases plus checked boundaries passed\n";
}
