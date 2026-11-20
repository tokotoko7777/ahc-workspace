#include <bits/stdc++.h>
#include "../../library/load-aware-route.hpp"
#include "../../library/segment-tree.hpp"
using namespace std;
#ifndef AHC034_ANNEAL
#define AHC034_ANNEAL 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=20,V=N*N;
using Route=vector<int>;
int distance_manhattan(int a,int b){return abs(a/N-b/N)+abs(a%N-b%N);}
struct Problem {
  array<int,V> height{};
  array<array<uint8_t,V>,V> dist{};
  void read(){
    int n;cin>>n;assert(n==N);
    for(int& x:height)cin>>x;
    for(int a=0;a<V;++a)for(int b=0;b<V;++b)dist[a][b]=distance_manhattan(a,b);
  }
  auto join()const{
    return [this](const LoadAwareRoute& a,const LoadAwareRoute& b){
      return join_load_routes(a,b,[this](int x,int y){return dist[x][y];});
    };
  }
  LoadAwareRoute summarize(const Route& r)const{
    LoadAwareRoute path;
    auto append=join();
    for(int v:r)path=append(path,LoadAwareRoute::at(v,height[v]));
    return path;
  }
  long long cost(const LoadAwareRoute& path)const{
    return path.empty()?0:100LL*dist[0][path.first]+path.travel_cost(100);
  }
  Route initial()const{
    Route best;
    long long best_cost=LLONG_MAX;
    for(int p=0;p<8;++p){
      Route plan;
      for(int y=0;y<N;++y)for(int c=0;c<N;++c){
        int x=y%2?N-1-c:c;
        int r=p&1?N-1-y:y,col=p&2?N-1-x:x;
        if(p&4)swap(r,col);
        int v=r*N+col;
        if(height[v])plan.push_back(v);
      }
      if(plan.empty())return plan;
      int sum=0,low=0,idx=0;
      for(int i=0;i<int(plan.size());++i){
        sum+=height[plan[i]];
        if(sum<low){low=sum;idx=i+1;}
      }
      assert(sum==0);
      rotate(plan.begin(),plan.begin()+idx,plan.end());
      auto info=summarize(plan);
      assert(info.feasible()&&info.net==0);
      if(long long c=cost(info);c<best_cost){
        best_cost=c;best=std::move(plan);
      }
    }
    return best;
  }
  Route search(Route base,Clock::time_point deadline)const{
    const int n=int(base.size());
    if(n<2)return base;
    auto join_segments=join();
    auto tree=make_segment_tree(n,LoadAwareRoute{},join_segments);
    auto build=[&](const Route& order){
      for(int i=0;i<n;++i)tree.data[tree.leaf_count+i]=LoadAwareRoute::at(order[i],height[order[i]]);
      for(int i=tree.leaf_count-1;i>0;--i)tree.data[i]=join_segments(tree.data[2*i],tree.data[2*i+1]);
    };
    long long best_cost=cost(summarize(base));
    Route best=base;
    mt19937 rng(340034);
    for(int epoch=0;epoch<2;++epoch){
      Route cur=best;
      build(cur);
      long long current=cost(tree.all());
      for(int step=0;step<100000;++step){
        if((step&127)==0&&Clock::now()>=deadline)return best;
        int l=rng()%n,r=rng()%n;
        if(rng()%4<2)r=clamp(l+int(rng()%23)-11,0,n-1);
        if(r==l)continue;
        if(l>r)swap(l,r);
        int mode=rng()%4;
        LoadAwareRoute fresh=tree.query(0,l);
        auto append=[&](int a,int b,bool rev=false){
          auto q=tree.query(a,b);
          fresh=join_segments(fresh,rev?q.reversed():q);
        };
        if(mode==0)append(l,r+1,true);
        else if(mode==1){append(l+1,r+1);append(l,l+1);}
        else if(mode==2){append(r,r+1);append(l,r);}
        else{append(r,r+1);append(l+1,r);append(l,l+1);}
        append(r+1,n);
        if(!fresh.feasible())continue;
        long long next=cost(fresh);
        double ratio=1-step/100000.;
        double temp=(epoch==0?2000.:800.)*ratio*ratio+10.;
        if(next>current&&double(rng())/4294967296.0>=exp((current-next)/temp))continue;
        if(mode==0)reverse(cur.begin()+l,cur.begin()+r+1);
        else if(mode==1)rotate(cur.begin()+l,cur.begin()+l+1,cur.begin()+r+1);
        else if(mode==2)rotate(cur.begin()+l,cur.begin()+r,cur.begin()+r+1);
        else swap(cur[l],cur[r]);
        current=next;
        build(cur);
#ifdef AHC034_VERIFY
        assert(cost(tree.all())==next && cost(summarize(cur))==next);
#endif
        if(next<best_cost){best_cost=next;best=cur;}
      }
    }
    return best;
  }
  void print(const Route& order)const{
    int pos=0,load=0;
    for(int target:order){
      while(pos/N<target/N){cout<<"D\n";pos+=N;}
      while(pos/N>target/N){cout<<"U\n";pos-=N;}
      while(pos%N<target%N){cout<<"R\n";++pos;}
      while(pos%N>target%N){cout<<"L\n";--pos;}
      int change=height[target];
      if(change>0)cout<<'+'<<change<<'\n';
      else cout<<change<<'\n';
      load+=change;assert(load>=0);
    }
    assert(load==0);
  }
};
}
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1550);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  auto base=p.initial();
  auto answer=AHC034_ANNEAL?p.search(base,deadline):base;
  cerr<<"start="<<p.cost(p.summarize(base))<<" move="<<p.cost(p.summarize(answer))<<'\n';
  p.print(answer);
}
