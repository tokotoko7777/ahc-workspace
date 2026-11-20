#pragma GCC optimize("O3")
#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include "../../library/calendar-interval-packing.hpp"
#ifdef AHC023_SIBLING_FLOW
#include "../../library/parallel-interval-packing.hpp"
#endif

#if !defined(AHC023_PHASE_WINDOWS) && !defined(AHC023_CLOSED_WINDOWS) && !defined(AHC023_STATIC_LEAVES)
#define AHC023_PHASE_WINDOWS
#endif

struct Crop {int start,end;};
struct Task {int crop,cell,start;};

int main() {
    std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
    int t,h,w,entrance_row;
    if(!(std::cin>>t>>h>>w>>entrance_row)) return 1;
    const int n=h*w, root=entrance_row*w;
    std::vector<std::vector<int>> adj(n);
    auto join=[&](int a,int b){adj[a].push_back(b);adj[b].push_back(a);};
    for(int r=0;r+1<h;++r) {
        std::string s; std::cin>>s;
        for(int c=0;c<w;++c) if(s[c]=='0') join(r*w+c,(r+1)*w+c);
    }
    for(int r=0;r<h;++r) {
        std::string s;std::cin>>s;
        for(int c=0;c+1<w;++c) if(s[c]=='0') join(r*w+c,r*w+c+1);
    }
    int k;std::cin>>k;
    std::vector<Crop> crops(k);
    CalendarIntervalPacking pack(t);
    for(int i=0;i<k;++i) {
        auto& s=crops[i];std::cin>>s.start>>s.end;
        pack.add(i,s.start,s.end,s.end-s.start+1);
    }
    std::vector<int> dist(n,-1),order{root};
    dist[root]=0;
    for(std::size_t i=0;i<order.size();++i) for(int a:adj[order[i]]) if(dist[a]<0) {
        dist[a]=dist[order[i]]+1;
        order.push_back(a);
    }
    // For each BFS layer, use as few of the preceding layer's cells as
    // parents as possible by a greedy cover. More leaves remain plantable.
    std::vector<std::vector<int>> layers(dist[order.back()]+1), children(n);
    std::vector<int> parent(n,-1);
    for(int i:order) layers[dist[i]].push_back(i);
    for(int d=int(layers.size())-1;d>0;--d) {
        int left=layers[d].size();
        while(left) {
            int best=-1,gain=0;
            for(int v:layers[d-1]) {
                int cnt=0;
                for(int u:adj[v]) cnt+=dist[u]==d&&parent[u]<0;
                if(cnt>gain) best=v,gain=cnt;
            }
            if(best<0) return 1;
            for(int u:adj[best]) if(dist[u]==d&&parent[u]<0) {
                children[best].push_back(u);parent[u]=best;--left;
            }
        }
    }
    std::vector<unsigned char> used(k),empty(t+1);
    std::vector<std::vector<unsigned char>> events(n,empty);
    std::vector<Task> result;
#ifdef AHC023_PHASE_WINDOWS
    std::vector<std::vector<unsigned char>> planting(n,empty), harvesting(n,empty);
    auto assign=[&](int cell,const std::vector<unsigned char>& blackout,auto allowed) {
        auto plan=pack.best(blackout,[&](int job){return !used[job]&&allowed(job);});
#else
    auto assign=[&](int cell,const std::vector<unsigned char>& blackout) {
        auto plan=pack.best(blackout,[&](int job){return !used[job];});
#endif
        for(int job:plan.jobs) {
            used[job]=1;
            result.push_back({job,cell,crops[job].start});
            events[cell][crops[job].start]=1;
            events[cell][crops[job].end]=1;
#ifdef AHC023_PHASE_WINDOWS
            planting[cell][crops[job].start]=1;
            harvesting[cell][crops[job].end]=1;
#endif
        }
    };
    for(auto it=order.rbegin();it!=order.rend();++it) {
        int cell=*it;
        if(cell!=root&&children[cell].empty()) {
#ifdef AHC023_PHASE_WINDOWS
            assign(cell,empty,[](int){return true;});
#else
            assign(cell,empty);
#endif
        }
    }
#ifndef AHC023_STATIC_LEAVES
    for(auto it=order.rbegin();it!=order.rend();++it) {
        int cell=*it;
        if(cell==root||children[cell].empty()) continue;
#ifdef AHC023_PHASE_WINDOWS
        for(int child:children[cell]) for(int d=1;d<=t;++d) {
            planting[cell][d]|=planting[child][d];
            harvesting[cell][d]|=harvesting[child][d];
        }
        std::vector<int> plant_prefix(t+1), harvest_prefix(t+1);
        for(int d=1;d<=t;++d) {
            plant_prefix[d]=plant_prefix[d-1]+planting[cell][d];
            harvest_prefix[d]=harvest_prefix[d-1]+harvesting[cell][d];
        }
        // Same-phase ties are schedulable: descendants plant before the
        // ancestor; the ancestor harvests before descendants. In the opposite
        // phases the ancestor still has a crop, so those are blocked.
        assign(cell,empty,[&](int job) {
            const auto [s,e]=crops[job];
            return plant_prefix[e]==plant_prefix[s] &&
                   harvest_prefix[e-1]==harvest_prefix[s-1];
        });
#else
        auto descendant=empty;
        for(int child:children[cell]) for(int d=1;d<=t;++d)
            descendant[d]|=events[child][d];
        events[cell]=descendant;
        assign(cell,descendant);
#endif
    }
#endif
#if defined(AHC023_SIBLING_FLOW) && defined(AHC023_PHASE_WINDOWS)
    // Freeze all interior schedules and every leaf outside this sibling set.
    // Sibling leaves have precisely the same tree-ancestor path; their
    // access phase masks are identical and their intervals interchangeable.
    std::vector<std::vector<Task>> by_cell(n);
    for(auto task:result) by_cell[task.cell].push_back(task);
    for(int branch:order) {
        std::vector<int> leaves;
        for(int child:children[branch])
            if(children[child].empty()) leaves.push_back(child);
        if(leaves.size()<2) continue;
        auto plant_block=empty,harvest_block=empty;
        for(int v=branch;v>=0;v=parent[v]) for(auto task:by_cell[v]) {
            int end=crops[task.crop].end;
            for(int d=task.start+1; d<=end; ++d) plant_block[d]=1;
            for(int d=task.start; d<end; ++d) harvest_block[d]=1;
        }
        std::vector<int> latest(t+1);
        for(int d=1;d<=t;++d) latest[d]=plant_block[d]?latest[d-1]:d;
        int before=0;
        for(int cell:leaves) for(auto task:by_cell[cell]) {
            const auto [s,e]=crops[task.crop];
            before+=e-s+1;
            used[task.crop]=0;
        }
        std::vector<ParallelIntervalJob> jobs;
        std::vector<int> job_ids;
        for(int j=0;j<k;++j) if(!used[j]) {
            const auto [s,e]=crops[j];
            if(latest[s] && !harvest_block[e]) {
                jobs.push_back({latest[s],e,e-s+1});
                job_ids.push_back(j);
            }
        }
        auto plan=pack_parallel_intervals(t,leaves.size(),jobs);
        int after=0;
        for(const auto& lane:plan) for(int j:lane) after+=jobs[j].weight;
        if(after>before) for(int i=0;i<int(leaves.size());++i) {
            auto& lane=by_cell[leaves[i]];
            lane.clear();
            for(int j:plan[i]) lane.push_back({job_ids[j],leaves[i],jobs[j].start});
        }
        for(int cell:leaves) for(auto task:by_cell[cell]) used[task.crop]=1;
    }
    result.clear();
    for(const auto& lane:by_cell) for(auto task:lane) result.push_back(task);
#endif
    std::cout<<result.size()<<'\n';
    for(auto [crop,cell,start]:result)
        std::cout<<crop+1<<' '<<cell/w<<' '<<cell%w<<' '<<start<<'\n';
}
