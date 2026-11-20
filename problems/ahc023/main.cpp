#pragma GCC optimize("O3")
#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include "../../library/calendar-interval-packing.hpp"

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
    auto assign=[&](int cell,const std::vector<unsigned char>& blackout) {
        auto plan=pack.best(blackout,[&](int job){return !used[job];});
        for(int job:plan.jobs) {
            used[job]=1;
            result.push_back({job,cell,crops[job].start});
            events[cell][crops[job].start]=1;
            events[cell][crops[job].end]=1;
        }
    };
    for(auto it=order.rbegin();it!=order.rend();++it) {
        int cell=*it;
        if(cell!=root&&children[cell].empty()) assign(cell,empty);
    }
#ifndef AHC023_STATIC_LEAVES
    for(auto it=order.rbegin();it!=order.rend();++it) {
        int cell=*it;
        if(cell==root||children[cell].empty()) continue;
        auto descendant=empty;
        for(int child:children[cell]) for(int d=1;d<=t;++d)
            descendant[d]|=events[child][d];
        events[cell]=descendant;
        assign(cell,descendant);
    }
#endif
    std::cout<<result.size()<<'\n';
    for(auto [crop,cell,start]:result)
        std::cout<<crop+1<<' '<<cell/w<<' '<<cell%w<<' '<<start<<'\n';
}
