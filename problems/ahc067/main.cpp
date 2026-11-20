#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include "../../library/undirected-bridges.hpp"

using Clock=std::chrono::steady_clock;
struct RNG {
    uint32_t value;
    uint32_t next() {value^=value<<13;value^=value>>17;value^=value<<5;return value;}
};
struct Layout {
    int steps=0;
    std::vector<std::pair<int,int>> doors,switches;
};
struct Problem {
    int N,M,K,area,dest;
    std::vector<std::string> map;
    std::vector<std::pair<int,int>> edges;
    std::vector<std::vector<std::pair<int,int>>> adjacency;
    bool read() {
        if(!(std::cin>>N>>M>>K)) return false;
        area=N*N;dest=area-1;
        map.resize(N);adjacency.resize(area);
        for(auto& s:map) std::cin>>s;
        for(int x=0;x<N;++x) for(int y=0;y<N;++y) if(map[x][y]=='.')
            for(auto [dx,dy]:std::array<std::pair<int,int>,2>{{{1,0},{0,1}}}) {
                int i=x+dx,j=y+dy;
                if(i>=N || j>=N || map[i][j]=='#') continue;
                int a=x*N+y,b=i*N+j,id=edges.size();
                edges.push_back({a,b});
                adjacency[a].push_back({b,id});adjacency[b].push_back({a,id});
            }
        return true;
    }
    struct Search {
        std::vector<int> dist,parent;
    };
    Search bfs(int from,const std::vector<bool>& closed,bool fast=false) const {
        Search s{std::vector<int>(area,-1),std::vector<int>(area,-1)};
        std::array<int,400> queue;
        int head=0,tail=1;
        s.dist[from]=0;queue[0]=from;
        while(head<tail) {
            int at=queue[head++];
            if(fast && at==dest) break;
            for(auto [to,e]:adjacency[at]) if(!closed[e] && s.dist[to]<0) {
                s.dist[to]=s.dist[at]+1;s.parent[to]=e;
                queue[tail++]=to;
            }
        }
        return s;
    }
    int opposite(int e,int at) const {return edges[e].first^edges[e].second^at;}
    std::vector<int> path(const Search& s) const {
        std::vector<int> list;
        if(s.dist[dest]<0) return list;
        for(int a=dest;a;) {
            int e=s.parent[a];list.push_back(e);a=opposite(e,a);
        }
        std::reverse(list.begin(),list.end());
        return list;
    }
    std::vector<bool> bridges(const std::vector<bool>& closed) const {
        std::vector<int> original;
        std::vector<std::pair<int,int>> active;
        for(int i=0;i<int(edges.size());++i) if(!closed[i]) {
            original.push_back(i);active.push_back(edges[i]);
        }
        auto flags=undirected_bridges(area,active);
        std::vector<bool> out(edges.size());
        for(unsigned i=0;i<flags.size();++i) if(flags[i]) out[original[i]]=true;
        return out;
    }
    void thin(std::vector<bool>& closed,int limit,RNG& rng,Clock::time_point stop) const {
        int total=std::count(closed.begin(),closed.end(),true);
        while(total<limit && Clock::now()<stop) {
            auto s=bfs(0,closed,true);
            auto toward=path(s);
            auto mandatory=bridges(closed);
            int best=-1,score=-1;
            uint32_t tie=0;
            for(int e:toward) if(!mandatory[e]) {
                closed[e]=true;
                auto next=bfs(0,closed,true);
                closed[e]=false;
                uint32_t r=rng.next();
                if(next.dist[dest]>score || (next.dist[dest]==score && r>tie))
                    best=e,score=next.dist[dest],tie=r;
            }
            if(best<0 || score<0) break;
            closed[best]=true;++total;
        }
    }
    Layout plain(const std::vector<bool>& closed) const {
        Layout p;
        p.steps=bfs(0,closed,true).dist[dest];
        for(int e=0;e<int(edges.size());++e) if(closed[e]) p.doors.push_back({e,2*K-1});
        return p;
    }
    Layout forced(const std::vector<bool>& closed) const {
        Layout out=plain(closed);
        int budget=std::min(K-1,M-int(out.doors.size()));
        if(!budget) return out;
        auto bridge=bridges(closed);
        struct Gate {int edge,left,right;};
        std::vector<Gate> gates{{-1,0,0}};
        int at=0;
        auto isolated=closed;
        for(int e:path(bfs(0,closed,true))) {
            int next=opposite(e,at);
            if(bridge[e]) {
                gates.push_back({e,at,next});
                isolated[e]=true;
            }
            at=next;
        }
        const int size=gates.size();
        std::vector<int> zone(area,-1);
        std::vector<Search> from,to;
        for(int k=0;k<size;++k) {
            from.push_back(bfs(gates[k].right,closed));
            to.push_back(bfs(gates[k].left,closed));
            const auto d=bfs(gates[k].right,isolated);
            for(int i=0;i<area;++i) if(d.dist[i]>=0) zone[i]=k;
        }
        struct Arc {int reward=-1,sw=-1;};
        std::vector<std::vector<Arc>> arcs(size,std::vector<Arc>(size));
        for(int a=0;a<size;++a) for(int b=a+1;b<size;++b) {
            const int direct=from[a].dist[gates[b].left];
            for(int i=0;i<area;++i) if(a<=zone[i] && zone[i]<b) {
                const int delta=1+from[a].dist[i]+to[b].dist[i]-direct;
                if(delta>arcs[a][b].reward) arcs[a][b]={delta,i};
            }
        }
        std::vector<std::vector<int>> dp(budget+1,std::vector<int>(size,-1));
        auto pred=dp;
        dp[0][0]=0;
        int last=0,used=0,gain=0;
        for(int count=0;count<budget;++count)
            for(int a=0;a<size;++a) if(dp[count][a]>=0)
                for(int b=a+1;b<size;++b) {
                    const int v=dp[count][a]+arcs[a][b].reward;
                    if(arcs[a][b].sw<0 || v<=dp[count+1][b]) continue;
                    dp[count+1][b]=v;pred[count+1][b]=a;
                    if(v>gain) gain=v,last=b,used=count+1;
                }
        out.steps+=gain;
        for(int c=used;c;--c) {
            int a=pred[c][last];
            out.doors.push_back({gates[last].edge,2*(c-1)+1});
            out.switches.push_back({arcs[a][last].sw,c-1});
            last=a;
        }
        return out;
    }
    Layout exchange(Layout record,std::vector<bool> closed,bool optimize_detours,
                    Clock::time_point stop) const {
        RNG rng{34164481};
        int current=optimize_detours ? forced(closed).steps : plain(closed).steps;
        for(int turn=0;turn<36 && Clock::now()<stop;++turn) {
            std::vector<int> occupied;
            for(int e=0;e<int(edges.size());++e) if(closed[e]) occupied.push_back(e);
            if(occupied.empty()) break;
            int open=occupied[rng.next()%occupied.size()];
            closed[open]=false;
            auto on_path=path(bfs(0,closed,true));
            auto mandatory=bridges(closed);
            std::vector<int> options;
            for(int e:on_path) if(e!=open && !mandatory[e]) options.push_back(e);
            if(options.empty() || rng.next()%3==0) {
                options.clear();
                for(int e=0;e<int(edges.size());++e)
                    if(e!=open && !closed[e] && !mandatory[e]) options.push_back(e);
            }
            if(options.empty()) {
                closed[open]=true;continue;
            }
            int close=options[rng.next()%options.size()];
            closed[close]=true;
            Layout trial=forced(closed);
            int value=optimize_detours ? trial.steps : plain(closed).steps;
            if(trial.steps>record.steps) record=std::move(trial);
            if(value>=current) current=value;
            else closed[close]=false,closed[open]=true;
        }
        return record;
    }
    Layout solve() const {
        auto stop=Clock::now()+std::chrono::milliseconds(1500);
        std::vector<bool> best_closed(edges.size());
        Layout best=plain(best_closed);
        for(int trial=0;trial<7 && Clock::now()<stop;++trial) {
            RNG rng{8478347u+734321u*unsigned(trial)};
            std::vector<bool> closed(edges.size());
            thin(closed,M-K+1,rng,stop);
#ifndef AHC067_STATIC
            Layout trial_gate=forced(closed);
            if(trial_gate.steps>best.steps) best=std::move(trial_gate),best_closed=closed;
#endif
            thin(closed,M,rng,stop);
            Layout p=plain(closed);
            if(p.steps>best.steps) best=std::move(p),best_closed=closed;
        }
#if defined(AHC067_SWAP_STATIC)
        best=exchange(std::move(best),std::move(best_closed),false,stop);
#elif !defined(AHC067_STATIC) && !defined(AHC067_NO_EXCHANGE)
        best=exchange(std::move(best),std::move(best_closed),true,stop);
#endif
        return best;
    }
    void print(const Layout& p) const {
        std::cout<<p.doors.size()<<'\n';
        for(auto [e,kind]:p.doors) {
            auto [a,b]=edges[e];
            std::cout<<(b-a==1?1:0)<<' '<<a/N<<' '<<a%N<<' '<<kind<<'\n';
        }
        std::cout<<p.switches.size()<<'\n';
        for(auto [p,kind]:p.switches) std::cout<<p/N<<' '<<p%N<<' '<<kind<<'\n';
        std::cerr<<"Steps = "<<p.steps<<'\n';
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Problem p;
    if(p.read()) p.print(p.solve());
}
