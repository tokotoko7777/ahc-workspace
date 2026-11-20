#include <bits/stdc++.h>
#include "../../library/graph-bfs.hpp"
using namespace std;
#ifndef AHC056_ALLOCATE
#define AHC056_ALLOCATE 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr char letters[]="UDLR";
struct Rule{int c,q,a,s,d;};
struct Program{
  int C=1,Q=1;
  vector<int> initial;
  vector<Rule> rules;
  int cost()const{return C+Q;}
};
struct Problem{
  int n,k,t;
  vector<vector<int>> adj;
  vector<int> stops,positions,dirs;
  void read(){
    cin>>n>>k>>t;
    adj.resize(n*n);string s;
    auto edge=[&](int a,int b){adj[a].push_back(b);adj[b].push_back(a);};
    for(int y=0;y<n;++y){
      cin>>s;
      for(int x=0;x+1<n;++x)if(s[x]=='0')edge(y*n+x,y*n+x+1);
    }
    for(int y=0;y+1<n;++y){
      cin>>s;
      for(int x=0;x<n;++x)if(s[x]=='0')edge(y*n+x,(y+1)*n+x);
    }
    for(int i=0;i<k;++i){int y,x;cin>>y>>x;stops.push_back(y*n+x);}
    for(int i=1;i<k;++i){
      auto path=graph_bfs(adj,stops[i-1]).path_to(stops[i]);
      assert(path.size()>=2);
      for(int j=0;j+1<int(path.size());++j){
        positions.push_back(path[j]);
        int d=path[j+1]-path[j];
        dirs.push_back(d==-n?0:d==n?1:d==-1?2:3);
      }
    }
    assert(int(positions.size())<=t);
  }
  Program compile(int Q,int mode)const{
    Program out{1,Q,vector<int>(n*n),{}};
    vector<int> load(Q);
    unordered_map<uint64_t,pair<int,int>> intern;
    intern.reserve(positions.size()*2+1);
    const int len=positions.size();
    int next_q=0;
    auto phase=[&](int i){return mode==0?i*Q/len:i%Q;};
    for(int i=len-1;i>=0;--i){
      const int v=positions[i],dir=dirs[i],next_c=out.initial[v];
      const uint64_t triple=(uint64_t(next_c)*Q+next_q)*4+dir;
      int q=mode<2?phase(i):0;
      const uint64_t key=mode<2?triple*Q+q:triple;
      if(auto it=intern.find(key);it!=intern.end()){
        out.initial[v]=it->second.second;
        next_q=it->second.first;
        continue;
      }
      if(mode>=2){
        q=int(min_element(load.begin(),load.end())-load.begin());
        if(mode==3&&load[next_q]<=load[q]+1)q=next_q;
      }
      const int c=load[q]++;
      intern.emplace(key,pair{q,c});
      out.rules.push_back({c,q,next_c,next_q,dir});
      out.initial[v]=c;
      next_q=q;
    }
    out.C=max(1,*max_element(load.begin(),load.end()));
    auto map=[&](int q){return q==next_q?0:q==0?next_q:q;};
    for(auto& r:out.rules){r.q=map(r.q);r.s=map(r.s);}
    return out;
  }
  void print(const Program& best)const{
    cout<<best.C<<' '<<best.Q<<' '<<best.rules.size()<<'\n';
    for(int y=0;y<n;++y)for(int x=0;x<n;++x)
      cout<<best.initial[y*n+x]<<(x+1==n?'\n':' ');
    for(auto a:best.rules)cout<<a.c<<' '<<a.q<<' '<<a.a<<' '<<a.s<<' '<<letters[a.d]<<'\n';
  }
};
}
int main(){
  auto deadline=Clock::now()+chrono::milliseconds(1350);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  const int len=p.positions.size(),qmax=min(len,2+int(2*sqrt(double(len))));
  auto best=p.compile(1,0);
  auto try_one=[&](int q,int mode){
    auto arm=p.compile(q,mode);
    if(arm.cost()<best.cost())best=move(arm);
  };
  for(int q=2;q<=qmax&&q+1<best.cost()&&Clock::now()<deadline;++q)
    for(int mode=0;mode<2;++mode)try_one(q,mode);
  const int baseline=best.cost();
  if(AHC056_ALLOCATE)for(int q=2;q<=qmax&&q+1<best.cost()&&Clock::now()<deadline;++q)
    for(int mode=2;mode<4;++mode)try_one(q,mode);
  cerr<<"base="<<baseline<<" cost="<<best.cost()<<" steps="<<len<<'\n';
  p.print(best);
}
