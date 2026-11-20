#include <bits/stdc++.h>
#include "../../library/unit-lexicographic-bfs.hpp"
using namespace std;
#ifndef AHC066_JOINT_ORDER
#define AHC066_JOINT_ORDER 0
#endif
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
#if AHC066_JOINT_ORDER
  struct Arc{int keys,expanded;};
  using Result=pair<int,int>;
  struct Matrix {
    vector<int> locations;
    vector<vector<Edge>> graph;
    vector<vector<Arc>> entries;
    Matrix(const Robot& r,const string& macro,int first_cell,Clock::time_point deadline) {
      graph=r.pose;
      for(int v=0;v<int(graph.size());++v){
        int u=v;
        for(char c:macro)u=r.pose[u][labels.find(c)].to;
        graph[v].push_back({u,int(macro.size())});
      }
      locations.push_back(first_cell);
      for(int i=0;i<r.m;++i){locations.push_back(r.ball[i]);locations.push_back(r.basket[i]);}
      // One pose BFS per required heading, not one per tested global order.
      const int k=locations.size()*4;
      entries.assign(k,vector<Arc>(k));
      UnitLexicographicBfs bfs(graph.size());
      for(int a=0;a<k;++a){
        if(Clock::now()>=deadline){entries.clear();return;}
        bfs.run(graph,4*locations[a/4]+a%4);
        for(int b=0;b<k;++b){
          int target=4*locations[b/4]+b%4;
          assert(bfs.length[target]>=0);
          entries[a][b]={bfs.length[target],bfs.secondary[target]};
        }
      }
    }
  };
  static Result ordered_cost(const Matrix& costs,int source,const vector<int>& order,
                             vector<array<int,4>>* parent=nullptr){
    constexpr int inf=1<<28;
    array<Result,4> layer{};layer.fill({inf,inf});
    layer[source%4]={0,0};
    int last=source/4;
    if(parent)parent->clear();
    for(int id:order)for(int next:{2*id+1,2*id+2}){
      array<Result,4> fresh{};fresh.fill({inf,inf});
      array<int,4> from{};
      for(int i=0;i<4;++i)for(int j=0;j<4;++j){
        const auto e=costs.entries[4*last+i][4*next+j];
        Result c={layer[i].first+e.keys+1,layer[i].second+e.expanded+1};
        if(c<fresh[j])fresh[j]=c,from[j]=i;
      }
      if(parent)parent->push_back(from);
      last=next;layer=fresh;
    }
    auto it=min_element(layer.begin(),layer.end());
    return *it;
  }
  pair<string,int> joint_trial(const string& macro,const vector<int>& original_order,
                               bool reorder,Clock::time_point deadline,
                               int& matrices,int& probes)const{
    int after_macro=0;
    for(char c:macro)after_macro=pose[after_macro][labels.find(c)].to;
    Matrix costs(*this,macro,after_macro/4,deadline);
    if(costs.entries.empty())return {{},0};
    ++matrices;
    const int source=after_macro%4;
    vector<int> working=original_order,optimal=working;
    auto current=ordered_cost(costs,source,working);
    auto best=current;
    auto record=[&](const vector<int>& order,Result cost) {
      if(cost.second+int(macro.size())<=T &&
         (best.second+int(macro.size())>T || cost<best))
        optimal=order,best=cost;
    };
    if(reorder){
      // Pose-aware initial task order with constrained pickup/drop adjacency.
      vector<char> used(m);
      int here=source;
      for(int k=0;k<m;++k){
        pair<int,int> least={INT_MAX,INT_MAX};int target=-1,id=-1;
        for(int i=0;i<m;++i)if(!used[i]){
          for(int pick=4*(2*i+1);pick<4*(2*i+1)+4;++pick)
            for(int drop=pick+4-(pick%4);drop<pick+8-(pick%4);++drop){
              auto a=costs.entries[here][pick],b=costs.entries[pick][drop];
              pair score{a.keys+b.keys,a.expanded+b.expanded};
              if(score<least)least=score,target=drop,id=i;
            }
        }
        used[id]=true;working[k]=id;here=target;
      }
      current=ordered_cost(costs,source,working);++probes;
      record(working,current);
      if(ordered_cost(costs,source,original_order)<current)
        working=original_order,current=ordered_cost(costs,source,working);
      mt19937 random(0x06600167);
      for(int i=1;i<48 && Clock::now()<deadline && m>1;++i){
        int a=random()%m,b=random()%m;
        if(a==b)b=(b+1)%m;
        auto next=working;
        if(i%2)swap(next[a],next[b]);
        else if(a<b)rotate(next.begin()+a,next.begin()+a+1,next.begin()+b+1);
        else rotate(next.begin()+b,next.begin()+a,next.begin()+a+1);
        auto value=ordered_cost(costs,source,next);++probes;
        record(next,value);
        if(value<current)working=move(next),current=value;
      }
    }
    int expanded=best.second+int(macro.size());
    if(expanded>T || Clock::now()>=deadline)return {{},expanded};
    // Re-run the exact same lexicographic 4-state transfer and save arrows.
    vector<array<int,4>> parent;
    ordered_cost(costs,source,optimal,&parent);
    vector<int> stops;
    for(int id:optimal){stops.push_back(2*id+1);stops.push_back(2*id+2);}
    // Select terminal heading by excluding all other headings in a final
    // four-state transfer. The penultimate path can still choose any heading.
    int goal=-1;
    if(!stops.empty()){
      for(int heading=0;heading<4 && goal<0;++heading){
        vector<int> ends(stops.size());
        int d=heading;
        for(int k=int(stops.size())-1;k>=0;--k){
          ends[k]=4*stops[k]+d;d=parent[k][d];
        }
        int u=source;
        Result cost{};
        for(int v:ends){auto c=costs.entries[u][v];cost.first+=c.keys+1;cost.second+=c.expanded+1;u=v;}
        if(cost==best)goal=heading;
      }
    }
    assert(goal>=0);
    vector<int> ends(stops.size());
    for(int k=int(stops.size())-1;k>=0;--k){
      ends[k]=4*stops[k]+goal;goal=parent[k][goal];
    }
    string out="M"+macro+"M";
    int u=4*costs.locations[source/4]+source%4;
    UnitLexicographicBfs bfs(pose.size());
    for(int v:ends){
      if(Clock::now()>=deadline)return {{},expanded};
      bfs.run(costs.graph,u);
      u=4*costs.locations[v/4]+v%4;
      append(out,bfs,u);out+='S';
    }
    assert(int(out.size())==best.first+int(macro.size())+2);
    return {move(out),expanded};
  }
#endif
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
#if AHC066_JOINT_ORDER
    const int current_templates=best.size();
    int matrices=0, probes=0;
    vector<int> order;
    for(int i=0;i<int(start.stops.size());i+=2)
      order.push_back(find(ball.begin(),ball.end(),start.stops[i])-ball.begin());
    vector<string> dictionary;
    if(AHC066_JOINT_ORDER==1)dictionary={"FFFFF","FFFFFFF",string(max(8,n*2/3),'F'),
                                         "FFRFFFR","FFLFFFL","FFRFFFFL"};
    else dictionary={"FFFFFF",string(n-1,'F'),"FFRFFF","FFLFFF","FFRFFL","FFLFFR"};
    for(const auto& macro:dictionary){
      if(Clock::now()>=deadline)break;
      accept(joint_trial(macro,order,AHC066_JOINT_ORDER==2,deadline,matrices,probes));
    }
#endif
    assert(int(best.size())<=T&&expanded<=T);
    cerr<<"literal="<<literal<<" base="<<base<<" score="<<best.size()<<" expanded="<<expanded;
    if(AHC066_TEMPLATES)cerr<<" frozen="<<frozen<<" trials="<<trials;
#if AHC066_JOINT_ORDER
    cerr<<" before_joint="<<current_templates<<" matrices="<<matrices<<" probes="<<probes;
#endif
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
