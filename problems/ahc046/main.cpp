#include <bits/stdc++.h>
#include "../../library/static-slide-grid.hpp"
using namespace std;
#ifndef AHC046_BLOCK
#define AHC046_BLOCK 1
#endif
namespace {
using Clock=chrono::steady_clock;
using Action=pair<char,int>;
using Path=vector<Action>;
constexpr int INF=1000000;
struct Problem{
  int n,m;
  vector<int> target;
  vector<char> goal;
  void read(){
    cin>>n>>m;
    target.resize(m);goal.resize(n*n);
    for(int& p:target){int r,c;cin>>r>>c;p=r*n+c;goal[p]=true;}
  }
  int adjacent(int p,int d)const{
    const int r=p/n+(d==0?-1:d==1?1:0),c=p%n+(d==2?-1:d==3?1:0);
    return 0<=r&&r<n&&0<=c&&c<n?r*n+c:-1;
  }
  struct BFS{
    vector<int> dist,prev,direction;
    vector<char> type;
    Path trace(int t)const{
      assert(dist[t]<INF);
      Path p;
      for(int v=t;prev[v]>=0;v=prev[v])p.emplace_back(type[v],direction[v]);
      reverse(p.begin(),p.end());return p;
    }
  };
  BFS explore(const StaticSlideGrid& grid,int start)const{
    const int v=n*n;
    BFS s{vector<int>(v,INF),vector<int>(v,-1),vector<int>(v),vector<char>(v)};
    vector<int> q{start};
    s.dist[start]=0;
    for(int i=0;i<int(q.size());++i){
      int a=q[i];
      auto visit=[&](int b,char op,int dir){
        if(b<0||s.dist[b]<INF)return;
        s.dist[b]=s.dist[a]+1;
        s.prev[b]=a;s.type[b]=op;s.direction[b]=dir;q.push_back(b);
      };
      for(int d=0;d<4;++d){visit(grid.step[a][d],'M',d);visit(grid.stop[a][d],'S',d);}
    }
    return s;
  }
  Path compute(bool use_block,Clock::time_point deadline)const{
    vector<unsigned char> wall(n*n);
    Path answer;
    for(int i=1;i<m;++i){
      int start=target[i-1],end=target[i],next=i+1<m?target[i+1]:-1;
      StaticSlideGrid grid(n,n,wall);
      auto origin=explore(grid,start);
      if(origin.dist[end]>=INF)return {};
      auto incumbent=origin.trace(end);
      int best_cost=origin.dist[end]+(next<0?0:explore(grid,end).dist[next]);
      int chosen=-1;
      if(use_block&&Clock::now()<deadline){
        vector<int> alter;
        for(int q:{end,next})if(q>=0)
          for(int d=0;d<4;++d)if(int p=adjacent(q,d);p>=0&&!goal[p])alter.push_back(p);
        for(int p=0;p<n*n;++p)if(wall[p])alter.push_back(p);
        sort(alter.begin(),alter.end());
        alter.erase(unique(alter.begin(),alter.end()),alter.end());
        for(int b:alter){
          wall[b]^=1;
          StaticSlideGrid g2(n,n,wall);
          int look=next<0?0:explore(g2,end).dist[next];
          for(int k=0;k<4;++k){
            int p=adjacent(b,k);
            if(p<0||wall[p]||origin.dist[p]>=INF||origin.dist[p]+1+look>=best_cost)continue;
            auto bin=explore(g2,p);
            if(bin.dist[end]>=INF)continue;
            int length=origin.dist[p]+1+bin.dist[end]+look;
            if(length>=best_cost)continue;
            best_cost=length;chosen=b;
            incumbent=origin.trace(p);
            incumbent.emplace_back('A',k^1);
            auto tail=bin.trace(end);
            incumbent.insert(incumbent.end(),tail.begin(),tail.end());
          }
          wall[b]^=1;
        }
      }
      if(chosen>=0)wall[chosen]^=1;
      answer.insert(answer.end(),incumbent.begin(),incumbent.end());
    }
    return answer;
  }
};
}
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1450);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  auto answer=p.compute(false,deadline);
  int start=answer.size();
  if(AHC046_BLOCK){
    auto contender=p.compute(true,deadline);
    if(!contender.empty()&&contender.size()<answer.size())answer=std::move(contender);
  }
  cerr<<"base="<<start<<" turns="<<answer.size()<<'\n';
  for(auto [op,d]:answer)cout<<op<<' '<<"UDLR"[d]<<'\n';
}
