#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "../../library/paired-coverage-reward.hpp"

using Money=long long;
struct Transfer {int type, cell;};
struct Network {
    const int n,t,size;
    Money cash;
    PairedCoverageReward fares;
    std::vector<Transfer> actions;
    std::vector<std::array<int,4>> adj;
    std::vector<std::vector<int>> area,coverage;
    std::vector<int> tile;

    Network(int n_,int turns,Money initial,const std::vector<int>& homes,
            const std::vector<int>& offices,const std::vector<Money>& prices)
        :n(n_),t(turns),size(n*n),cash(initial),fares(prices),adj(size),area(size),
         coverage(size),tile(size,-1) {
        for(int p=0;p<size;++p) {
            const int r=p/n,c=p%n;
            adj[p]={c?p-1:-1,c+1<n?p+1:-1,r?p-n:-1,r+1<n?p+n:-1};
            for(int i=-2;i<=2;++i) for(int j=-2;j<=2;++j)
                if(std::abs(i)+std::abs(j)<=2 && 0<=r+i && r+i<n && 0<=c+j && c+j<n)
                    area[p].push_back((r+i)*n+c+j);
        }
        for(int i=0;i<int(homes.size());++i) {
            for(int p:area[homes[i]]) coverage[p].push_back(2*i);
            for(int p:area[offices[i]]) coverage[p].push_back(2*i+1);
        }
    }
    int dist(int a,int b) const {return std::abs(a/n-b/n)+std::abs(a%n-b%n);}
    void step(int type=-1,int cell=-1) {
        if(type>=0) {
            const int cost=type?100:5000;
            assert(cash>=cost);
            cash-=cost;
            tile[cell]=type;
            if(type==0) fares.add(coverage[cell]);
        }
        cash+=fares.value();
        actions.push_back({type,cell});
        assert(int(actions.size())<=t);
    }
    void pay_step(int type,int cell) {
        const int cost=type?100:5000;
        while(cash<cost) {
            assert(fares.value()>0);
            step();
        }
        step(type,cell);
    }
    int rail(int a,int b,int c) const {
        constexpr std::array<int,16> shapes{0,0,0,1,0,4,5,0,0,3,6,0,2,0,0,0};
        int mask=0;
        for(int d=0;d<4;++d) if(adj[b][d]==a || adj[b][d]==c) mask|=1<<d;
        assert(shapes[mask]>0);
        return shapes[mask];
    }
    std::vector<int> shortest_first(int a,int b) const {
        std::vector<int> path{a};
        while(a/n!=b/n) a+=(a/n<b/n?n:-n),path.push_back(a);
        while(a%n!=b%n) a+=(a%n<b%n?1:-1),path.push_back(a);
        return path;
    }
    void build(const std::vector<int>& path) {
        for(int j=1;j+1<int(path.size());++j)
            pay_step(rail(path[j-1],path[j],path[j+1]),path[j]);
        pay_step(0,path.back());
    }
    bool start(const std::vector<int>& homes,const std::vector<int>& offices,
               const std::vector<Money>& prices) {
        // Only 13*13 possible station pairs per commuter; accumulating exact
        // fare at those pairs avoids scoring millions of pairs independently.
        std::vector<int> earnings(size*size);
        for(int i=0;i<int(homes.size());++i)
            for(int a:area[homes[i]]) for(int b:area[offices[i]])
                earnings[std::min(a,b)*size+std::max(a,b)]+=prices[i];
        Money best=0;
        int from=-1,to=-1,want=0;
        for(int a=0;a<size;++a) for(int b=a+1;b<size;++b) {
            if(!earnings[a*size+b]) continue;
            const int d=dist(a,b), cost=10000+100*(d-1);
            if(cost>cash || d+1>t) continue;
            Money value=1LL*(t-d)*earnings[a*size+b]-cost;
            if(value>best) best=value,from=a,to=b,want=earnings[a*size+b];
        }
        if(from<0) return false;
        step(0,from);
        build(shortest_first(from,to));
        assert(fares.value()==want);
        return true;
    }
    int finish_day(int rails) const {
        Money money=cash,income=fares.value();
        int day=actions.size();
        for(int i=0;i<=rails;++i) {
            const int cost=i==rails?5000:100;
            if(money<cost) {
                if(!income) return t+1;
                Money wait=(cost-money+income-1)/income;
                day+=wait;money+=wait*income;
            }
            ++day;
            if(day>t) return day;
            money+=income-cost;
        }
        return day;
    }
    bool extend() {
        std::vector<int> dist(size,-1),prev(size,-1),q;
        for(int i=0;i<size;++i) if(tile[i]==0) {
            dist[i]=0;
            q.push_back(i);
        }
        for(int j=0;j<int(q.size());++j) {
            int v=q[j];
            for(int p:adj[v]) if(p>=0 && tile[p]==-1 && dist[p]<0) {
                dist[p]=dist[v]+1;
                prev[p]=v;
                q.push_back(p);
            }
        }
        Money best=0;
        int at=-1;
        for(int p=0;p<size;++p) {
            if(tile[p]==0 || (tile[p]==-1 && dist[p]<0) || coverage[p].empty()) continue;
            Money delta=fares.delta(coverage[p]);
            if(!delta) continue;
            const int rails=std::max(0,dist[p]-1), day=finish_day(rails);
            if(day>t) continue;
            const Money gain=delta*(t-day+1)-5000-100*rails;
            if(gain>best) best=gain,at=p;
        }
        if(at<0) return false;
        std::vector<int> path{at};
        if(tile[at]==-1) for(int p=prev[at];p>=0;p=prev[p]) path.push_back(p);
        std::reverse(path.begin(),path.end());
        build(path);
        return true;
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n,m,t;Money initial;
    if(!(std::cin>>n>>m>>initial>>t)) return 1;
    std::vector<int> home(m),office(m);
    std::vector<Money> fares(m);
    for(int i=0,a,b,c,d;i<m;++i) {
        std::cin>>a>>b>>c>>d;
        home[i]=a*n+b;office[i]=c*n+d;
        fares[i]=std::abs(a-c)+std::abs(b-d);
    }
    Network model(n,t,initial,home,office,fares);
    bool started=model.start(home,office,fares);
#ifndef AHC043_ONE_LINE
    if(started) while(model.extend()) {}
#else
    (void)started;
#endif
    while(int(model.actions.size())<t) model.step();
    for(auto [a,c]:model.actions) {
        if(a<0) std::cout<<"-1\n";
        else std::cout<<a<<' '<<c/n<<' '<<c%n<<'\n';
    }
    std::cerr<<"Model = "<<model.cash<<'\n';
}
