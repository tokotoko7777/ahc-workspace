#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>
#include "../../library/two-sided-dag-frontier.hpp"

constexpr int R=30,V=R*(R+1)/2;
using Board=std::array<int,V>;
using Pair=std::pair<int,int>;

struct Grid {
    Board row{},col{};
    std::array<std::array<int,6>,V> nb{};
    std::vector<std::vector<int>> dag;
    static int id(int r,int c) {return r*(r+1)/2+c;}
    Grid():dag(V) {
        constexpr int dr[]={0,0,-1,-1,1,1},dc[]={-1,1,-1,0,0,1};
        for(int r=0;r<R;++r) for(int c=0;c<=r;++c) {
            const int i=id(r,c);
            row[i]=r; col[i]=c;
            for(int a=0;a<6;++a) {
                int y=r+dr[a],x=c+dc[a];
                nb[i][a]=(0<=y&&y<R&&0<=x&&x<=y)?id(y,x):-1;
                if(a>=4&&nb[i][a]>=0) dag[i].push_back(nb[i][a]);
            }
        }
    }
};

struct Path {
    std::vector<int> p;
    int rank=std::numeric_limits<int>::max();
};

struct Trial {
    const Grid& g;
    Board board,pos;
    TwoSidedDagFrontier frontier;
    Board dist,prev,secondary,q;
    Board order_bias{};
    std::vector<Pair> moves;
    int lo=0,hi=V-1;
    int mode,rot,extra,prior;
    const bool mixed;

    Trial(const Grid& grid,const Board& input,const TwoSidedDagFrontier& f,int variant,std::mt19937& rng)
        :g(grid),board(input),frontier(f),mode(variant%8),rot(rng()%6),extra(variant%3?0:1),
         prior(90+45*(variant%7)),mixed(variant%9!=0) {
        for(int i=0;i<V;++i) {
            pos[board[i]]=i;
            order_bias[i]=rng()%32;
        }
    }
    Path route(int v,bool high) {
        Path result;
        const int start=pos[v];
        dist.fill(-1);
        dist[start]=0;prev[start]=-1;secondary[start]=0;
        q[0]=start;int tail=1,best=-1,reach=V;
        for(int head=0;head<tail;++head) {
            int here=q[head],length=dist[here];
            if(length>reach+extra) break;
            if(frontier.ready(here,high)) {
                reach=std::min(reach,length);
                const int bias=(mode<4?1:3)*g.row[here]*(high?-1:1);
                const int value=128*length+secondary[here]+bias+(mode%2?order_bias[here]:0);
                if(value<result.rank) result.rank=value,best=here;
            }
            if(length>=reach+extra) continue;
            for(int k=0;k<6;++k) {
                const int j=g.nb[here][(k+rot)%6];
                if(j<0||!frontier.live(j)|| (dist[j]>=0&&dist[j]!=length+1)) continue;
                const int horizontal=(g.row[here]==g.row[j]);
                // When a neighbor is displaced downwards, favor moving a
                // higher-ranked label; similarly for upwards displacement.
                const int slide=(board[j]-(lo+hi)/2)*(g.row[j]-g.row[here]);
                const int cost=secondary[here]+slide/(25+10*(mode%4))+horizontal;
                if(dist[j]<0) { q[tail++]=j;dist[j]=length+1;secondary[j]=cost;prev[j]=here; }
                else if(cost<secondary[j]) secondary[j]=cost,prev[j]=here;
            }
        }
        if(best<0) return result;
        for(int i=best;i>=0;i=prev[i]) result.p.push_back(i);
        std::reverse(result.p.begin(),result.p.end());
        return result;
    }

    bool solve(std::size_t cutoff) {
        for(;lo<=hi;) {
            Path small=route(lo,false),large;
            if(mixed && lo<hi) large=route(hi,true);
            const int large_penalty=prior+std::max(0,(V-1-hi)-lo)*2;
            bool high=!large.p.empty()&&((large.rank+large_penalty<small.rank)||small.p.empty());
            Path& sel=high?large:small;
            if(sel.p.empty()) return false;
            for(int k=1;k<int(sel.p.size());++k) {
                const int a=sel.p[k-1],b=sel.p[k];
                std::swap(board[a],board[b]);
                pos[board[a]]=a;pos[board[b]]=b;
                if(!moves.empty() && (moves.back()==Pair{b,a} || moves.back()==Pair{a,b}))
                    moves.pop_back();
                else moves.emplace_back(a,b);
            }
            frontier.remove(sel.p.back(),high);
            if(high) --hi; else ++lo;
        }
        return moves.size()<cutoff;
    }
};

int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Board b{};
    for(auto& v:b) if(!(std::cin>>v)) return 1;
    const Grid g;
    const TwoSidedDagFrontier f(g.dag);
    std::mt19937 rng(8976321);
    std::vector<Pair> best;
    const auto stop=std::chrono::steady_clock::now()+std::chrono::milliseconds(510);
    for(int i=0;i<240&&(i==0||std::chrono::steady_clock::now()<stop);++i) {
        Trial trial(g,b,f,i,rng);
        if(trial.solve(best.empty()?10001:best.size())) best=std::move(trial.moves);
    }
    std::cout<<best.size()<<'\n';
    for(auto [a,b]:best)
        std::cout<<g.row[a]<<' '<<g.col[a]<<' '<<g.row[b]<<' '<<g.col[b]<<'\n';
}
