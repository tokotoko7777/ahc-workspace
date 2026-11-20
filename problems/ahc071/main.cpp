#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

using Mask=uint64_t;
struct Plan {
    int cost=0;
    std::vector<std::tuple<int,int,int>> bricks;
};
struct Problem {
    int W,H,K;
    std::array<int,10> cost{};
    std::vector<Mask> holes;
    static constexpr std::array<int,6> widths{0,1,3,5,7,9};
    static int weight(int value) {return value*1024+1;}
    bool read() {
        if(!(std::cin>>W>>H>>K)) return false;
        for(int l:widths) if(l) std::cin>>cost[l];
        holes.assign(H,0);
        for(int k=0;k<K;++k) {
            int x,y;std::cin>>x>>y;holes[y]|=Mask(1)<<x;
        }
        return true;
    }
    Mask one(Plan& plan,int y,Mask required) const {
        constexpr int INF=1<<28;
        std::vector<int> best(W+1,INF),from(W+1);
        best[0]=0;
        for(int x=0;x<W;++x) if(best[x]<INF) {
            if(((required>>x)&1)==0 && best[x]<best[x+1]) best[x+1]=best[x],from[x+1]=0;
            for(int l:widths) if(l && x+l<=W) {
                int v=best[x]+weight(cost[l]);
                if(v<best[x+l]) best[x+l]=v,from[x+l]=l;
            }
        }
        Mask below=0;
        for(int x=W;x>0;) {
            int l=from[x];
            if(l) {
                x-=l;plan.bricks.emplace_back(x,y,l);plan.cost+=cost[l];
                below|=Mask(1)<<(x+(l-1)/2);
            } else --x;
        }
        return below;
    }
    struct Tail {int left,center;};
    struct Move {int next,up,dn,cover,weight;};
    struct Kernel {
        std::vector<Tail> state{{0,-1}};
        std::array<std::array<int,6>,9> id{};
        std::vector<std::vector<Move>> transition;
        explicit Kernel(const std::array<int,10>& cost) {
            for(auto& line:id) line.fill(-1);
            id[0][0]=0;
            for(int l:widths) for(int off=1;off<l;++off) {
                const int left=l-off,c=std::max(-1,(l-1)/2-off);
                if(id[left][c+1]<0) {
                    id[left][c+1]=state.size();state.push_back({left,c});
                }
            }
            transition.resize(state.size()*9);
            for(int i=0;i<int(state.size());++i) for(int b=0;b<9;++b) {
                for(int startA:widths) {
                    if(i && startA) continue;
                    const int a=i ? state[i].left : startA;
                    const int center=i ? state[i].center : (startA ? (startA-1)/2 : -1);
                    const int newA=a<=1 ? 0 : id[a-1][std::max(-1,center-1)+1];
                    for(int startB:widths) {
                        if(b && startB) continue;
                        int lower=b ? b : startB;
                        if(center==0 && !lower) continue;
                        int newB=std::max(0,lower-1);
                        int cover=(a>0)+2*(lower>0);
                        int score=(startA ? weight(cost[startA]) : 0)+(startB ? weight(cost[startB]) : 0);
                        transition[i*9+b].push_back({newA*9+newB,startA,startB,cover,score});
                    }
                }
            }
        }
    };
    Mask two(Plan& plan,int y,Mask upper,Mask lower,const Kernel& k,Mask foundation=~Mask(0)) const {
        constexpr int INF=1<<28;
        int n=k.transition.size();
        std::vector<int> current(n,INF),next(n),choice(n*(W+1),-1);
        current[0]=0;
        for(int x=0;x<W;++x) {
            std::fill(next.begin(),next.end(),INF);
            int need=((upper>>x)&1)+2*((lower>>x)&1);
            for(int i=0;i<n;++i) if(current[i]<INF) {
                const auto& trans=k.transition[i];
                for(int j=0;j<int(trans.size());++j) {
                    const auto& t=trans[j];
                    if((t.cover&need)!=need || x+std::max(t.up,t.dn)>W) continue;
                    if(t.dn && !((foundation>>(x+(t.dn-1)/2))&1)) continue;
                    const int score=current[i]+t.weight;
                    if(score<next[t.next]) {
                        next[t.next]=score;choice[(x+1)*n+t.next]=i*64+j;
                    }
                }
            }
            current.swap(next);
        }
        Mask below=0;
        int at=0;
        for(int x=W;x>0;--x) {
            const int tag=choice[x*n+at];
            if(tag<0) throw std::logic_error("single bricks always provide a two-row solution");
            const int old=tag/64;
            const auto& t=k.transition[old][tag%64];
            if(t.up) plan.bricks.emplace_back(x-1,y,t.up),plan.cost+=cost[t.up];
            if(t.dn) {
                plan.bricks.emplace_back(x-1,y-1,t.dn);plan.cost+=cost[t.dn];
                below|=Mask(1)<<(x-1+(t.dn-1)/2);
            }
            at=old;
        }
        return below;
    }
    Plan windows(Plan plan,const Kernel& k,bool staggered) const {
        struct Row {
            int price=0;
            Mask used=0,centers=0;
            std::vector<std::tuple<int,int,int>> bricks;
            void add(int x,int y,int l,int c) {
                used|=((Mask(1)<<l)-1)<<x;
                centers|=Mask(1)<<(x+(l-1)/2);
                price+=c;bricks.emplace_back(x,y,l);
            }
        };
        std::vector<Row> rows(H);
        for(auto [x,y,l]:plan.bricks) rows[y].add(x,y,l,cost[l]);
        auto improve=[&](int y) {
            const Mask upper=holes[y] | (y+1<H ? rows[y+1].centers : 0);
            const Mask below=y>=2 ? rows[y-2].used : ~Mask(0);
            Plan v;
            two(v,y,upper,holes[y-1],k,below);
            if(v.cost<rows[y].price+rows[y-1].price) {
                rows[y]={};rows[y-1]={};
                for(auto [x,z,l]:v.bricks) rows[z].add(x,z,l,cost[l]);
            }
        };
        for(int pass=0;pass<4;++pass) {
            if(staggered) {
                for(int parity=0;parity<2;++parity)
                    for(int y=H-1;y>=1;--y)
                        if(y%2==(parity+pass)%2) improve(y);
            } else for(int y=H-1;y>=1;--y) improve(y);
        }
        Plan out;
        for(auto& r:rows) {
            out.cost+=r.price;
            out.bricks.insert(out.bricks.end(),r.bricks.begin(),r.bricks.end());
        }
        return out;
    }
    Plan solve() const {
        Plan best;
        Mask support=0;
        for(int y=H-1;y>=0;--y) support=one(best,y,holes[y]|support);
#ifndef AHC071_ONE_ROW
        const Kernel kernel(cost);
        // Two partitions, shifted by one row. Always retain the legal one-row plan.
        for(int odd=0;odd<2;++odd) {
            Plan candidate;
            support=0;
            int y=H-1;
            if(odd) support=one(candidate,y,holes[y]),--y;
            for(;y>0;y-=2) support=two(candidate,y,holes[y]|support,holes[y-1],kernel);
            if(y==0) one(candidate,0,holes[0]|support);
            if(candidate.cost<best.cost) best=std::move(candidate);
        }
#if defined(AHC071_SWEEP_STAGGERED)
        best=windows(std::move(best),kernel,true);
#elif !defined(AHC071_NO_WINDOWS)
        best=windows(std::move(best),kernel,false);
#endif
#endif
        return best;
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Problem p;
    if(!p.read()) return 0;
    const auto answer=p.solve();
    std::cout<<answer.bricks.size()<<'\n';
    for(auto [x,y,l]:answer.bricks) std::cout<<x<<' '<<y<<' '<<l<<'\n';
    std::cerr<<"Cost = "<<answer.cost<<'\n';
    return 0;
}
