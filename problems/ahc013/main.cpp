#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "../../library/dsu.hpp"
#include "../../library/grid-line-occupancy64.hpp"

struct Step { int from,to; };
struct Cable {int u,v,from,to,len,step;};
struct Plan { int score=0; std::vector<Step> links; };

struct Board {
    int n,k;
    std::vector<int> cell, pos, color;
    GridLineOccupancy64 sight;
    Board(int n,int k) : n(n), k(k), cell(n*n,-1), sight(n,n) {}
    void set(int id,int where) {
        int prev=pos[id];
        if(prev>=0) { cell[prev]=-1; sight.set(prev/n,prev%n,false); }
        pos[id]=where;
        cell[where]=id;
        sight.set(where/n,where%n,true);
    }
    std::vector<Cable> cables() const {
        std::vector<Cable> result;
        result.reserve(2*pos.size());
        for(int id=0;id<int(pos.size());++id) {
            const auto p=pos[id];
            auto see=sight.visible(p/n,p%n);
            for(int d:{1,3}) if(see[d]>=0) {
                int q=see[d], other=cell[q], step=d==1?n:1;
                if(color[id]==color[other]) result.push_back({id,other,p,q,(q-p)/step,step});
            }
        }
        return result;
    }
    Plan plan(int capacity,int policies) const {
        auto edges=cables();
        Dsu possible(pos.size());
        for(const auto& e:edges) possible.unite(e.u,e.v);
        std::vector<int> sizes(pos.size());
        for(int i=0;i<int(pos.size());++i) sizes[i]=possible.size(i);
        Plan best;
        const auto noise=[](std::uint32_t h) {
            h^=h>>16; h*=0x7feb352du; h^=h>>15; h*=0x846ca68bu; return h^(h>>16);
        };
        std::vector<std::pair<int,int>> order(edges.size());
        for(int pass=0;pass<policies;++pass) {
            for(int i=0;i<int(edges.size());++i) {
                const auto& e=edges[i];
                int w=pass%4;
                int key=(w==0 ? 0: (w==1?2:w==2?5:11))*sizes[e.u]*128
                        -(w==3?2:12)*e.len*128;
                key+=noise(e.from*7549u+e.to*11213u+pass*52361u)%(pass<4?128:2048);
                order[i]={-key,i};
            }
            std::sort(order.begin(),order.end());
            Dsu used(pos.size());
            std::vector<unsigned char> occupied(n*n);
            Plan cur;
            for(auto [key,i]:order) {
                if(int(cur.links.size())>=capacity) break;
                const auto& e=edges[i];
                if(used.same(e.u,e.v)) continue;
                bool ok=true;
                for(int j=e.from+e.step;j<e.to;j+=e.step) if(occupied[j]) {
                    ok=false; break;
                }
                if(!ok) continue;
                for(int j=e.from+e.step;j<e.to;j+=e.step) occupied[j]=1;
                cur.score+=used.size(e.u)*used.size(e.v);
                used.unite(e.u,e.v);
                cur.links.push_back({e.from,e.to});
            }
            if(cur.score>best.score) best=std::move(cur);
        }
        return best;
    }
};

int main() {
    std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
    int n,k;
    if(!(std::cin>>n>>k)) return 1;
    Board board(n,k);
    for(int r=0;r<n;++r) {
        std::string row; std::cin>>row;
        for(int c=0;c<n;++c) if(row[c]!='0') {
            int id=board.pos.size(); board.pos.push_back(-1);
            board.color.push_back(row[c]-'0');
            board.set(id,r*n+c);
        }
    }
    const int budget=100*k;
    Plan best=board.plan(budget,32);
    std::vector<Step> best_moves;
#ifndef AHC013_STATIC_ONLY
    const auto original=board;
    const auto until=std::chrono::steady_clock::now()+std::chrono::milliseconds(900);
    std::mt19937 rng(73213673);
    constexpr std::array<int,4> dr{-1,1,0,0}, dc{0,0,-1,1};
    for(int restart=0;restart<12&&std::chrono::steady_clock::now()<until;++restart) {
        board=original;
        std::vector<Step> path;
        if(restart%2==1) {
            path=best_moves;
            const int cut=std::min(int(path.size()),budget/4+int(rng()%(budget/3)));
            path.resize(cut);
            for(auto m:path) board.set(board.cell[m.from],m.to);
        }
        Plan current=board.plan(budget-path.size(),4);
        for(int iter=0;iter<420&&int(path.size())<budget&&std::chrono::steady_clock::now()<until;++iter) {
            int chosen=-1,dest=-1,origin=-1;
            Plan evaluated;
            double best_priority=-1e300;
            for(int attempt=0;attempt<5;++attempt) {
                const int id=rng()%board.pos.size(), src=board.pos[id], d=rng()%4;
                int r=src/n+dr[d], c=src%n+dc[d];
                if(r<0||r>=n||c<0||c>=n||board.cell[r*n+c]>=0) continue;
                const int dst=r*n+c;
                board.set(id,dst);
                auto trial=board.plan(budget-path.size()-1,4);
                board.set(id,src);
                const double value=trial.score+std::generate_canonical<double,32>(rng)*.1;
                if(value>best_priority) {
                    best_priority=value; chosen=id; dest=dst; origin=src;
                    evaluated=std::move(trial);
                }
            }
            if(chosen<0) continue;
            const double temperature=2.+(1.-iter/420.0)*7.;
            // Select moves on the actual legal cable plan, not just visibility.
            if(evaluated.score<current.score && std::generate_canonical<double,32>(rng)>=
                    std::exp((evaluated.score-current.score-1)/temperature)) continue;
            board.set(chosen,dest);
            if(!path.empty()&&path.back().to==origin&&path.back().from==dest) {
                path.pop_back();
                current=board.plan(budget-path.size(),4);
            } else {
                path.push_back({origin,dest});
                current=std::move(evaluated);
            }
            if(current.score>best.score || (iter%31==0&&current.score*10>=best.score*8)) {
                auto adj=board.plan(budget-path.size(),32);
                if(adj.score>best.score) {best=std::move(adj);best_moves=path;}
            }
        }
    }
#endif
    auto print=[&](const auto& v) {
        std::cout<<v.size()<<'\n';
        for(auto e:v) std::cout<<e.from/n<<' '<<e.from%n<<' '<<e.to/n<<' '<<e.to%n<<'\n';
    };
    print(best_moves); print(best.links);
}
