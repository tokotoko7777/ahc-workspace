#pragma GCC optimize("O3")
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <limits>
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
#if defined(AHC043_SEED_PORTFOLIO) || defined(AHC043_GEOMETRY_CONTROL)
using Clock=std::chrono::steady_clock;
struct Start {int a,b,rate,dist;long long base;};

static std::vector<Start> seeds(const Network& model,const std::vector<int>& home,
                                const std::vector<int>& office,const std::vector<Money>& fare) {
    const int n2=model.size;
    std::vector<int> earns(n2*n2);
    for(int i=0;i<int(home.size());++i)
        for(int a:model.area[home[i]]) for(int b:model.area[office[i]])
            earns[std::min(a,b)*n2+std::max(a,b)]+=fare[i];
    std::vector<Start> possible;
    for(int a=0;a<n2;++a) for(int b=a+1;b<n2;++b) {
        const int rate=earns[a*n2+b];
        if(!rate) continue;
        const int d=model.dist(a,b),cost=10000+100*(d-1);
        if(cost>model.cash || d+1>model.t) continue;
        long long base=1LL*rate*(model.t-d)-cost;
        if(base>0) possible.push_back({a,b,rate,d,base});
    }
    if(possible.empty()) return {};
    std::vector<long long> exposure(n2);
    for(int p=0;p<n2;++p)
        for(int e:model.coverage[p]) exposure[p]+=fare[e/2];

    auto top=[&](int profile,const std::vector<Start>& prior,bool spatial) {
        int idx=-1;
        long long value=std::numeric_limits<long long>::lowest();
        for(int i=0;i<int(possible.size());++i) {
            const auto& s=possible[i];
            if(spatial && std::any_of(prior.begin(),prior.end(),[&](const Start& q){
                const int match=std::min(model.dist(s.a,q.a)+model.dist(s.b,q.b),
                                         model.dist(s.a,q.b)+model.dist(s.b,q.a));
                return match<6;
            })) continue;
            const auto cost=10000+100*(s.dist-1);
            const auto saved=model.cash-cost;
            const auto lag=std::max(0LL,5000-saved-s.rate);
            auto wait=(lag+s.rate-1)/s.rate;
            const int delay=s.dist+1+std::min<Money>(1000,wait);
            const auto breadth=exposure[s.a]+exposure[s.b];
            const auto uncovered=std::max(0LL,breadth-2LL*s.rate);
            long long merit=s.base;
            switch(profile) {
                case 1: merit=1'000'000LL*s.rate/(16+delay);break;
                case 2: merit-=5'000LL*delay;break;
                case 3: merit+=200*uncovered;break;
                case 4: merit+=500*uncovered;break;
                case 5: merit+=500*uncovered-3'000LL*delay;break;
                case 6: merit+=300*std::min(exposure[s.a],exposure[s.b]);break;
            }
            if(merit>value) value=merit,idx=i;
        }
        return idx;
    };
    const auto first=possible[top(0,{},false)];
    std::vector<Start> result{first,first};
    for(int p=1;p<=6;++p) {
        const int idx=top(p,result,true);
        result.push_back(idx>=0 ? possible[idx] : first);
    }
    return result;
}
static std::vector<int> initial_path(Network& model,const Start& s,int shape) {
    if(shape==0) return model.shortest_first(s.a,s.b);
    if(shape==1) {
        auto path=model.shortest_first(s.b,s.a);
        std::reverse(path.begin(),path.end());
        return path;
    }
    // All paths remain Manhattan-shortest; score reachable minor station
    // upgrades along a partial ordering of the rectangle between the ends.
    model.fares.add(model.coverage[s.a]);
    model.fares.add(model.coverage[s.b]);
    const int n=model.n;
    const int r0=s.a/n,c0=s.a%n,r1=s.b/n,c1=s.b%n;
    const int h=std::abs(r1-r0)+1,w=std::abs(c1-c0)+1;
    const int sr=r1>=r0?1:-1,sc=c1>=c0?1:-1;
    std::vector<long long> dp(h*w);
    std::vector<int> trace(h*w,-1);
    auto cell=[&](int i,int j){return (r0+i*sr)*n+c0+j*sc;};
    for(int i=0;i<h;++i) for(int j=0;j<w;++j) {
        if(!i && !j) continue;
        const int p=cell(i,j),q=i*w+j;
        const Money upside=model.fares.delta(model.coverage[p]);
        const auto x=uint32_t(p*77411U+shape*31157U);
        const auto noise=((x^(x>>13))*0x9e3779b9U)>>27;
        const long long bonus=(8-shape)*upside+(shape-1)*noise;
        int from=(i>0 && (!j || dp[q-w]>dp[q-1])) ? q-w : q-1;
        dp[q]=dp[from]+bonus;
        trace[q]=from;
    }
    std::vector<int> path;
    for(int q=h*w-1;q>=0;q=trace[q]) path.push_back(cell(q/w,q%w));
    std::reverse(path.begin(),path.end());
    return path;
}
#endif

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
    Money value;
    std::vector<Transfer> output;
#if defined(AHC043_SEED_PORTFOLIO) || defined(AHC043_GEOMETRY_CONTROL)
    const auto limit=Clock::now()+std::chrono::milliseconds(2450);
    auto starts=seeds(model,home,office,fares);
    value=initial;
    output.assign(t,{-1,-1});
    for(int i=0;i<int(starts.size()) && Clock::now()<limit;++i) {
        const auto& x=starts[
#if defined(AHC043_SEED_PORTFOLIO)
            i
#else
            0
#endif
        ];
        Network empty=model;
        auto path=initial_path(empty,x,i);
        Network attempt=model;
        attempt.step(0,x.a);
        attempt.build(path);
        assert(attempt.fares.value()==x.rate);
        while(Clock::now()<limit && attempt.extend()) {}
        while(int(attempt.actions.size())<t) attempt.step();
        if(attempt.cash>value) value=attempt.cash,output=std::move(attempt.actions);
    }
#else
    bool started=model.start(home,office,fares);
#ifndef AHC043_ONE_LINE
    if(started) while(model.extend()) {}
#else
    (void)started;
#endif
    while(int(model.actions.size())<t) model.step();
    value=model.cash;
    output=std::move(model.actions);
#endif
    for(auto [a,c]:output) {
        if(a<0) std::cout<<"-1\n";
        else std::cout<<a<<' '<<c/n<<' '<<c%n<<'\n';
    }
    std::cerr<<"Model = "<<value<<'\n';
}
