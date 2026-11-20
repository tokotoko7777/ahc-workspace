#include "../library/bounded-depth-forest.hpp"
#include <algorithm>
#include <random>

static bool brute(const std::vector<int>& parents,const std::vector<long long>& w,int cap,
                  std::vector<int>& d,long long& score) {
    score=0;
    for(int i=0;i<int(parents.size());++i) {
        int x=i,depth=-1;
        while(x>=0 && depth<=cap) ++depth,x=parents[x];
        if(x>=0 || depth>cap) return false;
        d[i]=depth;
        score+=w[i]*(depth+1);
    }
    return true;
}
int main() {
    std::mt19937 rng(4110739);
    for(int trial=0;trial<180;++trial) {
        int n=1+rng()%30,cap=1+rng()%8;
        std::vector<int> parents(n,-1),depth(n),check(n);
        std::vector<long long> w(n);
        for(int i=0;i<n;++i) w[i]=int(rng()%30)-10;
        long long value=0,next=0;
        assert(brute(parents,w,cap,depth,value));
        BoundedDepthForest tree(parents,w,cap);
        for(int k=0;k<220;++k) {
            int v=rng()%n,p=int(rng()%(n+1))-1;
            auto probe=parents;
            probe[v]=p;
            const bool permitted=rng()%3!=0;
            bool legal=parents[v]!=p && brute(probe,w,cap,check,next);
            bool called=false;
            bool moved=tree.try_reparent(v,p,[&](long long gain) {
                called=true;assert(legal && gain==next-value);return permitted;
            });
            assert(called==legal && moved==(legal&&permitted));
            if(moved) parents=probe,depth=check,value=next;
            assert(tree.parents()==parents && tree.depths()==depth && tree.score()==value);
        }
    }
}
