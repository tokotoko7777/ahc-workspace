#include <bits/stdc++.h>
using namespace std;
#ifndef AHC035_TAIL
#define AHC035_TAIL 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=6,M=15,T=10,P=60,C=N*N;
using Order=array<int,P>;
using Population=array<array<int,M>,P>;
struct Solver {
  vector<pair<int,int>> edge;
  array<vector<int>,P> incident;
  array<int,C> degree{},cells{};
  Solver(){
    auto add=[&](int a,int b){
      int id=int(edge.size());edge.push_back({a,b});
      incident[a].push_back(id);incident[b].push_back(id);++degree[a];++degree[b];
    };
    for(int y=0;y<N;++y)for(int x=0;x<N;++x){
      int i=y*N+x;
      if(y+1<N)add(i,i+N);
      if(x+1<N)add(i,i+1);
    }
    iota(cells.begin(),cells.end(),0);
    stable_sort(cells.begin(),cells.end(),[&](int a,int b){return degree[a]>degree[b];});
  }
  Order baseline(const Population& values)const{
    Order rank,order;array<int,P> sum{};
    iota(rank.begin(),rank.end(),0);
    for(int i=0;i<P;++i)sum[i]=accumulate(values[i].begin(),values[i].end(),0);
    stable_sort(rank.begin(),rank.end(),[&](int a,int b){return sum[a]>sum[b];});
    for(int i=0;i<C;++i)order[cells[i]]=rank[i];
    for(int i=C;i<P;++i)order[i]=rank[i];
    return order;
  }
  struct Pair{
    double tail=0;
    int sum=0;
    uint16_t single=0,both=0;
  };
  struct Stat{
    double tail=0;int sum=0;
    array<int,M> single{},both{};
    void adjust(const Pair& p,int sign){
      tail+=sign*p.tail;sum+=sign*p.sum;
      for(unsigned mask=p.single;mask;mask&=mask-1)single[countr_zero(mask)]+=sign;
      for(unsigned mask=p.both;mask;mask&=mask-1)both[countr_zero(mask)]+=sign;
    }
  };
  Order optimize(const Population& values,Order order,int turn,Clock::time_point deadline)const{
    array<int,M> maxTrait{},second{};
    for(auto& v:values)for(int f=0;f<M;++f){
      int x=v[f];
      if(x>maxTrait[f]){second[f]=maxTrait[f];maxTrait[f]=x;}
      else if(x<maxTrait[f])second[f]=max(second[f],x);
    }
    array<double,M> loss_weight{};
    for(int f=0;f<M;++f)loss_weight[f]=(turn<=7?1.:0.)*(3.+0.55*(maxTrait[f]-second[f]));
    array<double,101> exp_value{};
    const double scale=turn<=6?43.:28.;
    for(int i=0;i<=100;++i)exp_value[i]=exp(i/scale);
    array<array<Pair,P>,P> pair;
    double max_tail=0.;
    for(int a=0;a<P;++a)for(int b=0;b<P;++b){
      Pair p{1.,0,0,0};
      for(int f=0;f<M;++f){
        int x=values[a][f],y=values[b][f];p.sum+=x+y;
        p.tail*=0.5*(exp_value[x]+exp_value[y]);
        bool ax=x==maxTrait[f],by=y==maxTrait[f];
        if(ax&&by)p.both|=1u<<f;
        if(ax!=by)p.single|=1u<<f;
      }
      max_tail=max(max_tail,p.tail);
      pair[a][b]=p;
    }
    for(auto& row:pair)for(auto& q:row)q.tail/=max_tail;
    auto full=[&](const Order& o){
      Stat stat;
      for(auto [a,b]:edge)stat.adjust(pair[o[a]][o[b]],1);
      return stat;
    };
    auto score=[&](const Stat& stat){
      double v=(turn<=6?0.32:0.12)*stat.sum/(2.*edge.size())+scale*log(stat.tail);
      for(int f=0;f<M;++f)if(stat.both[f]==0)
        v-=loss_weight[f]*ldexp(1.,-stat.single[f]);
      return v;
    };
    mt19937 rng(353530+turn);
    Stat stats=full(order);
    double cur=score(stats),best_score=cur;
    Order best=order;
    for(int step=0;step<16000;++step){
      if((step&255)==0&&Clock::now()>=deadline)break;
      int a=rng()%C,b=rng()%P;
      if(a==b)continue;
      array<int,8> changed;int len=0;
      for(int id:incident[a])changed[len++]=id;
      for(int id:incident[b])if(find(changed.begin(),changed.begin()+len,id)==changed.begin()+len)
        changed[len++]=id;
      auto next=stats;
      for(int k=0;k<len;++k){
        const auto [x,y]=edge[changed[k]];
        next.adjust(pair[order[x]][order[y]],-1);
      }
      swap(order[a],order[b]);
      for(int k=0;k<len;++k){
        const auto [x,y]=edge[changed[k]];
        next.adjust(pair[order[x]][order[y]],1);
      }
#ifdef AHC035_VERIFY
      if(step%31==0){
        auto check=full(order);
        assert(check.sum==next.sum&&check.single==next.single&&check.both==next.both);
        assert(abs(check.tail-next.tail)<1e-8*check.tail);
      }
#endif
      const double gain=score(next)-cur;
      double f=1-step/16000.;
      const double temp=(turn<=6?3.:1.7)*f*f+.04;
      if(gain>=0||double(rng())/4294967296.<exp(gain/temp)){
        stats=next;cur+=gain;
        // Recompute periodically to remove floating summation drift.
        if((step&255)==0){stats=full(order);cur=score(stats);}
        if(cur>best_score){best_score=cur;best=order;}
      }else swap(order[a],order[b]);
    }
    return best;
  }
};
}
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1550);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  int n,m,t;if(!(cin>>n>>m>>t))return 0;
  assert(n==N&&m==M&&t==T);
  Solver solve;Population values;
  for(int turn=0;turn<=T;++turn){
    for(auto& row:values)for(int& v:row)if(!(cin>>v))return 0;
    if(turn==T)break;
    auto order=solve.baseline(values);
    if(AHC035_TAIL&&Clock::now()<deadline)order=solve.optimize(values,order,turn,deadline);
    for(int y=0;y<N;++y)for(int x=0;x<N;++x)
      cout<<order[y*N+x]<<(x+1==N?'\n':' ');
    cout.flush();
  }
}
