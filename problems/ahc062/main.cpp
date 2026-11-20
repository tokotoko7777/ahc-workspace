#include <bits/stdc++.h>
#include "../../library/weighted-sequence-treap.hpp"
#include "ladder.hpp"
using namespace std;
#ifndef AHC062_ROTATE
#define AHC062_ROTATE 0
#endif
#ifndef AHC062_LANES
#define AHC062_LANES 1
#endif
namespace {
using Clock=chrono::steady_clock;
struct Problem{
  int n,v;
  vector<int> w;
  vector<vector<int>> near;
  mt19937 gen{210627};
  void read(){
    cin>>n;v=n*n;w.resize(v);near.resize(v);
    for(auto& x:w)cin>>x;
    for(int y=0;y<n;++y)for(int x=0;x<n;++x)
      for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){
        if(!(dy||dx)||y+dy<0||y+dy>=n||x+dx<0||x+dx>=n)continue;
        near[y*n+x].push_back((y+dy)*n+x+dx);
      }
  }
  bool adjacent(int a,int b)const{
    return max(abs(a/n-b/n),abs(a%n-b%n))==1;
  }
  pair<long long,vector<int>> start()const{
    long long best=-1;vector<int> order;
    for(int flags=0;flags<8;++flags){
      long long moment=0;vector<int> p;
      for(int r=0;r<n;++r)for(int j=0;j<n;++j){
        int x=(r%2)?n-1-j:j,y=r;
        if(flags&1)x=n-1-x;
        if(flags&2)y=n-1-y;
        if(flags&4)swap(x,y);
        int id=y*n+x;
        moment+=1LL*p.size()*w[id];p.push_back(id);
      }
      if(moment>best){best=moment;order=move(p);}
    }
    if(AHC062_LANES&&n>=6&&n%2==0){
      for(int flags=0;flags<16;++flags){
        auto path=two_lane_king_path(n,w,flags);
        long long value=0;
        for(int i=0;i<v;++i)value+=i*1LL*w[path[i]];
        if(value>best){best=value;order=move(path);}
      }
    }
    return {best,move(order)};
  }
  void solve(Clock::time_point deadline){
    auto [initial,p]=start();
    WeightedSequenceTreap route(p,w);
    int attempts=0,accepted=0,endpoint=0;
    constexpr int max_trials=280000;
    for(;attempts<max_trials;++attempts){
      if((attempts&511)==0&&Clock::now()>=deadline)break;
      int l=-1,r=-1;
      const auto seed=gen();
      if(AHC062_ROTATE&&seed%8==0){
        // One-end reversal. Connecting the old endpoint to an interior
        // cell allows flipping the tail without cutting a second old edge.
        const bool prefix=seed>>5&1;
        const int end=route.at(prefix?0:v-1);
        const int u=near[end][gen()%near[end].size()];
        const int at=route.rank(u);
        l=prefix?0:at+1;r=prefix?at:v;
      }else{
        const int a=gen()%v,b=near[a][gen()%near[a].size()];
        int i=route.rank(a),j=route.rank(b);
        if(i>j)swap(i,j);
        if(j==v-1||j-i<2||!adjacent(route.at(i+1),route.at(j+1)))continue;
        l=i+1;r=j+1;
      }
      if(l<r && route.try_reverse(l,r,[](long long d){return d>0;})){
        ++accepted;
        endpoint+=(l==0||r==v);
      }
    }
    p=route.order();
    for(int i=1;i<v;++i)assert(adjacent(p[i-1],p[i]));
    cerr<<"init="<<initial<<" raw="<<route.indexed_sum()<<" trials="<<attempts
        <<" accepted="<<accepted<<" endpoint="<<endpoint<<'\n';
    for(int id:p)cout<<id/n<<' '<<id%n<<'\n';
  }
};
}
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(2350);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();p.solve(deadline);
}
