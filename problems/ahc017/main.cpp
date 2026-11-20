#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <utility>
#include <vector>
#include "../../library/sparse-group-conflicts.hpp"

struct Edge { int a,b,w; };
struct Roads {
    int n,m,d,k;
    std::vector<Edge> e;
    std::vector<std::vector<std::pair<int,int>>> adj;
    std::vector<int> seen, bfs;
    int stamp=0;

    bool read() {
        if(!(std::cin>>n>>m>>d>>k)) return false;
        e.resize(m); adj.resize(n); seen.resize(n); bfs.resize(n);
        for(int id=0;id<m;++id) {
            auto& x=e[id]; std::cin>>x.a>>x.b>>x.w; --x.a;--x.b;
            adj[x.a].emplace_back(x.b,id); adj[x.b].emplace_back(x.a,id);
        }
        return true;
    }

    bool reaches(const std::vector<int>& day,int repair,int id) {
        auto [s,t,w]=e[id];
        ++stamp;
        seen[s]=stamp; bfs[0]=s; int tail=1;
        for(int i=0;i<tail;++i) {
            for(auto [v,j]:adj[bfs[i]]) {
                if(day[j]==repair||seen[v]==stamp) continue;
                if(v==t) return true;
                seen[v]=stamp; bfs[tail++]=v;
            }
        }
        return false;
    }

#ifndef AHC017_ENDPOINT_ONLY
    void shortest(int src,int target,int exclude,std::vector<int>& distance,
                  std::vector<int>& parent,std::vector<int>& settled) const {
        distance.assign(n,1000000000); parent.assign(n,-1);
        settled.clear();
        using Entry=std::pair<int,int>;
        std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> heap;
        distance[src]=0; heap.emplace(0,src);
        while(!heap.empty()) {
            auto [du,u]=heap.top();heap.pop();
            if(du!=distance[u]) continue;
            settled.push_back(u);
            if(u==target) return;
            for(auto [v,id]:adj[u]) if(id!=exclude && du+e[id].w<distance[v]) {
                distance[v]=du+e[id].w; parent[v]=id; heap.emplace(distance[v],v);
            }
        }
    }
    int other(int from,int id) const { return from^e[id].a^e[id].b; }

    void add_backups(SparseGroupConflicts& conflicts) const {
        std::vector<int> traffic(m),distance,parent,settled,paths(n);
        std::mt19937 sources(80978211);
        for(int i=0;i<20;++i) {
            auto source=sources()%n;
            shortest(source,-1,-1,distance,parent,settled);
            std::fill(paths.begin(),paths.end(),1);
            for(auto it=settled.rbegin();it!=settled.rend();++it) {
                int u=*it,id=parent[u];
                if(id<0) continue;
                traffic[id]+=paths[u]; paths[other(u,id)]+=paths[u];
            }
        }
        for(int id=0;id<m;++id) {
            const auto& edge=e[id];
            shortest(edge.a,edge.b,id,distance,parent,settled);
            const int extra=std::max(0,distance[edge.b]-edge.w);
            const int value=25+std::min(380,traffic[id]*8/std::max(1,n/20))
                              +std::min(96,extra/950);
            for(int v=edge.b;v!=edge.a&&parent[v]>=0;) {
                int b=parent[v];
                conflicts.add(id,b,value);
                v=other(v,b);
            }
        }
    }
#endif
};

int main() {
    std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
    Roads r;
    if(!r.read()) return 1;
    SparseGroupConflicts conflicts(r.m);
    for(const auto& out:r.adj) {
        for(int i=0;i<int(out.size());++i) for(int j=i+1;j<int(out.size());++j)
            conflicts.add(out[i].second,out[j].second,10+50/(int(out.size())-1));
    }
    std::mt19937 rng(4571301);
    std::vector<int> day(r.m,-1), load(r.d), order(r.m), options(r.d);
    std::iota(order.begin(),order.end(),0);
    std::iota(options.begin(),options.end(),0);
    bool connected=false;
    for(int attempt=0;attempt<5&&!connected;++attempt) {
        std::shuffle(order.begin(),order.end(),rng);
        std::stable_sort(order.begin(),order.end(),[&](int a,int b) {
            auto x=r.e[a],y=r.e[b];
            return std::min(r.adj[x.a].size(),r.adj[x.b].size())<
                   std::min(r.adj[y.a].size(),r.adj[y.b].size());
        });
        std::fill(day.begin(),day.end(),-1);
        std::fill(load.begin(),load.end(),0);
        connected=true;
        for(int id:order) {
            std::array<std::pair<int,int>,30> best{};
            for(int d=0;d<r.d;++d)
                best[d]={int(conflicts.move_delta(day,id,d))+3*load[d],d};
            std::sort(best.begin(),best.begin()+r.d);
            int pick=-1;
            for(int i=0;i<r.d;++i) {
                int d=best[i].second;
                if(load[d]==r.k) continue;
                day[id]=d;
                if(r.reaches(day,d,id)) { pick=d; break; }
            }
            if(pick<0) { connected=false; break; }
            ++load[pick];
        }
    }
    if(!connected) {
        // Always keep an output compliant with capacity, even if a day
        // happens to be disconnected; the statement assigns such paths 1e9.
        for(int i=0;i<r.m;++i) day[order[i]]=i%r.d;
    } else {
#ifndef AHC017_ENDPOINT_ONLY
        r.add_backups(conflicts);
#endif
        auto best=day;
        auto score=conflicts.cost(day), best_score=score;
        auto stop=std::chrono::steady_clock::now()+std::chrono::milliseconds(1750);
        constexpr int proposals=60000;
        for(int iter=0;iter<proposals;++iter) {
            if((iter&255)==0 && std::chrono::steady_clock::now()>=stop) break;
            int a=rng()%r.m, b=rng()%r.m;
            const int da=day[a], db=day[b];
            if(da==db) continue;
            const bool single=(rng()%7==0 && load[db]<r.k);
            const auto delta=single?conflicts.move_delta(day,a,db):conflicts.swap_delta(day,a,b);
#ifndef AHC017_ENDPOINT_ONLY
            const double temperature=7.+(1.0-iter/double(proposals))*40.;
#else
            const double temperature=3.+(1.0-iter/double(proposals))*14.;
#endif
            if(delta>0 && std::generate_canonical<double,32>(rng)>=std::exp(-delta/temperature)) continue;
            day[a]=db;
            if(!single) day[b]=da;
            if(!r.reaches(day,db,a)||(!single&&!r.reaches(day,da,b))) {
                day[a]=da;
                if(!single) day[b]=db;
                continue;
            }
            if(single) {--load[da];++load[db];}
            score+=delta;
            if(score<best_score) {best_score=score;best=day;}
        }
        day=std::move(best);
    }
    for(int i=0;i<r.m;++i) std::cout<<day[i]+1<<(i+1==r.m?'\n':' ');
}
