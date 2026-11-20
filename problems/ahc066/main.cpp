#include <bits/stdc++.h>
#include "../../library/unit-lexicographic-bfs.hpp"
using namespace std;
#ifndef AHC066_REPLAN
#define AHC066_REPLAN 1
#endif
namespace {
using Clock=chrono::steady_clock;
using Edge=UnitLexicographicBfs::Edge;
constexpr string_view labels="FRLP";
struct Trace{string commands;vector<int> stops;};
struct Robot{
  int n,m,T;
  vector<int> ball,basket;
  vector<vector<Edge>> pose;
  void read(){
    cin>>n>>m>>T;
    vector<string> v(n),h(n-1);
    for(auto& s:v)cin>>s;
    for(auto& s:h)cin>>s;
    pose.resize(4*n*n);
    for(int i=0;i<n*n;++i)for(int d=0;d<4;++d){
      const int y=i/n,x=i%n;
      int p=i;
      if(d==0&&x+1<n&&v[y][x]=='0')p=i+1;
      if(d==1&&y+1<n&&h[y][x]=='0')p=i+n;
      if(d==2&&x>0&&v[y][x-1]=='0')p=i-1;
      if(d==3&&y>0&&h[y-1][x]=='0')p=i-n;
      pose[4*i+d]={{4*p+d,1},{4*i+(d+1)%4,1},{4*i+(d+3)%4,1}};
    }
    for(int i=0;i<m;++i){
      int a,b,c,d;cin>>a>>b>>c>>d;
      ball.push_back(a*n+b);basket.push_back(c*n+d);
    }
  }
  int best_pose(UnitLexicographicBfs& bfs,int goal)const{
    int end=4*goal;
    for(int v=end+1;v<4*goal+4;++v)
      if(pair{bfs.length[v],bfs.secondary[v]}<pair{bfs.length[end],bfs.secondary[end]})end=v;
    assert(bfs.length[end]>=0);
    return end;
  }
  static void append(string& plan,UnitLexicographicBfs& bfs,int end){
    for(int e:bfs.edges_to(end))plan.push_back(labels[e]);
  }
  Trace plain(bool greedy)const{
    UnitLexicographicBfs bfs(pose.size());
    vector<char> done(m);
    Trace result;
    int here=0;
    for(int i=0;i<m;++i){
      bfs.run(pose,here);
      int id=i;
      if(greedy){
        int best=INT_MAX;
        for(int j=0;j<m;++j)if(!done[j]){
          const int u=best_pose(bfs,ball[j]);
          const int spatial=abs(ball[j]/n-basket[j]/n)+abs(ball[j]%n-basket[j]%n);
          const int merit=8*bfs.length[u]+spatial;
          if(merit<best){best=merit;id=j;}
        }
      }
      done[id]=true;
      for(int cell:{ball[id],basket[id]}){
        bfs.run(pose,here);here=best_pose(bfs,cell);
        append(result.commands,bfs,here);
        result.commands+='S';result.stops.push_back(cell);
      }
    }
    return result;
  }
  vector<string> macros(const string& s)const{
    unordered_map<string,int> counts;
    for(int l=0;l<int(s.size());++l){
      for(int r=l;r<min(l+12,int(s.size()))&&s[r]!='S';++r)
        if(r-l+1>=4)++counts[s.substr(l,r-l+1)];
    }
    vector<pair<int,string>> rank;
    for(auto& [pattern,cnt]:counts)if(cnt>=2)
      rank.emplace_back((cnt-1)*(int(pattern.size())-1),pattern);
    ranges::sort(rank,greater{});
    vector<string> result;
    for(int i=0;i<min(10,int(rank.size()));++i)result.push_back(rank[i].second);
    return result;
  }
  string compress(const string& s,const string& macro)const{
    const int begin=s.find(macro),len=macro.size();
    string result=s.substr(0,begin)+"M"+macro+"M";
    for(int i=begin+len;i<int(s.size());){
      if(s.compare(i,len,macro)==0){result+='P';i+=len;}
      else result+=s[i++];
    }
    return result;
  }
  pair<string,int> reroute(const Trace& trace,const string& macro)const{
    const int begin=trace.commands.find(macro),len=macro.size();
    string out=trace.commands.substr(0,begin)+"M"+macro+"M";
    int here=0,completed=0,expanded=begin+len;
    for(int i=0;i<expanded;++i){
      char c=trace.commands[i];
      if(c=='S')++completed;
      else here=pose[here][labels.find(c)].to;
    }
    auto g=pose;
    for(int v=0;v<int(pose.size());++v){
      int u=v;
      for(char c:macro)u=pose[u][labels.find(c)].to;
      g[v].push_back({u,len});
    }
    UnitLexicographicBfs bfs(pose.size());
    for(int i=completed;i<int(trace.stops.size());++i){
      bfs.run(g,here);
      here=best_pose(bfs,trace.stops[i]);
      expanded+=1+bfs.secondary[here];
      if(expanded>T)return {{},expanded};
      append(out,bfs,here);out+='S';
    }
    return {move(out),expanded};
  }
  void solve(Clock::time_point deadline)const{
    auto start=plain(false),alternate=plain(true);
    if(alternate.commands.size()<start.commands.size()&&int(alternate.commands.size())<=T)
      start=move(alternate);
    if(int(start.commands.size())>T){
      for(int i=0;i<T;++i)cout<<start.commands[i]<<'\n';
      return;
    }
    const int literal=start.commands.size();
    auto list=macros(start.commands);
    string best=start.commands;
    int expanded=literal;
    for(const auto& macro:list){
      auto next=compress(start.commands,macro);
      if(next.size()<best.size())best=move(next);
    }
    const int base=best.size();
    if(AHC066_REPLAN)for(const auto& macro:list){
      if(Clock::now()>=deadline)break;
      auto [route,cost]=reroute(start,macro);
      if(!route.empty()&&route.size()<best.size()){
        best=move(route);expanded=cost;
      }
    }
    assert(int(best.size())<=T&&expanded<=T);
    cerr<<"literal="<<literal<<" base="<<base<<" score="<<best.size()<<" expanded="<<expanded<<'\n';
    for(char c:best)cout<<c<<'\n';
  }
};
}
int main(){
  auto deadline=Clock::now()+chrono::milliseconds(1380);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Robot a;a.read();a.solve(deadline);
}
