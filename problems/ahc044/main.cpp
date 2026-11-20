#include <bits/stdc++.h>
using namespace std;
#ifndef AHC044_ANNEAL
#define AHC044_ANNEAL 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=100;
using Counts=array<int,N>;
struct Plan{Counts a{},b{};};
struct Problem{
  int L;
  Counts target;
  void read(){int n;cin>>n>>L;assert(n==N);for(int& x:target)cin>>x;}
  int error(const Plan& p)const{
    Counts count{};
    int at=0;
    for(int t=0;t<L;++t)at=(++count[at]&1)?p.a[at]:p.b[at];
    int e=0;
    for(int i=0;i<N;++i)e+=abs(count[i]-target[i]);
    return e;
  }
  Plan initial()const{
    Counts order; iota(order.begin(),order.end(),0);
    sort(order.begin(),order.end(),[&](int a,int b){return pair{target[a],a}<pair{target[b],b};});
    vector<int> cycle;
    for(int i=0;i<N;i+=2)cycle.push_back(order[i]);
    for(int i=N-1;i>=0;i-=2)cycle.push_back(order[i]);
    Plan p;
    Counts need;
    for(int i=0;i<N;++i)need[i]=2*target[i];
    for(int i=0;i<N;++i){
      int a=cycle[i],b=cycle[(i+1)%N];
      p.a[a]=b;need[b]-=target[a];
    }
    reverse(order.begin(),order.end());
    for(int item:order){
      int j=int(max_element(need.begin(),need.end())-need.begin());
      p.b[item]=j;need[j]-=target[item];
    }
    return p;
  }
  Counts residual(const Plan& p)const{
    Counts r;
    for(int i=0;i<N;++i)r[i]=-2*target[i];
    for(int i=0;i<N;++i){r[p.a[i]]+=target[i];r[p.b[i]]+=target[i];}
    return r;
  }
  Plan refine(Plan initial,int& best_error,Clock::time_point deadline)const{
    mt19937 rng(440044);
    Plan best=initial;
    for(int pass=0;pass<9;++pass){
      if(Clock::now()>=deadline)break;
      Plan p=pass%3==0?initial:best;
      auto r=residual(p);
      const double tilt=pass%3==0?.30:pass%3==1?.7:1.0;
      auto loss=[&](int d,int i){return double(abs(d))+tilt*double(d)*d/(target[i]+1250);};
      double cost=0;
      for(int i=0;i<N;++i)cost+=loss(r[i],i);
      double record=cost;
      Plan proxy_best=p;
      for(int step=0;step<22000;++step){
        if((step&255)==0&&Clock::now()>=deadline)return best;
        int u=rng()%N,v=rng()%N;
        bool sw=(rng()&3)!=0;
        int a=p.b[u],b=sw?p.b[v]:v;
        if(a==b||(sw&&u==v))continue;
        int amount=sw?target[u]-target[v]:target[u];
        double delta=loss(r[a]-amount,a)+loss(r[b]+amount,b)-loss(r[a],a)-loss(r[b],b);
        double f=1-step/22000.,temp=(pass%3==0?3200.:1700.)*f*f+.15;
        if(delta>0&&double(rng())/4294967296.0>=exp(-delta/temp))continue;
        r[a]-=amount;r[b]+=amount;cost+=delta;
        if(sw)swap(p.b[u],p.b[v]);else p.b[u]=v;
#ifdef AHC044_VERIFY
        if((step%67)==0)assert(r==residual(p));
#endif
        if(cost<record){record=cost;proxy_best=p;}
        if((step%1201)==0&&step>8000){
          const int e=error(p);
          if(e<best_error){best_error=e;best=p;}
        }
      }
      for(const auto& q:{p,proxy_best}){
        int e=error(q);
        if(e<best_error){best_error=e;best=q;}
      }
    }
    return best;
  }
};
}
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1550);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  auto q=p.initial();
  const int start=p.error(q);
  int e=start;
  if(AHC044_ANNEAL)q=p.refine(q,e,deadline);
  assert(e==p.error(q));
  cerr<<"start="<<start<<" error="<<e<<'\n';
  for(int i=0;i<N;++i)cout<<q.a[i]<<' '<<q.b[i]<<'\n';
}
