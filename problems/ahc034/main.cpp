#include <bits/stdc++.h>
#include "../../library/load-aware-route.hpp"
#include "../../library/min-cost-flow.hpp"
#include "../../library/segment-tree.hpp"
using namespace std;
#ifndef AHC034_SHORTCUTS
#define AHC034_SHORTCUTS 0
#endif
#ifndef AHC034_FLOW
#define AHC034_FLOW 1
#endif
#ifndef AHC034_EPOCHS
#define AHC034_EPOCHS 2
#endif
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
  Route search(Route base,Clock::time_point deadline,int epochs=AHC034_EPOCHS,uint32_t seed=340034)const{
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
    mt19937 rng(seed);
    for(int epoch=0;epoch<epochs;++epoch){
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
  struct Walk {vector<int> point,change;};
  Walk expand(const Route& order)const{
    Walk w{{0},{0}};
    int pos=0;
    auto step=[&](int inc){pos+=inc;w.point.push_back(pos);w.change.push_back(0);};
    for(int target:order){
      while(pos/N<target/N)step(N);
      while(pos/N>target/N)step(-N);
      while(pos%N<target%N)step(1);
      while(pos%N>target%N)step(-1);
      w.change.back()+=height[target];
    }
    return w;
  }
  long long travel(const Walk& w)const{
    int load=0;long long cost=0;
    assert(w.point.size()==w.change.size());
    for(int i=0;i<int(w.point.size());++i){
      load+=w.change[i];assert(load>=0);
      if(i+1<int(w.point.size())){
        assert(dist[w.point[i]][w.point[i+1]]==1);
        cost+=100+load;
      }
    }
    assert(load==0);
    return cost;
  }
  bool split_fixed_walk(Walk& w,Clock::time_point deadline,long long target=-1)const{
    if(Clock::now()>=deadline)return false;
    const int n=int(w.point.size()),source=n+V,sink=source+1;
    int total=0;
    MinCostFlow<int,long long> flow(sink+1);
    for(int v=0;v<V;++v)if(height[v]>0){
      total+=height[v];flow.add_edge(source,n+v,height[v],0);
    }else if(height[v]<0)flow.add_edge(n+v,sink,-height[v],0);
    vector<int> transfer(n,-1);
    for(int t=0;t<n;++t){
      if(t+1<n)flow.add_edge(t,t+1,total,1);
      const int v=w.point[t],h=height[v];
      if(h>0)transfer[t]=flow.add_edge(n+v,t,h,0);
      else if(h<0)transfer[t]=flow.add_edge(t,n+v,-h,0);
    }
    auto [sent,work]=flow.flow_until(source,sink,total,[&]{return Clock::now()>=deadline;});
    if(sent!=total)return false;
    Walk candidate{w.point,vector<int>(n)};
    array<int,V> served{};
    for(int t=0;t<n;++t)if(transfer[t]>=0){
      const int v=w.point[t];
      candidate.change[t]=flow.get_edge(transfer[t]).flow*(height[v]>0?1:-1);
      served[v]+=candidate.change[t];
    }
    assert(served==height);
    const long long computed=travel(candidate);
    assert(computed==100LL*(n-1)+work);
    if(computed>=(target<0?travel(w):target))return false;
    w=std::move(candidate);
    return true;
  }
#if AHC034_SHORTCUTS
  Route shorten(Route route,int span,bool first_col,Clock::time_point deadline,
                int& attempts,int& applied)const{
    array<int,V> visits{},delta{};
    for(int v:route)++visits[v];
    for(int l=0;l+2<int(route.size()) && Clock::now()<deadline;){
      int best_save=0,right=-1;
      Route best_insert;
      for(int r=min(int(route.size())-1,l+span);r>=l+2;--r){
        const int a=route[l],b=route[r];
        int save=r-l-dist[a][b];
        if(save<=best_save)continue;
        for(bool col:{first_col,!first_col}){
          Route insert;
          int u=a;
          auto move_to=[&](bool horizontal){
            const int d=horizontal?1:N;
            auto coord=[&](int v){return horizontal?v%N:v/N;};
            while(coord(u)!=coord(b)){
              u+=coord(u)<coord(b)?d:-d;insert.push_back(u);
            }
          };
          move_to(col);move_to(!col);
          assert(u==b && int(insert.size())==dist[a][b]);
          ++attempts;
          for(int k=l+1;k<=r;++k)--delta[route[k]];
          for(int v:insert)++delta[v];
          bool okay=true;
          for(int k=l+1;k<=r;++k){
            const int v=route[k];
            if(height[v] && visits[v]+delta[v]==0)okay=false;
          }
          for(int v:insert)--delta[v];
          for(int k=l+1;k<=r;++k)++delta[route[k]];
          if(okay){best_save=save;right=r;best_insert=move(insert);break;}
        }
      }
      if(right>=0){
        for(int k=l+1;k<=right;++k)--visits[route[k]];
        for(int v:best_insert)++visits[v];
        route.erase(route.begin()+l+1,route.begin()+right+1);
        route.insert(route.begin()+l+1,best_insert.begin(),best_insert.end());
        ++applied;
        // Return a few steps to catch a new local seam without rescanning everything.
        l=max(0,l-2);
      }else ++l;
    }
    assert(route.front()==0);
    for(int v=0;v<V;++v)assert(!height[v] || visits[v]>0);
    for(int i=1;i<int(route.size());++i)assert(dist[route[i-1]][route[i]]==1);
    return route;
  }
#endif
  void print(const Walk& w)const{
    for(int i=0;i<int(w.point.size());++i){
      if(i){
        int d=w.point[i]-w.point[i-1];
        cout<<(d==N?'D':d==-N?'U':d==1?'R':'L')<<'\n';
      }
      const int change=w.change[i];
      if(change>0)cout<<'+'<<change<<'\n';
      else if(change<0)cout<<change<<'\n';
    }
  }
};
}
int main(){
  const auto begin=Clock::now();
  const auto deadline=begin+chrono::milliseconds(1550);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  auto base=p.initial();
  auto answer=AHC034_ANNEAL?p.search(base,deadline):base;
  auto walk=p.expand(answer);
  const long long fixed=p.travel(walk);
  if(AHC034_FLOW)p.split_fixed_walk(walk,begin+chrono::milliseconds(1730));
#if AHC034_SHORTCUTS
  const auto inherited=walk;
  const long long floor_inherit=p.travel(inherited);
  const auto end=begin+chrono::milliseconds(1850);
  int attempts=0,edits=0,trials=0,better=0,min_steps=int(inherited.point.size())-1;
  for(int i=0;i<2&&Clock::now()<end;++i){
    Problem::Walk next;
    if(AHC034_SHORTCUTS==1){
      next=p.expand(p.search(answer,begin+chrono::milliseconds(1750),1,340241+997*i));
    }else{
      next.point=p.shorten(inherited.point,i?56:24,i!=0,end,attempts,edits);
      next.change.assign(next.point.size(),0);
    }
    min_steps=min(min_steps,int(next.point.size())-1);
    ++trials;
    if(p.split_fixed_walk(next,end,p.travel(walk))){
      walk=move(next);++better;
    }else if(AHC034_SHORTCUTS==1 && p.travel(next)<p.travel(walk)){
      // If the flow is no better within the clock, the legal literal cargo
      // allocation on the control walk is also eligible.
      walk=move(next);++better;
    }
  }
#endif
  cerr<<"start="<<p.cost(p.summarize(base))<<" move="<<p.travel(walk)
      <<" fixed="<<fixed<<" walk="<<walk.point.size()-1;
#if AHC034_SHORTCUTS
  cerr<<" inherited="<<floor_inherit<<" original_steps="<<inherited.point.size()-1
      <<" min_steps="<<min_steps<<" trials="<<trials<<" attempts="<<attempts
      <<" edits="<<edits<<" better="<<better;
#endif
  cerr<<'\n';
  p.print(walk);
}
