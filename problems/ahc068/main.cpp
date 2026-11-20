#include <bits/stdc++.h>
#include "../../library/half-swap-interval.hpp"
using namespace std;
#ifndef AHC068_STRIPS
#define AHC068_STRIPS 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=20,V=N*N;
using Board=array<int,V>;
struct Op{char d;int r,c,h,w;};
struct Problem{
  Board initial;
  array<string,N> east;
  array<string,N-1> south;
  array<vector<int>,V> adj;
  vector<int> order;
  array<array<uint8_t,V>,V> md{};
  void read(){
    int n;cin>>n;assert(n==N);
    for(int& x:initial)cin>>x;
    for(auto& s:east)cin>>s;
    for(auto& s:south)cin>>s;
    auto edge=[&](int a,int b){adj[a].push_back(b);adj[b].push_back(a);};
    for(int y=0;y<N;++y)for(int x=0;x<N;++x){
      if(x+1<N&&east[y][x]=='0')edge(N*y+x,N*y+x+1);
      if(y+1<N&&south[y][x]=='0')edge(N*y+x,N*(y+1)+x);
    }
    for(int p=0;p<V;++p)for(int q=0;q<V;++q)
      md[p][q]=abs(p/N-q/N)+abs(p%N-q%N);
    array<bool,V> seen{};
    const int root=(N/2)*N+N/2;
    seen[root]=true;order.push_back(root);
    for(int i=0;i<int(order.size());++i)for(int u:adj[order[i]])if(!seen[u]){
      seen[u]=true;order.push_back(u);
    }
    assert(order.size()==V);ranges::reverse(order);
  }
  vector<Op> solve(bool strip,Clock::time_point deadline)const{
    Board at=initial,pos{};
    for(int p=0;p<V;++p)pos[at[p]]=p;
    array<bool,V> active;active.fill(true);
    vector<Op> out;
    for(int target:order){
      if(pos[target]==target){active[target]=false;continue;}
      if(strip&&Clock::now()>=deadline)return {};
      array<array<int,2>,V> lo,hi;
      if(strip){
        for(int p=0;p<V;++p){
          int y=p/N,x=p%N;
          lo[p][0]=x;lo[p][1]=y;
          if(active[p]&&x&&active[p-1]&&east[y][x-1]=='0')lo[p][0]=lo[p-1][0];
          if(active[p]&&y&&active[p-N]&&south[y-1][x]=='0')lo[p][1]=lo[p-N][1];
        }
        for(int p=V-1;p>=0;--p){
          int y=p/N,x=p%N;
          hi[p][0]=x+1;hi[p][1]=y+1;
          if(active[p]&&x+1<N&&active[p+1]&&east[y][x]=='0')hi[p][0]=hi[p+1][0];
          if(active[p]&&y+1<N&&active[p+N]&&south[y][x]=='0')hi[p][1]=hi[p+N][1];
        }
      }
      const int source=pos[target];
      array<int,V> parent,queue;
      parent.fill(-1);parent[target]=target;queue[0]=target;
      int count=1;
      for(int head=0;head<count&&parent[source]<0;++head){
        const int p=queue[head];
        auto add=[&](int u){if(parent[u]<0&&active[u]){parent[u]=p;queue[count++]=u;}};
        for(int u:adj[p])add(u);
        if(strip)for(int d=0;d<2;++d){
          const int coord=d?p/N:p%N,step=d?N:1,radius=(hi[p][d]-lo[p][d])/2;
          for(int z=max(lo[p][d],coord-radius);z<min(hi[p][d],coord+radius+1);++z)
            if(z!=coord)add(p+(z-coord)*step);
        }
      }
      assert(parent[source]>=0);
      for(int p=source;p!=target;p=parent[p]){
        const int q=parent[p],d=(p/N!=q/N),step=d?N:1;
        const int x=d?p/N:p%N,y=d?q/N:q%N,diff=abs(x-y);
        const int base=d?p%N:p/N*N;
        const auto limits=half_swap_interval_starts(strip?lo[p][d]:min(x,y),
            strip?hi[p][d]:max(x,y)+1,x,y);
        assert(limits);
        int pick=limits->first,score=INT_MIN;
        for(int l=limits->first;l<=limits->second;++l){
          int g=0;
          for(int i=0;i<diff;++i){
            int a=base+(l+i)*step,b=a+diff*step;
            g+=md[a][at[a]]+md[b][at[b]]-md[a][at[b]]-md[b][at[a]];
          }
          if(g>score){score=g;pick=l;}
        }
        const int top=base+pick*step;
        out.push_back({d?'V':'H',top/N,top%N,d?2*diff:1,d?1:2*diff});
        for(int i=0;i<diff;++i){
          int a=top+i*step,b=a+diff*step;
          assert(active[a]&&active[b]);
          swap(at[a],at[b]);pos[at[a]]=a;pos[at[b]]=b;
        }
        assert(pos[target]==q);
      }
      assert(at[target]==target);
      active[target]=false;
    }
    for(int i=0;i<V;++i)assert(at[i]==i);
    assert(out.size()<=100000);
    return out;
  }
};
}
int main(){
  auto deadline=Clock::now()+chrono::milliseconds(1400);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  auto ans=p.solve(false,deadline);
  const int baseline=ans.size();
  if(AHC068_STRIPS){
    auto q=p.solve(true,deadline);
    if(!q.empty()&&q.size()<ans.size())ans=move(q);
  }
  cerr<<"base="<<baseline<<" turns="<<ans.size()<<'\n';
  for(auto [d,r,c,h,w]:ans)cout<<d<<' '<<r<<' '<<c<<' '<<h<<' '<<w<<'\n';
}
