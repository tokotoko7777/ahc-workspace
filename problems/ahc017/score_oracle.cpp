// Independent all-pairs scorer and capacity / connectivity verifier.
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
struct Link {int a,b,w,day;};
struct Arc {int v,w;};

int main(int argc,char**argv) {
    if(argc!=3) return 1;
    std::ifstream in(argv[1]), answer(argv[2]);
    int n,m,d,k;
    if(!(in>>n>>m>>d>>k)) return 1;
    std::vector<int> load(d+1);
    std::vector<Link> edges(m);
    for(auto& link:edges) {
        if(!(in>>link.a>>link.b>>link.w) || !(answer>>link.day)
           || link.day<1 || link.day>d || ++load[link.day]>k) return 2;
        --link.a;--link.b;
    }
    std::string trailing;
    if(answer>>trailing) return 2;
    std::vector<std::vector<Arc>> adj(n);
    const int inf=1'000'000'000;
    using Entry=std::pair<int,int>;
    std::vector<int> dist(n);
    std::int64_t initial=0,total=0,disconnected_days=0;
    for(int blocked=0;blocked<=d;++blocked) {
        for(auto& row:adj) row.clear();
        for(auto e:edges) if(e.day!=blocked) {
            adj[e.a].push_back({e.b,e.w}); adj[e.b].push_back({e.a,e.w});
        }
        std::int64_t daily_sum=0;
        for(int source=0;source+1<n;++source) {
            std::fill(dist.begin(),dist.end(),inf);
            std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> q;
            dist[source]=0;q.emplace(0,source);
            while(!q.empty()) {
                auto [du,u]=q.top();q.pop();
                if(dist[u]!=du) continue;
                for(auto [v,w]:adj[u]) if(du+w<dist[v]) {
                    dist[v]=du+w; q.emplace(dist[v],v);
                }
            }
            if(blocked && !source && std::count(dist.begin(),dist.end(),inf)) ++disconnected_days;
            for(int j=source+1;j<n;++j) daily_sum+=dist[j];
        }
        if(!blocked) initial=daily_sum;
        else total+=daily_sum-initial;
    }
    const std::int64_t den=std::int64_t(d)*n*(n-1)/2;
    const __int128 scaled=static_cast<__int128>(total)*1000;
    const auto rounded=static_cast<std::int64_t>((2*scaled+den)/(2*den));
    std::cout<<rounded<<' '<<disconnected_days<<'\n';
}
