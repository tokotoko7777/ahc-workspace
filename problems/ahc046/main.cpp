#include <bits/stdc++.h>
#include "../../library/static-slide-grid.hpp"
using namespace std;
#ifndef AHC046_CROSS
#define AHC046_CROSS 1
#endif
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
  struct Choice{int changed=-1,look=0;Path path;};
  vector<Choice> choices(vector<unsigned char>& wall,int i,int& checks,Clock::time_point deadline)const{
    const int start=target[i-1],end=target[i],next=i+1<m?target[i+1]:-1;
    StaticSlideGrid grid(n,n,wall);
    auto root=explore(grid,start);
    vector<Choice> options;
    if(root.dist[end]<INF)
      options.push_back({-1,next<0?0:explore(grid,end).dist[next],root.trace(end)});
    vector<int> alter;
    for(int q:{end,next})if(q>=0)
      for(int d=0;d<4;++d)if(int b=adjacent(q,d);b>=0&&!goal[b])alter.push_back(b);
    for(int b=0;b<n*n;++b)if(wall[b])alter.push_back(b);
    sort(alter.begin(),alter.end());alter.erase(unique(alter.begin(),alter.end()),alter.end());
    for(int b:alter){
      if(Clock::now()>=deadline)break;
      ++checks;
      wall[b]^=1;
      StaticSlideGrid g2(n,n,wall);
      int look=next<0?0:explore(g2,end).dist[next];
      int best=INF,where=-1,move=-1;
      Path suf;
      for(int d=0;d<4;++d){
        int p=adjacent(b,d);
        if(p<0||wall[p]||root.dist[p]+1>=best)continue;
        auto finish=explore(g2,p);
        int total=root.dist[p]+1+finish.dist[end];
        if(total<best){best=total;where=p;move=d^1;suf=finish.trace(end);}
      }
      if(where>=0){
        auto seq=root.trace(where);
        seq.emplace_back('A',move);seq.insert(seq.end(),suf.begin(),suf.end());
        options.push_back({b,look,std::move(seq)});
      }
      wall[b]^=1;
    }
    return options;
  }
  Path beam(Path incumbent,Clock::time_point deadline,int& checks)const{
    constexpr int width=8;
    struct State{vector<unsigned char> wall;Path past;int rank;};
    vector<State> pool{{vector<unsigned char>(n*n),{},0}};
    for(int i=1;i<m;++i){
      vector<State> next;
      for(auto& s:pool){
        if(Clock::now()>=deadline)return incumbent;
        for(auto& c:choices(s.wall,i,checks,deadline)){
          int len=int(s.past.size()+c.path.size());
          if(c.look>=INF||len>=int(incumbent.size()))continue;
          State t{s.wall,s.past,3*len+2*c.look};
          if(c.changed>=0)t.wall[c.changed]^=1;
          t.past.insert(t.past.end(),c.path.begin(),c.path.end());
          next.push_back(std::move(t));
        }
      }
      sort(next.begin(),next.end(),[](const State& a,const State& b){
        return pair{a.rank,a.past.size()}<pair{b.rank,b.past.size()};
      });
      pool.clear();
      for(auto& s:next){
        if(int(pool.size())>=width)break;
        if(any_of(pool.begin(),pool.end(),[&](const State& p){return p.wall==s.wall;}))continue;
        pool.push_back(std::move(s));
      }
      if(pool.empty())return incumbent;
    }
    return std::move(pool.front().past);
  }
  Path multi(Path incumbent,Clock::time_point deadline,int& checks)const{
    for(int lambda:{0,1,2,3,4,6,8,11}){
      vector<unsigned char> wall(n*n);
      Path past;
      for(int i=1;i<m;++i){
        if(Clock::now()>=deadline)return incumbent;
        auto options=choices(wall,i,checks,deadline);
        auto rank=[&](const Choice& a){return pair{3*int(a.path.size())+lambda*a.look,int(a.path.size())};};
        auto best=min_element(options.begin(),options.end(),[&](const Choice& a,const Choice& b){
          if((a.look<INF)!=(b.look<INF))return a.look<INF;
          return rank(a)<rank(b);
        });
        if(best==options.end()||best->look>=INF){past.clear();break;}
        if(best->changed>=0)wall[best->changed]^=1;
        past.insert(past.end(),best->path.begin(),best->path.end());
      }
      if(!past.empty()&&past.size()<incumbent.size())incumbent=std::move(past);
    }
    return incumbent;
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
  int incumbent=answer.size(),checks=0;
  if(AHC046_BLOCK&&AHC046_CROSS==1)answer=p.beam(std::move(answer),deadline,checks);
  if(AHC046_BLOCK&&AHC046_CROSS==2)answer=p.multi(std::move(answer),deadline,checks);
  cerr<<"base="<<start<<" turns="<<answer.size()<<" incumbent="<<incumbent<<" checks="<<checks<<'\n';
  for(auto [op,d]:answer)cout<<op<<' '<<"UDLR"[d]<<'\n';
}
