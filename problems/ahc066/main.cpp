#include <bits/stdc++.h>
#include "../../library/unit-lexicographic-bfs.hpp"
using namespace std;
#ifndef AHC066_REPLAN
#define AHC066_REPLAN 1
#endif
#ifndef AHC066_TEMPLATES
#define AHC066_TEMPLATES 2
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
    for(int i=0;i<min(20,int(rank.size()));++i)result.push_back(rank[i].second);
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
  vector<string> geometry_macros()const{
    return {"FFFF","FFFFFF",string(max(7,n/2),'F'),string(n-1,'F'),
            "FFRFFF","FFLFFF","FFRFFL","FFLFFR","FFFRFFFL","FFFLFFFR"};
  }
  pair<string,int> reroute(const Trace& trace,const string& macro,int prefix=-1)const{
    const int begin=prefix<0?int(trace.commands.find(macro)):prefix,len=macro.size();
    string out=trace.commands.substr(0,begin)+"M"+macro+"M";
    int here=0,completed=0,expanded=begin+len;
    if(expanded>T)return {{},expanded};
    auto play_basic=[&](char c){
      if(c=='S')++completed;
      else here=pose[here][labels.find(c)].to;
    };
    for(int i=0;i<begin;++i)play_basic(trace.commands[i]);
    for(char c:macro)play_basic(c);
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
    const int common=min(10,int(list.size()));
    string best=start.commands;
    int expanded=literal;
    for(int i=0;i<common;++i){
      auto next=compress(start.commands,list[i]);
      if(next.size()<best.size())best=move(next);
    }
    const int base=best.size();
    auto accept=[&](pair<string,int> result){
      auto& [route,cost]=result;
      if(!route.empty()&&route.size()<best.size()){best=move(route);expanded=cost;}
    };
    if(AHC066_REPLAN)for(int i=0;i<common&&Clock::now()<deadline;++i)
      accept(reroute(start,list[i]));
    const int frozen=best.size();
    int trials=0;
    if(AHC066_TEMPLATES==1)for(int i=0;i<10&&Clock::now()<deadline;++i){
      // If the trace has fewer than twenty candidates repeat a prior trial, so
      // the control never gets a smaller evaluation budget than the variant.
      if(!list.empty())accept(reroute(start,list[(i+10)%list.size()]));
      ++trials;
    }
    if(AHC066_TEMPLATES==2)for(const auto& macro:geometry_macros()){
      if(Clock::now()>=deadline)break;
      accept(reroute(start,macro,0));++trials;
    }
    assert(int(best.size())<=T&&expanded<=T);
    cerr<<"literal="<<literal<<" base="<<base<<" score="<<best.size()<<" expanded="<<expanded;
    if(AHC066_TEMPLATES)cerr<<" frozen="<<frozen<<" trials="<<trials;
    cerr<<'\n';
    for(char c:best)cout<<c<<'\n';
  }
};
}
int main(){
  auto deadline=Clock::now()+chrono::milliseconds(1380);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Robot a;a.read();a.solve(deadline);
}
