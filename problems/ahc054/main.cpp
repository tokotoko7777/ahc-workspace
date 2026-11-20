#include <bits/stdc++.h>
#include "../../library/masked-bfs-workspace.hpp"
using namespace std;
#ifndef AHC054_PLAN
#define AHC054_PLAN 2
#endif
#ifndef AHC054_SPURS
#define AHC054_SPURS 2
#endif
namespace {
using Clock=chrono::steady_clock;
struct Problem{
  int n,start,goal;
  vector<char> empty;
  vector<vector<int>> adj;
  mt19937 rng{54107};
  void read(){
    int ti,tj;cin>>n>>ti>>tj;start=n/2;goal=n*ti+tj;
    empty.resize(n*n);adj.resize(n*n);
    constexpr int dy[]={-1,1,0,0},dx[]={0,0,-1,1};
    for(int y=0;y<n;++y){
      string s;cin>>s;
      for(int x=0;x<n;++x){
        int p=y*n+x;empty[p]=s[x]=='.';
        for(int i=0;i<4;++i){
          int yy=y+dy[i],xx=x+dx[i];
          if(0<=yy&&yy<n&&0<=xx&&xx<n)adj[p].push_back(yy*n+xx);
        }
      }
    }
  }
  vector<int> route(int pos,const MaskedBfsWorkspace& bfs) const {
    if(bfs.distance(pos)<0)return {};
    vector<int> p{pos};
    while(p.back()!=goal)p.push_back(bfs.parent(p.back()));
    return p;
  }
  vector<int> trail(Clock::time_point cutoff){
    MaskedBfsWorkspace bfs(n*n);
    bfs.run(adj,goal,[&](int v){return empty[v];});
    auto best=route(start,bfs);assert(!best.empty());
    if(AHC054_PLAN<2)return best;
    for(int epoch=0;epoch<(AHC054_SPURS==1?60:30)&&Clock::now()<cutoff;++epoch){
      auto permitted=empty;
      vector<int> current{start};
      int tail=start, heading=-1;
      const bool reverse=epoch%3==1;
      const int target=reverse?start:goal;
      if(reverse)current[0]=tail=goal;
      while(tail!=target&&Clock::now()<cutoff){
        vector<int> options;
        for(int v:adj[tail])if(permitted[v])options.push_back(v);
        if(find(options.begin(),options.end(),target)!=options.end()){
          current.push_back(target);tail=target;break;
        }
        permitted[tail]=false;
        for(int v:options)permitted[v]=false;
        bfs.run(adj,target,[&](int v){return permitted[v];});
        int pick=-1, best_q=INT_MIN;
        for(int v:options){
          int shortest=INT_MAX,degree=0;
          for(int u:adj[v])if(int d=bfs.distance(u);d>=0){
            shortest=min(shortest,1+d);++degree;
          }
          if(!degree)continue;
          const int straight=(v-tail)==heading;
          const int score = (epoch%4==0?11:6)*shortest - 18*degree
              +(epoch%4<2?40:-5)*straight+int(rng()%78);
          if(score>best_q){best_q=score;pick=v;}
        }
        if(pick<0)break;
        heading=pick-tail;tail=pick;permitted[tail]=true;
        current.push_back(tail);
      }
      if(tail==target&&current.size()>best.size()){
        if(reverse)ranges::reverse(current);
        best=move(current);
      }
    }
    return best;
  }
  void spurs(const vector<int>& spine,vector<char>& saved)const{
    // Protect a single spine attachment, then grow an induced blind side
    // path which never touches any other cell of the growing forest.
    vector<int> order=spine;
    for(int v:order){
      if(v==goal)continue;
      vector<int> best;
      for(int u:adj[v])if(empty[u]&&!saved[u]){
        int count=0;for(int w:adj[u])count+=saved[w];
        if(count!=1)continue;
        vector<char> occupied=saved;
        vector<int> chain{u};occupied[u]=true;
        int prev=v;
        bool bent=false;
        while(chain.size()<15){
          const int tail=chain.back();
          int next=-1,utility=-1;
          for(int q:adj[tail])if(empty[q]&&!occupied[q]){
            int degree=0,exits=0;
            for(int z:adj[q]){
              degree+=occupied[z];
              exits+=empty[z]&&!occupied[z];
            }
            if(degree!=1)continue;
            // Prefer a bend to make cells beyond discovery rays, then room.
            int val=10*exits+17*((q-tail)!=(tail-prev)&&!bent);
            if(val>utility){utility=val;next=q;}
          }
          if(next<0)break;
          bent|=(next-tail)!=(tail-prev);
          prev=tail;occupied[next]=true;chain.push_back(next);
        }
        if(bent&&chain.size()>=4&&chain.size()>best.size())best=move(chain);
      }
      for(int v:best)saved[v]=true;
    }
  }

};
}
int main(){
  const auto cutoff=Clock::now()+chrono::milliseconds(1400);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  vector<char> protected_cell(p.n*p.n);
  int length=0;
  if(AHC054_PLAN){
    auto trail=p.trail(cutoff);length=int(trail.size())-1;
    for(int v:trail)protected_cell[v]=true;
    for(int i=0;i<int(trail.size());++i){
      int deg=0;for(int u:p.adj[trail[i]])deg+=protected_cell[u];
      assert(deg==1+(i>0&&i+1<int(trail.size())));
    }
    if(AHC054_SPURS==2)p.spurs(trail,protected_cell);
  }
  cerr<<"path="<<length<<'\n';
  vector<char> revealed(p.n*p.n);
  for(int turn=0;;++turn){
    int y,x,k;if(!(cin>>y>>x>>k))break;
    for(int i=0;i<k;++i){int a,b;cin>>a>>b;revealed[a*p.n+b]=true;}
    if(y*p.n+x==p.goal)break;
    if(turn||!AHC054_PLAN){cout<<"0\n"<<flush;continue;}
    revealed[p.start]=true;
    vector<int> trees;
    for(int v=0;v<p.n*p.n;++v)if(p.empty[v]&&!revealed[v]&&!protected_cell[v])trees.push_back(v);
    cout<<trees.size();
    for(int v:trees)cout<<' '<<v/p.n<<' '<<v%p.n;
    cout<<'\n'<<flush;
  }
}
