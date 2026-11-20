#pragma GCC optimize("O3")
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>
#include "../../library/graph-bfs.hpp"
#include "../../library/greedy-path-forest.hpp"

#if !defined(AHC036_NO_WINDOW_ROUTE) && !defined(AHC036_SINGLETON) && !defined(AHC036_WINDOW_ROUTE)
#define AHC036_WINDOW_ROUTE
#endif

using Clock=std::chrono::steady_clock;

struct Action {int length,a,b;};
struct Plan {
    std::vector<int> source;
    std::vector<Action> actions;
    int signals=0;
};
struct Trip {
    int n,la,lb;
    std::vector<int> walk;
    std::vector<std::vector<int>> uses;
    void prepare() {
        uses.resize(n);
        for(int i=0;i<int(walk.size());++i) uses[walk[i]].push_back(i);
    }
    Plan run(std::vector<int> src,bool multi,Clock::time_point deadline={}) const {
        const int count=walk.size();
        Plan plan{std::move(src),{},0};
        std::vector<std::vector<int>> where(n);
        for(int j=0;j<la;++j) where[plan.source[j]].push_back(j);
        std::vector<int> b(lb,-1),numbers(n),used(n);
        std::vector<int> lens{2,4,8,12,16,lb};
        std::sort(lens.begin(),lens.end());
        lens.erase(std::unique(lens.begin(),lens.end()),lens.end());
        for(int i=0;i<count;++i) {
            if(multi && i%16==0 && Clock::now()>=deadline) {
                plan.signals=1'000'000;
                return plan; // abandon partial plan; singleton fallback is complete
            }
            const int v=walk[i];
            ++used[v];
            if(!numbers[v]) {
                int slot=0, furthest=-1;
                for(int j=0;j<lb;++j) {
                    int u=b[j];
                    int next=(u<0 || used[u]==int(uses[u].size()))?count:uses[u][used[u]];
                    if(next>furthest) furthest=next,slot=j;
                }
                Action chosen{1,where[v][0],slot};
                if(multi) {
                    int end=std::min(count,i+2*lb);
                    struct Window {int value,l,p;};
                    std::vector<Window> windows;
                    std::vector<int> local(n);
                    auto add=[&](int l,int pa) {
                        for(int k=0;k<l;++k) ++local[plan.source[pa+k]];
                        int score=0;
                        for(int j=i;j<end;++j) score+=local[walk[j]]>0 ? end-j : 0;
                        for(int k=0;k<l;++k) --local[plan.source[pa+k]];
                        windows.push_back({score,l,pa});
                    };
                    const auto& positions=where[v];
                    const int anchors=std::min(32,int(positions.size()));
                    for(int k=0;k<anchors;++k) {
                        const int index=positions[std::size_t(k)*positions.size()/anchors];
                        for(int l:lens) if(l<=lb)
                            for(int offset:{0,l/2,l-1}) add(l,std::clamp(index-offset,0,la-l));
                    }
                    auto order=[](Window a,Window b) {
                        return std::tie(a.value,a.l,a.p)>std::tie(b.value,b.l,b.p);
                    };
                    std::sort(windows.begin(),windows.end(),order);
                    windows.erase(std::unique(windows.begin(),windows.end(),[](Window a,Window b) {
                        return a.l==b.l && a.p==b.p;
                    }),windows.end());
                    if(windows.size()>8) windows.resize(8);
                    int best=-1;
                    auto evaluate=[&](int l,int pa,int pb) {
                        for(int k=0;k<l;++k) {
                            if(b[pb+k]>=0) --numbers[b[pb+k]];
                            ++numbers[plan.source[pa+k]];
                        }
                        int prefix=0,gain=0;
                        while(i+prefix<end && numbers[walk[i+prefix]]>0) ++prefix;
                        for(int j=i;j<end;++j) gain+=(numbers[walk[j]]>0)?end-j:0;
                        int value=4096*prefix+gain;
                        for(int k=0;k<l;++k) {
                            if(b[pb+k]>=0) ++numbers[b[pb+k]];
                            --numbers[plan.source[pa+k]];
                        }
                        if(value>best) best=value,chosen={l,pa,pb};
                    };
                    evaluate(chosen.length,chosen.a,chosen.b);
                    for(auto [priority,l,pa]:windows)
                        for(int pb=0;pb+l<=lb;++pb) evaluate(l,pa,pb);
                }
                for(int k=0;k<chosen.length;++k) {
                    int& entry=b[chosen.b+k];
                    if(entry>=0) --numbers[entry];
                    entry=plan.source[chosen.a+k];
                    ++numbers[entry];
                }
                ++plan.signals;
                plan.actions.push_back(chosen);
            }
            plan.actions.push_back({0,v,0});
        }
        return plan;
    }
    std::vector<int> layout() const {
        std::vector<int> weight(n*n),located(n);
        int before=0;
        for(int v:walk) {
            ++weight[before*n+v];++weight[v*n+before];before=v;
        }
        std::vector<std::tuple<int,int,int>> order;
        for(int a=0;a<n;++a) for(int b=a+1;b<n;++b)
            if(weight[a*n+b]) order.emplace_back(weight[a*n+b],a,b);
        std::sort(order.rbegin(),order.rend());
        std::vector<std::pair<int,int>> edges;
        for(auto [w,a,b]:order) edges.emplace_back(a,b);
        auto paths=greedy_path_forest(n,edges);
        std::stable_sort(paths.begin(),paths.end(),[&](const auto& a,const auto& b) {
            return a.size()>b.size();
        });
        std::vector<int> source;
        for(const auto& p:paths) for(int a:p) {
            located[a]=source.size();source.push_back(a);
        }
        struct Fragment {int score,at;};
        std::vector<Fragment> f;
        for(int i=0;i+lb<=int(walk.size());i+=2) {
            int gain=0;
            for(int k=1;k<lb;++k) {
                int a=walk[i+k-1],b=walk[i+k],d=std::abs(located[a]-located[b]);
                if(d>1) gain+=std::min(d,lb)*weight[a*n+b];
            }
            if(gain) f.push_back({gain,i});
        }
        std::sort(f.begin(),f.end(),[](Fragment a,Fragment b){return a.score>b.score;});
        std::vector<bool> used_edge(n*n);
        for(auto [quality,i]:f) {
            if(int(source.size())>=la) break;
            int novel=0;
            for(int k=1;k<lb;++k) {
                int a=walk[i+k-1],b=walk[i+k];
                novel+=std::abs(located[a]-located[b])>1 && !used_edge[a*n+b];
            }
            if(novel<2) continue;
            for(int k=0;k<lb && int(source.size())<la;++k) {
                source.push_back(walk[i+k]);
                if(k) {
                    int a=walk[i+k-1],b=walk[i+k];
                    used_edge[a*n+b]=used_edge[b*n+a]=true;
                }
            }
        }
        source.resize(la,0);
        return source;
    }
};
#if defined(AHC036_WINDOW_ROUTE) || defined(AHC036_FORCE_WINDOW)
// Graph of projected full-buffer windows. A signal update may enter any
// connected green component containing or adjacent to the current vertex.
static Plan window_route(const std::vector<std::vector<int>>& adj,
                         const std::vector<int>& targets,std::vector<int> source,
                         int lb,Clock::time_point deadline) {
    const int n=adj.size(), windows=int(source.size())-lb+1;
    constexpr int invalid=1'000'000;
    Plan plan{std::move(source),{},0};
    auto fail=[&]() -> Plan {plan.signals=invalid;return std::move(plan);};
    struct Component {int window;std::vector<int> vertices;};
    std::vector<Component> comp;
    std::vector<std::vector<int>> incident(n);
    std::vector<int> selected(n,-1),seen(n,-1),for_city(n,-1);
    for(int p=0;p<windows;++p) {
        for(int i=0;i<lb;++i) selected[plan.source[p+i]]=p;
        for(int i=0;i<lb;++i) {
            int v=plan.source[p+i];
            if(seen[v]==p) continue;
            const int id=comp.size();
            comp.push_back({p,{v}});
            seen[v]=p;
            auto& queue=comp.back().vertices;
            for(int j=0;j<int(queue.size());++j)
                for(int u:adj[queue[j]]) if(selected[u]==p && seen[u]!=p)
                    queue.push_back(u),seen[u]=p;
            for(int x:queue) {
                if(for_city[x]!=id) incident[x].push_back(id),for_city[x]=id;
                for(int u:adj[x]) if(for_city[u]!=id)
                    incident[u].push_back(id),for_city[u]=id;
            }
        }
    }
    for(auto& e:incident) std::stable_sort(e.begin(),e.end(),[&](int a,int b){
        return comp[a].vertices.size()>comp[b].vertices.size();
    });
    std::vector<int> seen_comp(comp.size(),-1), prior(n), by(n), member(n,-1), search;
    int current=0,active=-1,mark=0;
    auto walk=[&](int goal) {
        if(current==goal) return true;
        if(active<0) return false;
        ++mark;
        for(int i=0;i<lb;++i) member[plan.source[active+i]]=mark;
        std::vector<int> q{current};
        std::vector<int> trace(n,-1);
        trace[current]=current;
        for(int at=0;at<int(q.size()) && trace[goal]<0;++at)
            for(int u:adj[q[at]]) if(member[u]==mark && trace[u]<0)
                trace[u]=q[at],q.push_back(u);
        if(trace[goal]<0) return false;
        std::vector<int> rev;
        for(int v=goal;v!=current;v=trace[v]) rev.push_back(v);
        for(auto it=rev.rbegin();it!=rev.rend();++it) plan.actions.push_back({0,*it,0});
        current=goal;
        return true;
    };
    for(int i=0;i<int(targets.size());++i) {
        if(Clock::now()>=deadline) return fail();
        const int goal=targets[i];
        if(goal==current) continue;
        std::fill(prior.begin(),prior.end(),-1);
        prior[current]=current;
        by[current]=-1;
        search={current};
        // First flood-fill cities already reachable without a signal update.
        if(active>=0) {
            ++mark;
            for(int j=0;j<lb;++j) member[plan.source[active+j]]=mark;
            for(int j=0;j<int(search.size());++j)
                for(int u:adj[search[j]]) if(member[u]==mark && prior[u]<0) {
                    prior[u]=current;by[u]=-1;search.push_back(u);
                }
        }
        // Each expansion costs one update; overlapping/adjacent windows can
        // change both the best signal sequence and the underlying travel path.
        for(int j=0;j<int(search.size()) && prior[goal]<0;++j) {
            int v=search[j];
            for(int id:incident[v]) if(seen_comp[id]!=i) {
                seen_comp[id]=i;
                for(int u:comp[id].vertices) if(prior[u]<0) {
                    prior[u]=v;by[u]=id;search.push_back(u);
                }
            }
        }
        if(prior[goal]<0) return fail();
        std::vector<int> reverse;
        for(int v=goal;v!=current;v=prior[v]) reverse.push_back(v);
        for(auto it=reverse.rbegin();it!=reverse.rend();++it) {
            const int v=*it,id=by[v];
            if(id>=0) {
                active=comp[id].window;
                plan.actions.push_back({lb,active,0});++plan.signals;
            }
            if(!walk(v) || plan.actions.size()>100000) return fail();
        }
    }
    return plan;
}
#endif

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    const auto deadline=Clock::now()+std::chrono::milliseconds(2450);
    Trip trip;
    int m,t;
    if(!(std::cin>>trip.n>>m>>t>>trip.la>>trip.lb)) return 1;
    std::vector<std::vector<int>> adj(trip.n);
    for(int i=0,a,b;i<m;++i) {
        std::cin>>a>>b;adj[a].push_back(b);adj[b].push_back(a);
    }
    std::vector<int> targets(t);
    for(int i=0,last=0;i<t;++i) {
        int target;std::cin>>target;targets[i]=target;
        auto p=graph_bfs(adj,last).path_to(target);
        trip.walk.insert(trip.walk.end(),p.begin()+1,p.end());
        last=target;
    }
    trip.prepare();
    std::vector<int> source(trip.la);
    std::iota(source.begin(),source.begin()+trip.n,0);
    Plan best=trip.run(std::move(source),false);
#ifndef AHC036_SINGLETON
    auto layout=trip.layout();
    Plan batched=trip.run(layout,true,deadline);
    if(batched.signals<best.signals) best=std::move(batched);
#if defined(AHC036_WINDOW_ROUTE) || defined(AHC036_FORCE_WINDOW)
    Plan routed=window_route(adj,targets,std::move(layout),trip.lb,deadline);
#ifdef AHC036_FORCE_WINDOW
    if(routed.signals<1'000'000) best=std::move(routed);
#else
    if(routed.signals<best.signals) best=std::move(routed);
#endif
#endif
#endif
    for(int i=0;i<trip.la;++i) std::cout<<best.source[i]<<(i+1==trip.la?'\n':' ');
    for(auto [l,a,b]:best.actions) {
        if(l) std::cout<<"s "<<l<<' '<<a<<' '<<b<<'\n';
        else std::cout<<"m "<<a<<'\n';
    }
    std::cerr<<"Model = "<<best.signals<<'\n';
}
