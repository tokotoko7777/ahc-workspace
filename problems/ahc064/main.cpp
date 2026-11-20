#include <bits/stdc++.h>
#include "../../library/noncrossing-matching.hpp"
using namespace std;
#ifndef AHC064_PACK
#define AHC064_PACK 2
#endif
#ifndef AHC064_ORDER
#define AHC064_ORDER 1
#endif
namespace {
constexpr int R=10;
struct Move{int type,i,j,k;};
using Schedule=vector<vector<Move>>;
struct Solver{
  array<vector<int>,R> dep,side;
  vector<Move> work;
  void read(){
    int r;cin>>r;assert(r==R);
    for(auto& x:dep){x.resize(10);for(int& v:x)cin>>v;}
  }
  void add(int type,int i,int j,int k){
    assert(k>0);
    auto& a=dep[i];auto& b=side[j];
    if(type==0){
      assert(int(a.size())>=k&&int(b.size())+k<=20);
      b.insert(b.begin(),a.end()-k,a.end());a.resize(a.size()-k);
    }else{
      assert(int(b.size())>=k&&int(a.size())+k<=15);
      a.insert(a.end(),b.begin(),b.begin()+k);b.erase(b.begin(),b.begin()+k);
    }
    work.push_back({type,i,j,k});
  }
  void build(int policy=0,int trial=0){
    mt19937 rng(649011+trial);
    array<int,R> target{},door_load{},yard_load{};
    for(int i=0;i<R;++i)add(0,i,i,10);
    for(int finished=0;finished<100;){
      int goal=-1,j=-1,block=-1;
      int best=INT_MIN;
      for(int i=0;i<R;++i)if(target[i]<10){
        int value=10*i+target[i];
        for(int z=0;z<R;++z){
          const auto it=ranges::find(side[z],value);
          if(it==side[z].end())continue;
          const int blocked=it-side[z].begin();
          int chain=1;
          while(chain<side[z].end()-it&&target[i]+chain<10&&it[chain]==value+chain)++chain;
          int quality=policy?int(rng()%109):-i;
          if(policy==2)quality+=(-10-(trial%4)*7)*blocked+45*chain
              -((trial/4)%3)*yard_load[z]-((trial/12)%3)*door_load[i];
          if(quality>best){best=quality;goal=i;j=z;block=blocked;}
          break;
        }
      }
      assert(goal>=0&&j>=0);
      int value=10*goal+target[goal];
        vector<pair<int,int>> buffer;
        for(int k=0;k<R&&block>0;++k){
          // Favor dep near this siding to leave shorter crossing spans.
          const int i=(k+j)%R;
          if(i==goal)continue;
          const int take=min(block,15-int(dep[i].size()));
          if(take>0){add(1,i,j,take);buffer.emplace_back(i,take);block-=take;}
        }
        assert(block==0);
        assert(!side[j].empty()&&side[j][0]==value);
        int k=1;
        while(k<int(side[j].size())&&value+k<10*goal+10&&side[j][k]==value+k)++k;
        add(1,goal,j,k);target[goal]+=k;finished+=k;
        yard_load[j]+=1+2*int(buffer.size());++door_load[goal];
        for(auto [i,take]:buffer|views::reverse)add(0,i,j,take);
    }
    for(int i=0;i<R;++i){
      assert(dep[i].size()==10&&side[i].empty());
      for(int j=0;j<10;++j)assert(dep[i][j]==10*i+j);
    }
  }
  Schedule serial()const{
    Schedule ans(1);
    ans[0].assign(work.begin(),work.begin()+R);
    for(int a=R;a<int(work.size());++a)ans.push_back({work[a]});
    return ans;
  }
  Schedule pack(bool critical)const{
    const int n=work.size();
    vector<vector<int>> next(n);
    vector<int> in(n),tail(n,1),ready;
    array<int,R> dep_last,side_last;dep_last.fill(-1);side_last.fill(-1);
    for(int p=0;p<n;++p){
      const auto [type,i,j,k]=work[p];
      int a=dep_last[i],b=side_last[j];
      if(a>=0){next[a].push_back(p);++in[p];}
      if(b>=0&&b!=a){next[b].push_back(p);++in[p];}
      if(!in[p])ready.push_back(p);
      dep_last[i]=side_last[j]=p;
    }
    for(int i=n-1;i>=0;--i)for(int p:next[i])tail[i]=max(tail[i],1+tail[p]);
    Schedule result;
    struct Edge{int left,right,weight;};
    int done=0;
    while(done<n){
      vector<Edge> edges;
      for(int p:ready)edges.push_back({work[p].i,work[p].j,critical?1+tail[p]*tail[p]:1});
      const auto picked=maximum_weight_noncrossing_matching(edges);
      assert(!picked.empty());
      vector<Move> group;
      vector<int> future;
      vector<char> removed(ready.size());
      for(int z:picked){
        removed[z]=true;const int p=ready[z];++done;
        group.push_back(work[p]);
        for(int u:next[p])if(--in[u]==0)future.push_back(u);
      }
      for(int i=0;i<int(ready.size());++i)if(!removed[i])future.push_back(ready[i]);
      ready=move(future);result.push_back(move(group));
    }
    return result;
  }
  Schedule completed()const{
    auto first=pack(false),second=pack(true);
    return second.size()<first.size()?move(second):move(first);
  }
  void portfolio(){
    Solver incumbent=*this;
    incumbent.build();
    auto ans=incumbent.completed();
    const int frozen=ans.size();
    int chosen_ops=incumbent.work.size(), candidates=0;
    for(int trial=0;trial<32;++trial){
      Solver alternate=*this;
      alternate.build(AHC064_ORDER,trial);
      auto next=alternate.completed();
      if(next.size()<ans.size()){
        ans=move(next);
        chosen_ops=alternate.work.size();
      }
      ++candidates;
    }
    cerr<<"frozen="<<frozen<<" ops="<<chosen_ops<<" trials="<<candidates<<" turns="<<ans.size()<<'\n';
    output(ans);
  }
  static void output(const Schedule& ans){
    assert(ans.size()<=4000);
    cout<<ans.size()<<'\n';
    for(const auto& group:ans){
      cout<<group.size()<<'\n';
      for(auto [t,i,j,k]:group)cout<<t<<' '<<i<<' '<<j<<' '<<k<<'\n';
    }
  }
  void print(){
    auto baseline=serial(),ans=baseline;
    int cardinal=0,critical=0;
    if(AHC064_PACK){
      auto q=pack(false);cardinal=q.size();
      if(q.size()<ans.size())ans=move(q);
    }
    if(AHC064_PACK>=2){
      auto q=pack(true);critical=q.size();
      if(q.size()<ans.size())ans=move(q);
    }
    assert(ans.size()<=4000);
    cerr<<"base="<<baseline.size()<<" ops="<<work.size()<<" card="<<cardinal
        <<" critical="<<critical<<" turns="<<ans.size()<<'\n';
    output(ans);
  }
};
}
int main(){
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Solver game;game.read();
  if(AHC064_ORDER)game.portfolio();
  else{game.build();game.print();}
}
