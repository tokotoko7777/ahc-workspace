#pragma GCC optimize("O3")
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "../../library/class-constrained-pair-insertion.hpp"

using Clock=std::chrono::steady_clock;
struct RNG {
    uint32_t x=19891765;
    uint32_t get() { x^=x<<13;x^=x>>17;x^=x<<5;return x; }
};
struct Problem {
    int n,c,start,finish;
    std::vector<int> label,match;
    std::vector<std::vector<int>> dist;

    bool read() {
        if(!(std::cin>>n)) return false;
        c=n*n;start=c;finish=c+1;
        label.resize(c);
        match.assign(c,-1);
        std::vector<int> first(c/2,-1);
        for(int i=0;i<c;++i) {
            int a;std::cin>>a;label[i]=a;
            if(first[a]<0) first[a]=i;
            else match[i]=first[a],match[first[a]]=i;
        }
        dist.assign(c+2,std::vector<int>(c+2));
        for(int i=0;i<=c;++i) for(int j=0;j<=c;++j)
            dist[i][j]=std::abs((i%c)/n-(j%c)/n)+std::abs(i%n-j%n);
        return true;
    }
    std::vector<int> greedy() const {
        std::vector<int> path{start},used(c);
        int at=start;
        for(int t=0;t<c/2;++t) {
            int a=-1,score=1<<30;
            for(int i=0;i<c;++i) if(!used[i]) {
                int s=dist[at][i]*4+dist[i][match[i]];
                if(s<score) a=i,score=s;
            }
            int b=match[a];
            used[a]=used[b]=1;
            path.push_back(a);path.push_back(b);
            at=b;
        }
        path.push_back(finish);
        return path;
    }
    std::vector<int> improve(std::vector<int> route,Clock::time_point deadline) const {
        const auto d=[&](int a,int b){return dist[a][b];};
        const auto measure=[&](const auto& path) {
            int v=0;
            for(unsigned i=1;i<path.size();++i) v+=d(path[i-1],path[i]);
            return v;
        };
        auto best=route;
        int current=measure(route),record=current;
        RNG rand;
        std::vector<int> work,classes,stack;
        work.reserve(c+2);classes.reserve(c+2);stack.reserve(c/2);
        constexpr int iterations=20000;
        double temp=4;
        for(int it=0;it<iterations;++it) {
            if(it%64==0) {
                if(Clock::now() >= deadline) break;
                temp=4*std::pow(.07,double(it)/iterations);
            }
            int a=rand.get()%c,b=match[a];
            work.clear();classes.clear();stack.clear();
            work.push_back(start);classes.push_back(0);
            int reduced=0;
            for(int p:route) if(p!=a && p!=b && p!=start) {
                reduced+=d(work.back(),p);
#ifdef AHC059_ADJACENT
                classes.push_back(work.size());
#else
                classes.push_back(stack.empty() ? 0 : stack.back()+1);
                if(p!=finish) {
                    if(!stack.empty() && stack.back()==label[p]) stack.pop_back();
                    else stack.push_back(label[p]);
                }
#endif
                work.push_back(p);
            }
            ClassPairInsertion<int> best_insert;
            if(rand.get()%5) {
                best_insert=best_class_constrained_pair_insertion(work,classes,c+2,a,b,d);
                auto reverse=best_class_constrained_pair_insertion(work,classes,c+2,b,a,d);
                if(reverse.delta<best_insert.delta) best_insert=reverse,std::swap(a,b);
            } else {
                int l=1+rand.get()%(work.size()-1),r=l,seen=0;
                for(int g=1;g<int(work.size());++g)
                    if(classes[g]==classes[l] && rand.get()%unsigned(++seen)==0) r=g;
                if(r<l) std::swap(r,l);
                const int delta=l==r ? d(work[l-1],a)+d(a,b)+d(b,work[l])-d(work[l-1],work[l])
                    : d(work[l-1],a)+d(a,work[l])-d(work[l-1],work[l])
                     +d(work[r-1],b)+d(b,work[r])-d(work[r-1],work[r]);
                best_insert={delta,l,r};
            }
            int next=reduced+best_insert.delta;
            if(next>current && (rand.get()+.5)/4294967296.0>=std::exp((current-next)/temp)) continue;
            current=next;
            work.insert(work.begin()+best_insert.second_gap,b);
            work.insert(work.begin()+best_insert.first_gap,a);
            route.swap(work);
            if(next<record) record=next,best=route;
        }
        return best;
    }
    void emit(const std::vector<int>& route) const {
        int x=0,y=0,moves=0;
        std::string out;
        auto walk=[&](int& at,int target,char plus,char minus) {
            while(at!=target) {
                const int step=at<target ? 1 : -1;
                out+=step==1 ? plus : minus;
                at+=step;++moves;
            }
        };
        for(int p:route) if(p!=start && p!=finish) {
            walk(x,p/n,'D','U');walk(y,p%n,'R','L');
            out+='Z';
        }
        for(char op:out) std::cout<<op<<'\n';
        std::cerr<<"Moves = "<<moves<<'\n';
    }
};
int main() {
    std::cin.tie(nullptr);std::ios::sync_with_stdio(false);
    auto deadline=Clock::now()+std::chrono::milliseconds(1550);
    Problem problem;
    if(problem.read()) problem.emit(problem.improve(problem.greedy(),deadline));
}
