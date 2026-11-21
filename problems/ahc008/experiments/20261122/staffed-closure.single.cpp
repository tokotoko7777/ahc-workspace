#pragma GCC optimize("O3")
#include <bits/stdc++.h>
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/max-flow.hpp

// Dinic法による最大流。整数容量で使う。
// 使い方:
// MaxFlow<long long> flow(n);
// flow.add_edge(from, to, capacity);
// long long answer = flow.flow(source, sink);
template <class Capacity>
struct MaxFlow {
  struct Edge {
    int to;
    int reverse_index;
    Capacity capacity;
  };

  struct EdgeInfo {
    int from;
    int to;
    Capacity capacity;
    Capacity flow;
  };

  int n;
  std::vector<std::vector<Edge>> graph;
  std::vector<std::pair<int, int>> edge_positions;
  std::vector<int> level;
  std::vector<int> next_edge;

  explicit MaxFlow(int vertex_count)
      : n(vertex_count), graph(vertex_count), level(vertex_count),
        next_edge(vertex_count) {
    assert(vertex_count >= 0);
  }

  int add_edge(int from, int to, Capacity capacity) {
    assert(0 <= from && from < n && 0 <= to && to < n);
    assert(capacity >= Capacity{});
    const int index = static_cast<int>(edge_positions.size());
    edge_positions.push_back({from, static_cast<int>(graph[from].size())});
    const int from_index = static_cast<int>(graph[from].size());
    int to_index = static_cast<int>(graph[to].size());
    if (from == to) ++to_index;
    graph[from].push_back({to, to_index, capacity});
    graph[to].push_back({from, from_index, Capacity{}});
    return index;
  }

  EdgeInfo get_edge(int index) const {
    assert(0 <= index && index < static_cast<int>(edge_positions.size()));
    const auto [from, position] = edge_positions[index];
    const Edge& edge = graph[from][position];
    const Edge& reverse = graph[edge.to][edge.reverse_index];
    return {from, edge.to, edge.capacity + reverse.capacity,
            reverse.capacity};
  }

  std::vector<EdgeInfo> edges() const {
    std::vector<EdgeInfo> result;
    result.reserve(edge_positions.size());
    for (int i = 0; i < static_cast<int>(edge_positions.size()); ++i) {
      result.push_back(get_edge(i));
    }
    return result;
  }

  Capacity flow(int source, int sink,
                Capacity limit = std::numeric_limits<Capacity>::max()) {
    assert(0 <= source && source < n && 0 <= sink && sink < n);
    assert(source != sink && limit >= Capacity{});
    Capacity result{};
    while (result < limit && build_level_graph(source, sink)) {
      std::fill(next_edge.begin(), next_edge.end(), 0);
      while (result < limit) {
        const Capacity pushed =
            send_flow(source, sink, limit - result);
        if (pushed == Capacity{}) break;
        result += pushed;
      }
    }
    return result;
  }

  // 現在の残余グラフでsourceから到達できる頂点。最大流後は最小カットになる。
  std::vector<char> min_cut(int source) const {
    assert(0 <= source && source < n);
    std::vector<char> reachable(n, false);
    std::vector<int> queue;
    queue.reserve(n);
    reachable[source] = true;
    queue.push_back(source);
    for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
      const int vertex = queue[head];
      for (const Edge& edge : graph[vertex]) {
        if (edge.capacity == Capacity{} || reachable[edge.to]) continue;
        reachable[edge.to] = true;
        queue.push_back(edge.to);
      }
    }
    return reachable;
  }

 private:
  bool build_level_graph(int source, int sink) {
    std::fill(level.begin(), level.end(), -1);
    std::vector<int> queue;
    queue.reserve(n);
    level[source] = 0;
    queue.push_back(source);
    for (int head = 0; head < static_cast<int>(queue.size()); ++head) {
      const int vertex = queue[head];
      for (const Edge& edge : graph[vertex]) {
        if (edge.capacity == Capacity{} || level[edge.to] != -1) continue;
        level[edge.to] = level[vertex] + 1;
        queue.push_back(edge.to);
      }
    }
    return level[sink] != -1;
  }

  Capacity send_flow(int vertex, int sink, Capacity upper_bound) {
    if (vertex == sink) return upper_bound;
    for (int& index = next_edge[vertex];
         index < static_cast<int>(graph[vertex].size()); ++index) {
      Edge& edge = graph[vertex][index];
      if (edge.capacity == Capacity{} ||
          level[edge.to] != level[vertex] + 1) {
        continue;
      }
      const Capacity pushed =
          send_flow(edge.to, sink, std::min(upper_bound, edge.capacity));
      if (pushed == Capacity{}) continue;
      edge.capacity -= pushed;
      graph[edge.to][edge.reverse_index].capacity += pushed;
      return pushed;
    }
    return Capacity{};
  }
};
#include <algorithm>
#include <cassert>
#include <vector>

// 四近傍の平坦グリッド。多数の候補への最初の一歩を、経路復元なしで引く。
// 例: bfs.run(start, [&](int cell) { return !wall[cell]; });
//     int next = bfs.first_step(goal); // 到達不能なら -1、goal==start なら start
// 行優先 index = row * width + col。run() ごとに内部領域を再利用する。
class FlatGridFirstStepBfs {
 public:
  FlatGridFirstStepBfs(int height, int width)
      : width_(width), dist_(height * width), first_(height * width),
        queue_(height * width) {
    assert(height > 0 && width > 0);
  }

  template <class CanEnter>
  void run(int start, CanEnter can_enter) {
    const int n = static_cast<int>(dist_.size());
    assert(0 <= start && start < n && can_enter(start));
    std::fill(dist_.begin(), dist_.end(), -1);
    dist_[start] = 0;
    first_[start] = start;
    int top = 0, end = 1;
    queue_[0] = start;
    while (top < end) {
      const int v = queue_[top++];
      const auto push = [&](int u) {
        if (dist_[u] >= 0 || !can_enter(u)) return;
        dist_[u] = dist_[v] + 1;
        first_[u] = v == start ? u : first_[v];
        queue_[end++] = u;
      };
      if (v >= width_) push(v - width_);
      if (v + width_ < n) push(v + width_);
      if (v % width_ != 0) push(v - 1);
      if (v % width_ != width_ - 1) push(v + 1);
    }
  }

  int distance(int cell) const { return dist_[cell]; }
  int first_step(int cell) const { return dist_[cell] < 0 ? -1 : first_[cell]; }

 private:
  int width_;
  std::vector<int> dist_, first_, queue_;
};
using namespace std;
namespace {
constexpr int S=30, V=S*S;
constexpr char MOVE[]="UDLR", BUILD[]="udlr";
array<array<int,4>,V> nb;
int pos(int a,int b) {return a*S+b;}
int dir(int a,int b) {for(int d=0;d<4;d++)if(nb[a][d]==b)return d; return -1;}
int md(int a,int b){return abs(a/S-b/S)+abs(a%S-b%S);}
struct Pet {int p,t; float vx=0.,vy=0.; int prev=-1;};
struct Pen {vector<int> wall,gate,inside; int cap=0, cx=-1, cy=-1; bool captured=false;};
struct Worker {
 FlatGridFirstStepBfs bfs{S,S};
 vector<pair<int,int>> moves;
};
struct Game {
 vector<Pet> pets; vector<int> humans; vector<Pen> pens; vector<Worker> workers;
 array<unsigned char,V> wall{}, avoid{}, block{}, need{};
 string act;
 int turn=0, cap_mode=0, layout=1, nslots=6, width=6, length=30, mouths=8;
 void init(int argc,char** argv) {
  if(argc>1) layout=atoi(argv[1]);
  if(argc>2) nslots=atoi(argv[2]);
  if(argc>3) width=atoi(argv[3]);
  if(argc>4) length=atoi(argv[4]);
  if(argc>5) mouths=atoi(argv[5]);
  if(argc>6) cap_mode=atoi(argv[6]);
  if(argc>7) lineTurn=atoi(argv[7]);
  if(argc>8) lineMax=atoi(argv[8]);
  if(argc>9) lineSmart=atoi(argv[9]);
  if(argc>10) cohesion=atoi(argv[10]);
  if(argc>11) multi=atoi(argv[11]);
  if(argc>12) secdist=atoi(argv[12]);
  if(argc>13) multiMin=atoi(argv[13]);
  if(argc>14) forecastMode=atoi(argv[14]);
  if(argc>15) predT=atoi(argv[15]);
  if(argc>16) predW=atoi(argv[16]);
  for(int x=0;x<S;x++)for(int y=0;y<S;y++)
   nb[pos(x,y)]={x?pos(x-1,y):-1,x<S-1?pos(x+1,y):-1,y?pos(x,y-1):-1,y<S-1?pos(x,y+1):-1};
  int n,m,r,c,t;cin>>n; pets.resize(n);
  for(auto &p:pets){cin>>r>>c>>t;p={pos(r-1,c-1),t};}
  cin>>m; humans.resize(m); workers.resize(m);
  for(int& p:humans){cin>>r>>c;p=pos(r-1,c-1);}
  if(argc>17) fmode=atoi(argv[17]);
  if(argc>18) fturn=atoi(argv[18]);
  form();
 }
 int phase=0, target=-1, radius=3, tag=0, limit=20, bestPrev=0;
 int phase2=0, target2=-1, bestPrev2=0;
 int multi=1, secdist=12, multiMin=3;
 int forecastMode=1, predT=8, predW=20;
 vector<int> pdeg[V];
 vector<array<float,V>> forecast;
 int fstamp=-1;
 array<unsigned char,V> globalset{}, globalmoved{};
 int dogPhase=0, dogMode=0, dogStart=0, dogClosed=0, dogAnchor=-1, dogGate=-1, dogOut=-1, dogDecoy=-1;
 int dogInsideDist=9, dogSide=0, dogCount=0, dogSaveLimit=18;
 vector<int> dogWalls, dogInside;
 array<unsigned char,V> dogMask{};
 bool captiveHuman(int i) const {return dogDecoy==i&&dogClosed;}
 void designDog(){
  dogMode=layout;
  if(!dogMode||count_if(pets.begin(),pets.end(),[](Pet p){return p.t==4;})==0){dogPhase=9;return;}
  double best=-1e9;
  for(int z=0;z<4;z++)for(int dep=max(4,width+4);dep<=min(18,width+9);dep++){
   int ax=z&1?29:0, ay=z&2?29:0;
   double cost=0,extra=0;
   for(int p:humans)cost+=md(p,pos(ax,ay))*.19;
   for(auto p:pets){
    int d=md(p.p,pos(ax,ay));
    if(p.t==4)cost+=d*.30;
    else {
     double scale=p.t==1?3.4:p.t==2?4.8:p.t==3?5.3:8.6;
     extra+=1./(1.+exp((d-dep+1)/scale));
    }
   }
   double v=-cost+extra*(tag<=1?12:20) -(dep+1)*dep*.13;
   if(best<v)best=v,dogSide=z,dogInsideDist=dep;
  }
  auto cell=[&](int x,int y){return pos((dogSide&1)?29-x:x,(dogSide&2)?29-y:y);};
  for(int x=0;x<=dogInsideDist;x++)for(int y=0;y<=dogInsideDist-x;y++){
   if(x+y==dogInsideDist){
    if(x==dogInsideDist/2)dogGate=cell(x,y),dogOut=cell(x+1,y);
    else dogWalls.push_back(cell(x,y));
   }else dogInside.push_back(cell(x,y)),dogMask[cell(x,y)]=1;
  }
  dogAnchor=cell(0,0);
 }

 vector<Pen> pockets;
 array<int,V> group{}, seen{}, currentCt{};
 int groupN=0;

 struct Separator {vector<int> come; int mode=0, from=0, at=0;};
 vector<Separator> separators;
 int lineId=-1, lineStart=0, lineH=0, lineBase=0;
 int lineTurn=210, lineMax=21, lineSmart=0;
 int linStamp=0, cohesion=1;
 clock_t started=clock();
 bool overBudget() const {return clock()-started>1.65*CLOCKS_PER_SEC;}
 array<int,V> linTag{}, linLabel{};
 array<int,V> forbid{};
 int qline[V], lcount[V], lanimals[V];
 double linePrev=0.;
 bool validLine(int p,int d){
  int r=p/S+d/3-1,c=p%S+d%3-1;
  return r>=0&&r<S&&c>=0&&c<S;
 }
 void finiteLines(){
  for(int dx=0;dx<=1;dx++)for(int dy=-1;dy<=1;dy++){
   if(!dx&&dy!=1)continue;
   for(int r=0;r<S;r++)for(int c=0;c<S;c++){
    int y=r-dx,x=c-dy;
    if(y>=0&&y<S&&x>=0&&x<S)continue;
    Separator ls;
    for(y=r,x=c;y>=0&&y<S&&x>=0&&x<S;y+=dx,x+=dy)ls.come.push_back(pos(y,x));
    if(ls.come.size()>=3)separators.push_back(move(ls));
   }
  }
 }
 double rawscore(){
  vector<int> pp(groupN);
  for(auto p:pets)pp[group[p.p]]++;
  double v=0;for(int k=0;k<(int)humans.size();k++)
   if(!captiveHuman(k))v+=ldexp(currentCt[group[humans[k]]],-pp[group[humans[k]]]);
  return v;
 }
 pair<double,int> testLine(const vector<int>& line, int root, bool keep, int delay){
  for(int p:line)forbid[p]=1;
  ++linStamp;
  int labels=0;
  for(int p=0;p<V;p++)if(group[p]==root&&!forbid[p]&&linTag[p]!=linStamp){
   int n=1;qline[0]=p;linTag[p]=linStamp;linLabel[p]=labels;
   for(int i=0;i<n;i++)for(int q:nb[qline[i]])if(q>=0&&!forbid[q]&&group[q]==root&&linTag[q]!=linStamp)
    linTag[q]=linStamp,linLabel[q]=labels,qline[n++]=q;
   lcount[labels]=n;lanimals[labels++]=0;
  }
  for(int p:line)forbid[p]=0;
  if(labels<2)return {0.,-1};
  int target=-1;
  double best=0;
  for(auto p:pets)if(group[p.p]==root&&linTag[p.p]==linStamp)lanimals[linLabel[p.p]]++;
  for(int k=0;k<labels;k++){
   if(lcount[k]<6)continue;
   double v=ldexp(lcount[k],-lanimals[k]);
   if(delay&&lineSmart>0) {
    double extra=0;
    for(auto p:pets)if(group[p.p]==root&&linTag[p.p]==linStamp&&linLabel[p.p]!=k){
     int d=99;for(int w:line)d=min(d,md(p.p,w));
     if(d<=delay/2+2)extra+=std::max(0.,1.-double(d)/double(delay/2+3))*(p.t>=4?1.3:.7);
    }
    v/=1.+extra*.32;
   }
   if(v>best)best=v,target=k;
  }
  return {best,target};
 }
 vector<int> makeLine(const Separator& s,int root,int a,int b) {
  vector<int> v;
  for(int i=a;i<b;i++){int p=s.come[i];if(group[p]==root)v.push_back(p);}
  return v;
 }
 bool latePlan(){
  if(turn<lineTurn||overBudget())return false;
  int hm=humans.size(),root=-1;
  for(int i=0;i<hm;i++)if(!captiveHuman(i)&&(root<0||currentCt[group[humans[i]]]>currentCt[root]))root=group[humans[i]];
  if(root<0)return false;
  int bn=0;for(auto p:pets)bn+=group[p.p]==root;
  if(!bn)return false;
  double current=ldexp(currentCt[root],-bn);
  vector<int> toBuild;
  if(lineId>=0){
   toBuild=makeLine(separators[lineId],lineH,lineBase,separators[lineId].at);
   if(toBuild.empty()||turn-lineStart>=35||lineH!=root)lineId=-1;
   else {
    auto [benefit,id]=testLine(toBuild,root,false,0);
    if(id<0||benefit<current*(turn<280?1.06:1.02)){lineId=-1;}
   }
  }
  if(lineId<0){
   double best=0.;
   int limit=min(lineMax,(299-turn)*max(1,int(hm)-1));
   array<int,V> nearwall{}, etd{};
   for(int p=0;p<V;p++) if(group[p]==root){
    for(int k=0;k<9;k++)if(k!=4&&validLine(p,k)&&wall[p+(k/3-1)*S+k%3-1])nearwall[p]=1;
    int d=99;
    for(int u=0;u<hm;u++)if(!captiveHuman(u))for(int q:nb[p])if(q>=0&&!wall[q]){
     int vv=workers[u].bfs.distance(q);
     if(vv>=0)d=min(d,vv);
    }
    etd[p]=d;
   }
   unordered_set<uint64_t> hashes;
   for(int j=0;j<(int)separators.size();j++){
    if(j%8==0&&overBudget())break;
    auto& line=separators[j];
    int n=line.come.size();
    for(int i=0;i<n;i++){
     if(group[line.come[i]]!=root)continue;
     if(i>0&&!nearwall[line.come[i]])continue;
     int added=0,maxEta=0;
     uint64_t hash=0;
     for(int e=i+1;e<=n;e++){
      int p=line.come[e-1];
      if(group[p]!=root)break;
      added++;
      if(added>limit)break;
      maxEta=max(maxEta,etd[p]);
      uint64_t z=p+0x9e3779b97f4a7c15ULL;
      z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;
      z=(z^(z>>27))*0x94d049bb133111ebULL;
      hash^=z^(z>>31);
      if(e<n&&!nearwall[line.come[e-1]])continue;
      int eta=maxEta;
      if(!hashes.insert(hash).second)continue;
      auto w=makeLine(line,root,i,e);
      eta+=((int)w.size()+hm-1)/hm*2;
      if(turn+eta>=300)continue;
      auto [benefit,id]=testLine(w,root,false,eta);
      if(id<0||benefit<current*1.05)continue;
      double v=(benefit-current)/(1.+eta*.13+w.size()*.07);
      if(v>best){best=v;lineId=j;lineBase=i;line.at=e;lineH=root;lineStart=turn;toBuild=move(w);}
     }
    }
   }
  }
  if(lineId<0)return false;
  auto [gain,keep]=testLine(toBuild,root,true,0);
  if(keep<0){lineId=-1;return false;}
  array<unsigned char,V> proposed{},moved{},selected{};
  for(int p:toBuild)selected[p]=1;
  vector<pair<int,int>> evacuate;
  bool canClose=true;
  for(int i=0;i<hm;i++)if(group[humans[i]]==root&&!captiveHuman(i)){
   int h=humans[i];
   if(linTag[h]==linStamp&&linLabel[h]==keep)continue;
   canClose=false;
   int best=999,next=-1;
   for(int p=0;p<V;p++)if(linTag[p]==linStamp&&linLabel[p]==keep){
    int d=workers[i].bfs.distance(p);
    if(d>=0&&d<best)best=d,next=workers[i].bfs.first_step(p);
   }
   if(next>=0&&next!=h)evacuate.emplace_back(i,next);
  }
  for(auto [i,p]:evacuate){
   act[i]=MOVE[dir(humans[i],p)];moved[p]=1;
  }
  vector<tuple<int,int,int,int>> jobs;
  for(int p:toBuild)for(int i=0;i<hm;i++)if(act[i]=='.'&&!captiveHuman(i)&&group[humans[i]]==root){
   for(int q:nb[p])if(q>=0&&!selected[q]&&!wall[q]&&linTag[q]==linStamp&&linLabel[q]==keep){
    int d=workers[i].bfs.distance(q);
    if(d<0)continue;
    jobs.emplace_back(-7*d-block[p]*(d==0?19:6),i,p,q);
   }
  }
  sort(jobs.rbegin(),jobs.rend());
  int nb=0,remain=toBuild.size();
  array<unsigned char,V> assigned{};
  for(auto [priority,i,p,q]:jobs)if(act[i]=='.'&&!assigned[p]){
   int h=humans[i];
   if(q==h){
    if(block[p]||moved[p]||(!canClose&&nb+1>=remain))continue;
    proposed[p]=1;act[i]=BUILD[dir(h,p)];nb++;
   }else{
    int next=workers[i].bfs.first_step(q);
    if(next<0||proposed[next])continue;
    act[i]=MOVE[dir(h,next)];moved[next]=1;
   }
   assigned[p]=1;
  }
  return true;
 }
 void form(){
  radius=nslots; limit=length; tag=mouths; designDog(); finiteLines();
  for(int x=0;x<S;x++)for(int y=0;y<S;y++)for(int rr:{2,3,4,5,6,7}){
   Pen z;
   z.cx=pos(x,y);z.cy=rr;
   for(int r=max(0,x-rr);r<=min(29,x+rr);r++)for(int c=max(0,y-rr);c<=min(29,y+rr);c++){
    int d=abs(x-r)+abs(y-c);
    if(d==rr)z.wall.push_back(pos(r,c));
    if(d<rr)z.inside.push_back(pos(r,c));
   }
   pockets.push_back(move(z));
  }
 }
 void prep(){
  act=string(humans.size(),'.');
  block=wall;need.fill(0);avoid.fill(0);
  for(auto p:pets){
   block[p.p]=1;
   for(int q:nb[p.p])if(q>=0)block[q]=1;
  }
  for(int p:humans)block[p]=1;
  for(int i=0;i<(int)humans.size();i++)workers[i].bfs.run(humans[i],[&](int p){return !wall[p];});
  group.fill(-1);groupN=0;
  int k[900],n=0;
  for(int p=0;p<V;p++)if(!wall[p]&&group[p]<0){
   n=1;k[0]=p;group[p]=groupN;
   for(int i=0;i<n;i++)for(int q:nb[k[i]])if(q>=0&&!wall[q]&&group[q]<0)group[q]=groupN,k[n++]=q;
   currentCt[groupN++]=n;
  }
 }
 void pred(){
  if(fstamp==turn)return;
  fstamp=turn;
  forecast.resize(3);
  for(auto& f:forecast)f.fill(0);
  for(int p=0;p<V;p++){
   pdeg[p].clear();
   if(wall[p])continue;
   for(int q:nb[p])if(q>=0&&!wall[q])pdeg[p].push_back(q);
  }
  array<int,V> hcount{};
  for(int i=0;i<(int)humans.size();i++)if(!captiveHuman(i))hcount[group[humans[i]]]++;
  for(auto p:pets)if(hcount[group[p.p]]){
   int idx=-1,dd=999;
   for(int i=0;i<(int)humans.size();i++)if(!captiveHuman(i)){
    int d=workers[i].bfs.distance(p.p);if(d>=0&&d<dd)idx=i,dd=d;
   }
   if(idx<0)continue;
   array<float,V> prob{},next{};
   prob[p.p]=1.f;
   int pass=0;
   for(int t=1;t<=predT*2;t++){
    int steps=p.t>=4?2:p.t;
    for(int u=0;u<steps;u++){
     next.fill(0.f);
     for(int q=0;q<V;q++)if(prob[q]>1e-8f){
      auto &v=pdeg[q];
      if(v.empty()){next[q]+=prob[q];continue;}
      if(p.t==4&&u==0&&forecastMode>=2){
       int best=workers[idx].bfs.distance(q);
       int copies=0;
       for(int r:v)copies+=workers[idx].bfs.distance(r)<best;
       if(copies){
        auto share=prob[q]/copies;
        for(int r:v)if(workers[idx].bfs.distance(r)<best)next[r]+=share;
        continue;
       }
      }
      if(p.t==5&&u==0&&forecastMode>=3){
       float wsum=0.f,w[4];
       for(int j=0;j<(int)v.size();j++){
        int r=v[j];
        w[j]=clamp(1.f+p.vx*(r/S-q/S)+p.vy*(r%S-q%S),.2f,1.8f);wsum+=w[j];
       }
       auto share=prob[q]/wsum;
       for(int j=0;j<(int)v.size();j++)next[v[j]]+=share*w[j];
       continue;
      }
      auto share=prob[q]/v.size();
      for(int r:v)next[r]+=share;
     }
     prob=next;
    }
    if(t==predT/2||t==predT||t==predT*2){
     if(pass<3)for(int q=0;q<V;q++)forecast[pass][q]+=prob[q];
     pass++;
    }
   }
  }
 }
 int select(vector<bool> eligible, int excluded=-1){
  vector<int> apos,at;vector<int> groups(groupN);
  for(int k=0;k<(int)humans.size();k++)if(eligible[k])groups[group[humans[k]]]++;
  for(auto p:pets)if(groups[group[p.p]])apos.push_back(p.p),at.push_back(p.t);
  if(apos.empty())return -1;
  if(forecastMode)pred();
  int ret=-1;
  double best=-1e9;
  for(int i=0;i<(int)pockets.size();i++) {
   auto& z=pockets[i];if(z.cy>radius)continue;
   if(excluded>=0){
    auto& orig=pockets[excluded];
    if(md(z.cx,orig.cx)<max(z.cy+orig.cy+2,secdist))continue;
   }
   double pd=1e9, pbenefit=0;
   int arr=999;
   for(int k=0;k<(int)humans.size();k++) if(eligible[k]) {
    int d=workers[k].bfs.distance(z.cx);
    if(d>=0)arr=min(arr,d);
   }
   if(arr>=18)continue;
   int left=0,blockct=0,area=0,hct=0, adjacentwalls=0;
   for(int q:z.wall){left+=!wall[q];blockct+=block[q]&&!wall[q];}
   if(!left)continue;
   for(int q:z.inside)if(!wall[q]){
    area++;
    for(int q2:nb[q])if(q2<0||wall[q2])adjacentwalls++;
   }
   for(int q:humans)if(md(q,z.cx)<z.cy)hct++;
   if(!area)continue;
   for(int j=0;j<(int)apos.size();j++){
    if(group[apos[j]]!=group[z.cx]&&group[z.cx]>=0)continue;
    int d=md(apos[j],z.cx);
    pd=min(pd,double(d));
    double u=at[j]==4?0.65:at[j]==1?1.24:1.0;
    pbenefit+=u*max(0.,1.5-(double(d))/double(z.cy+1));
   }
   if(pd>z.cy+(forecastMode?4:1))continue;
   double v=(pbenefit*30.0+adjacentwalls*0.10)/(1.+arr*.07+left*.095+blockct*.03)-area*.17+min(hct,3)*.5;
   if(tag==2)v-=area*.33;
   if(tag==3)v+=min(z.cy-2,2)*3;
   if(tag==7)v+=min(z.cy-2,3)*2.5;
   if(tag==8)v+=min(z.cy-2,4)*2;
   if(tag==9)v+=min(z.cy-2,5)*1.4;
   if(tag==4)v+=(z.cy-2)*max(0.,(100.-turn)/35.);
   if(tag==5)v+=hct*1.5;
   if(tag==6)v+=(z.cy-2)*1.6;
   if(forecastMode){
    int k=arr+left/3<=predT/2?0:arr+left/3<=predT?1:2;
    double e=0.;for(int q:z.inside)e+=forecast[k][q];
    double u=predW*e/(1.+arr*.055+left*.07);
    double penalty=(pd>z.cy+1)?(pd-z.cy-1)*.8:0.;
    v+=u-penalty;
   }
   if(best<v)best=v,ret=i;
  }
  return ret;
 }
 bool dogPlan(){
  if(dogPhase>=9)return false;
  vector<int> ls;
  for(int w:dogWalls)if(!wall[w])ls.push_back(w);
  int hm=humans.size(), dins=0, dogFar=0, dnum=0, pins=0, hin=0;
  for(auto p:pets){
   int inside=dogMask[p.p];
   pins+=inside;
   if(p.t==4){dins+=inside; dnum++; dogFar+=inside&&md(p.p,dogGate)>=4;}
  }
  for(int h:humans)hin+=dogMask[h];
  auto walk=[&](int i,int p,auto& placed){
   int h=humans[i],q=workers[i].bfs.first_step(p);
   if(q>=0&&q!=h&&!placed[q])act[i]=MOVE[dir(h,q)];
  };
  array<unsigned char,V> placed{}, moved{};
  if(dogPhase==0){
   if(ls.empty())dogPhase=1;
   else {
    vector<tuple<int,int,int,int>> choices;
    for(int i=0;i<hm;i++)for(int w:ls)for(int q:nb[w]){
     if(q<0||wall[q]||q==dogGate||find(dogWalls.begin(),dogWalls.end(),q)!=dogWalls.end())continue;
     int d=workers[i].bfs.distance(q);
     if(d<0)continue;
     int key=-7*d-dogMask[q]*2-block[w]*(d==0?18:6);
     choices.emplace_back(key,i,q,w);
    }
    sort(choices.rbegin(),choices.rend());
    for(auto [key,i,q,w]:choices)if(act[i]=='.'&&!placed[w]){
     if(humans[i]==q){
      if(!block[w]&&!moved[w])act[i]=BUILD[dir(q,w)],placed[w]=1;
     }else if(!moved[w]){
      walk(i,q,placed);
      if(act[i]!='.')moved[nb[humans[i]][find(MOVE,MOVE+4,act[i])-MOVE]]=1;
     }
    }
    for(int i=0;i<hm;i++)if(act[i]=='.')walk(i,dogAnchor,placed);
    if(turn<70)return true;
    dogPhase=9;return false;
   }
  }
  if(dogPhase==1){
   bool trigger=dins==dnum && (dogFar==dnum || turn>95);
   if(turn>130)trigger=dins>=max(1,dnum-1);
   if(turn>150)trigger=dins>0;
   if(trigger)dogPhase=2,dogStart=turn,dogCount=dins;
   else if(turn>=155){dogPhase=9;return false;}
   else {for(int i=0;i<hm;i++)walk(i,dogAnchor,placed);return true;}
  }
  if(dogPhase==2){
   if(dogDecoy<0 && dogMode!=2){
    int best=10000;
    for(int i=0;i<hm;i++){int d=workers[i].bfs.distance(dogAnchor); if(d>=0&&d<best)best=d,dogDecoy=i;}
   }
   int outside=-1;
   for(int i=0;i<hm;i++)if(i!=dogDecoy&&humans[i]==dogOut){outside=i;break;}
   bool can=outside>=0&&!block[dogGate]&&dins>0&&(dins>=dogCount||turn>dogStart+dogSaveLimit)&&
            hin==(dogDecoy>=0&&dogMask[humans[dogDecoy]]);
   for(int i=0;i<hm;i++){
    if(i==dogDecoy)walk(i,dogAnchor,placed);
    else if(can&&i==outside){act[i]=BUILD[dir(dogOut,dogGate)];}
    else walk(i,dogOut,placed);
   }
   if(can){dogClosed=1; dogPhase=9;}
   else if(turn>dogStart+50){dogDecoy=-1;dogPhase=9;return false;}
   return true;
  }
  return false;
 }
 void pocketPlan(vector<bool> canWork, int &target,int &phase,int& bestPrev,int excluded){
  need.fill(0);avoid.fill(0);
  if(target>=0){
   auto& z=pockets[target];
   int remain=0;
   for(int p:z.wall)remain+=!wall[p];
   if(!remain){target=-1;}
   else if(turn-phase>=limit || (excluded>=0&&md(z.cx,pockets[excluded].cx)<max(z.cy+pockets[excluded].cy+2,secdist)))target=-1;
   else if(turn-phase>=5&&remain>=bestPrev-1){
    int d=999;
    for(auto p:pets)d=min(d,md(p.p,z.cx));
    if(d>z.cy+1)target=-1;
   }
  }
  if(target<0){
   target=select(canWork,excluded);phase=turn;
   if(target<0)return;
   bestPrev=0;
   for(int p:pockets[target].wall)bestPrev+=!wall[p];
  }
  auto& z=pockets[target];
  for(int p:z.wall)need[p]=1;
  for(int p:z.inside)avoid[p]=1;
  int humans_in=0, animal_in=0,remain=0;
  for(int p:z.wall)remain+=!wall[p];
  for(int i=0;i<(int)humans.size();i++)if(!captiveHuman(i))humans_in+=avoid[humans[i]];
  for(auto p:pets)animal_in+=avoid[p.p];
  if(!animal_in&&turn-phase>=5){
   int mdist=999;
   for(auto p:pets)if(group[p.p]>=0&&currentCt[group[p.p]]>=40)mdist=min(mdist,md(z.cx,p.p));
   if(mdist>z.cy+3){target=-1;return;}
  }
  struct Job {int p,v;};
  vector<Job> jobs;
  for(int p:z.wall)if(!wall[p])jobs.push_back({p,100-2*block[p]});
  vector<tuple<int,int,int,int>> pairings;
  for(int j=0;j<(int)jobs.size();j++){
   auto job=jobs[j];int p=job.p;
   for(int i=0;i<(int)humans.size();i++)if(canWork[i]&&act[i]=='.'){
    int best=-1,key=-100000;
    for(int q:nb[p])if(q>=0&&!wall[q]&&!avoid[q]&&!need[q]){
     int d=workers[i].bfs.distance(q);
     if(d<0)continue;
     int f=job.v-6*d-block[p]*(d==0?22:5);
     if(f>key)best=q,key=f;
    }
    if(best>=0)pairings.emplace_back(key,i,j,best);
   }
  }
  sort(pairings.rbegin(),pairings.rend());
  auto& set=globalset;auto& moved=globalmoved;
  vector<bool> busy(humans.size()), assigned(jobs.size());
  int builds=0;
  for(auto [key,i,j,q]:pairings)if(!busy[i]&&!assigned[j]) {
   int p=jobs[j].p,h=humans[i];
   if(h==q){
    if(block[p]||moved[p]||set[p]||(humans_in&&builds+1>=remain))continue;
    act[i]=BUILD[dir(h,p)];set[p]=1;builds++;
   }else{
    int next=workers[i].bfs.first_step(q);
    if(next<0||set[next])continue;
    act[i]=MOVE[dir(h,next)];moved[next]=1;
   }
   busy[i]=true;assigned[j]=true;
  }
  for(int i=0;i<(int)humans.size();i++)if(canWork[i]&&act[i]=='.'&&!busy[i]&&avoid[humans[i]]) {
   int h=humans[i],best=-1,key=99999;
   for(int p=0;p<V;p++)if(!avoid[p]&&!need[p]&&!wall[p]){
    int d=workers[i].bfs.distance(p);
    if(d>=0&&d<key){int q=workers[i].bfs.first_step(p);if(q>=0&&!set[q])key=d,best=q;}
   }
   if(best>=0)act[i]=MOVE[dir(h,best)],moved[best]=1;
  }
  return;
 }

 int fmode=3, fdone=0, fg=0, fsel=0, fturn=125;
 // Compress all non-buildable grid components; the only remaining individual
 // vertices are legal walls with currently adjacent eligible workers.
 bool compressedFlow(){
  if(!fmode||turn<fturn||overBudget())return false;
  const int nh=humans.size(),inf=10000000;
  array<int,V> id,staff{};
  id.fill(-1);
  vector<int> gates;
  for(int i=0;i<nh;i++)if(!captiveHuman(i))for(int p:nb[humans[i]])if(p>=0&&!block[p])staff[p]|=1<<i;
  for(int i=0;i<nh;i++)if(!captiveHuman(i))for(int p:nb[humans[i]])if(p>=0&&staff[p]&&id[p]<0){
   id[p]=gates.size();gates.push_back(p);
  }
  if(gates.empty())return false;
  int ng=gates.size(),nc=ng;
  vector<int> area(ng,1), cnt(ng), hc(ng);
  int bfs[V];
  for(int p=0;p<V;p++)if(!wall[p]&&id[p]<0){
   int n=1;bfs[0]=p;id[p]=nc;
   for(int i=0;i<n;i++)for(int q:nb[bfs[i]])if(q>=0&&!wall[q]&&id[q]<0)id[q]=nc,bfs[n++]=q;
   area.push_back(n);cnt.push_back(0);hc.push_back(0);nc++;
  }
  for(auto p:pets)cnt[id[p.p]]++;
  for(int i=0;i<nh;i++)if(!captiveHuman(i))hc[id[humans[i]]]++;
  // Only adjacency incident to staffed gates remains.
  vector<pair<int,int>> adj;
  vector<vector<int>> chain(nc);
  for(int g=0;g<ng;g++)for(int q:nb[gates[g]])if(q>=0&&id[q]>=0){
   int a=id[q];
   if(a<g)continue;
   adj.emplace_back(a,g);
   chain[a].push_back(g);chain[g].push_back(a);
  }
  double old=0.,best=0.;
  vector<int> gp(groupN),gh(groupN);for(auto p:pets)gp[group[p.p]]++;
  for(int i=0;i<nh;i++)if(!captiveHuman(i))gh[group[humans[i]]]++;
  for(int g=0;g<groupN;g++)old+=ldexp(1.*gh[g]*currentCt[g],-gp[g]);
  string chosen=act;
  auto score=[&](const string& a){
   array<bool,V> blocked{};
   vector<int> hp=humans;
   for(int i=0;i<nh;i++)if(a[i]!='.'){
    if(a[i]>='a')blocked[nb[humans[i]][find(BUILD,BUILD+4,a[i])-BUILD]]=1;
    else hp[i]=nb[humans[i]][find(MOVE,MOVE+4,a[i])-MOVE];
   }
   for(int i=0;i<nh;i++)if(blocked[hp[i]])return -1.;
   array<int,V> hist{},petAt{};for(auto p:pets)petAt[p.p]++;
   vector<int> seen_g(groupN);
   double total=0;
   for(int i=0;i<nh;i++)if(!captiveHuman(i)&&!hist[hp[i]]){
    if(seen_g[group[humans[i]]]++)return -1.;
    int n=1,b=0,h=0;hist[hp[i]]=i+1;bfs[0]=hp[i];
    for(int j=0;j<n;j++){int p=bfs[j];b+=petAt[p];for(int q:nb[p])if(q>=0&&!wall[q]&&!blocked[q]&&!hist[q]){
     hist[q]=i+1;bfs[n++]=q;
    }}
    for(int j=0;j<nh;j++)if(!captiveHuman(j)&&hist[hp[j]]==i+1)h++;
    total+=ldexp(1.*h*n,-b);
   }
   return total;
  };
  vector<int> orders{64,128,256,512,1024};
  bool promising=false;
  for(int i=ng;i<nc;i++)if(cnt[i]&&!hc[i]&&!chain[i].empty())promising=true;
  if(!promising)return false;
  if(fmode>=3)old=max(old,score(act));
  auto check=[&](const vector<int>& cut,const vector<int>& use){
   vector<char> marked(nc);
   for(int q:cut)marked[q]=1;
   vector<int> seen_g(groupN);
   double s=0.;
   vector<int> queue(nc);
   for(int i=0;i<nh;i++)if(!captiveHuman(i)){
    int k=id[humans[i]];if(marked[k])continue;
    if(fmode>=2&&seen_g[group[humans[i]]]++)return;
    int n=1;queue[0]=k;marked[k]=1;
    int a=0,c=0,h=0;
    for(int j=0;j<n;j++){int q=queue[j];a+=area[q];c+=cnt[q];h+=hc[q];
     for(int r:chain[q])if(!marked[r])queue[n++]=r,marked[r]=1;
    }
    s+=ldexp(1.*a*h,-c);
   }
   string trial=act;
   if(fmode<3)
    for(int i=0;i<nh;i++)trial[i]=use[i]<0?'.':BUILD[dir(humans[i],gates[use[i]])];
   else{
    for(int i=0;i<nh;i++)if(use[i]>=0){
     char b=BUILD[dir(humans[i],gates[use[i]])];
     if((fmode==4||fmode==6)&&act[i]!='.'&&act[i]!=b)return;
     trial[i]=b;
    }
    for(int i=0;i<nh;i++)if(trial[i]!='.'&&trial[i]<'a'){
     int q=nb[humans[i]][find(MOVE,MOVE+4,trial[i])-MOVE];
     if(id[q]>=0&&id[q]<ng&&find(cut.begin(),cut.end(),id[q])!=cut.end())trial[i]='.';
    }
    // Double building is legal in the tester, but remove the redundant action.
    array<bool,V> dup{};
    for(int i=0;i<nh;i++)if(trial[i]>='a'){
     int q=nb[humans[i]][find(BUILD,BUILD+4,trial[i])-BUILD];
     if(dup[q])trial[i]='.';else dup[q]=1;
    }
    s=score(trial);
   }
   if(s-old>best+1e-10)best=s-old,chosen=trial;
  };
  if(fmode>=5){
   vector<uint64_t> compMask(nc);
   for(auto [a,b]:adj)if(a>=ng)compMask[a]|=1ULL<<b;
   vector<uint64_t> wants;
   for(int i=ng;i<nc;i++)if(cnt[i]&&!hc[i]&&compMask[i]&&popcount(compMask[i])<=nh)
    wants.push_back(compMask[i]);
   sort(wants.begin(),wants.end());
   wants.erase(unique(wants.begin(),wants.end()),wants.end());
   vector<uint64_t> all{0};
   unordered_set<uint64_t> seen{0};
   auto aug=[&](auto &&self,int q,int& seen,vector<int>& use)->bool{
    int bs=staff[gates[q]]&~seen;
    while(bs){
     int k=__builtin_ctz(bs);bs&=bs-1;if(seen>>k&1)continue;seen|=1<<k;
     if((fmode!=6||act[k]=='.'||act[k]==BUILD[dir(humans[k],gates[q])])&&(use[k]<0||self(self,use[k],seen,use))){
      use[k]=q;return true;
     }
    }
    return false;
   };
   for(uint64_t w:wants){
    int lim=all.size();
    for(int i=0;i<lim&&all.size()<4096;i++){
     uint64_t x=all[i]|w;
     if(popcount(x)>nh||!seen.insert(x).second)continue;
     vector<int> cut,use(nh,-1);
     bool ok=true;
     for(uint64_t y=x;y;y&=y-1){
      int q=countr_zero(y),v=0;
      if(!aug(aug,q,v,use)){ok=false;break;}
      cut.push_back(q);
     }
     if(!ok)continue;
     all.push_back(x);
     check(cut,use);
    }
   }
   if(best<=0.)return false;
   act=chosen;
   fdone++;fg+=count_if(act.begin(),act.end(),[](char x){return x>='a';});
   return true;
  }
  const int s=2*nc,t=s+1;
  for(int reward:orders){
   vector<char> disabled(ng);
   for(int retry=0;retry<3;retry++){
    MaxFlow<int> f(t+1);
    for(int i=0;i<nc;i++){
     f.add_edge(2*i,2*i+1,(i<ng&&!disabled[i])?35:inf);
     if(cnt[i])f.add_edge(s,2*i+1,cnt[i]*reward);
     f.add_edge(2*i+1,t,hc[i]?inf:area[i]);
    }
    for(auto [a,b]:adj){
     f.add_edge(2*a+1,2*b,inf);
     f.add_edge(2*b+1,2*a,inf);
    }
    f.flow(s,t,inf);
    auto reach=f.min_cut(s);
    vector<int> cut,use(nh,-1);
    for(int i=0;i<ng;i++)if(reach[2*i]&&!reach[2*i+1])cut.push_back(i);
    if(cut.empty())break;
    auto aug=[&](auto &&self,int q,int& seen)->bool{
     int bs=staff[gates[q]]&~seen;
     while(bs){
      int k=__builtin_ctz(bs);bs&=bs-1;if(seen>>k&1)continue;seen|=1<<k;
      if((fmode<4||act[k]=='.'||act[k]==BUILD[dir(humans[k],gates[q])])&&(use[k]<0||self(self,use[k],seen))){use[k]=q;return true;}
     }return false;
    };
    bool ok=true;
    for(int q:cut){int seen=0;if(!aug(aug,q,seen)){disabled[q]=1;ok=false;break;}}
    if(!ok)continue;
    check(cut,use);
    break;
   }
  }
  if(best<=0.)return false;
  act=chosen;
  fdone++;fg+=count_if(act.begin(),act.end(),[](char x){return x>='a';});
  return true;
 }
 string plan(){
  prep();
  if(dogPlan())return act;
  if(fmode<3&&compressedFlow())return act;
  if(latePlan())return act;
  globalset.fill(0);globalmoved.fill(0);
  int hm=humans.size(), free=0, pop=0;
  vector<bool> p1(hm),p2(hm);
  for(int i=0;i<hm;i++)if(!captiveHuman(i))p1[i]=true,free++;
  for(auto p:pets)if(group[p.p]==group[humans[dogClosed&&dogDecoy==0]])pop++;
  if(multi==0||free<2*multiMin||(multi>=2&&turn>=210)||pop<3){
   pocketPlan(p1,target,phase,bestPrev,-1);
   return act;
  }
  if(target<0){
   target=select(p1);phase=turn;bestPrev=0;
   if(target<0)return act;
   for(int p:pockets[target].wall)bestPrev+=!wall[p];
  }
  vector<pair<int,int>> rank;
  int cc=pockets[target].cx;
  for(int i=0;i<hm;i++)if(p1[i]){
   int d=workers[i].bfs.distance(cc);
   if(d<0)d=md(cc,humans[i])+5;
   int k=d;
   if(target2>=0){
    int d2=workers[i].bfs.distance(pockets[target2].cx);
    if(d2<0)d2=md(pockets[target2].cx,humans[i])+5;
    k=d-d2;
   }
   rank.push_back({k,i});
  }
  sort(rank.begin(),rank.end());
  int mainWorkers=(free+1)/2;
  if(multi>=3)mainWorkers=free-multiMin;
  for(int j=mainWorkers;j<free;j++)p1[rank[j].second]=false,p2[rank[j].second]=true;
  pocketPlan(p1,target,phase,bestPrev,target2);
  pocketPlan(p2,target2,phase2,bestPrev2,target);
  return act;
 }
 string repair(string q){
  if(!cohesion)return q;
  int hm=humans.size();
  array<unsigned char,V> testwall=wall;
  int after[12];
  for(int i=0;i<hm;i++){
   after[i]=humans[i];
   if(q[i]=='.')continue;
   if(q[i]<'a')after[i]=nb[humans[i]][find(MOVE,MOVE+4,q[i])-MOVE];
   else testwall[nb[humans[i]][find(BUILD,BUILD+4,q[i])-BUILD]]=1;
  }
  int parent=-1;
  for(int i=0;i<hm;i++)if(!captiveHuman(i)&&(parent<0||currentCt[group[after[i]]]>currentCt[group[after[parent]]]))parent=i;
  if(parent<0)return q;
  int root=group[after[parent]];
  auto reach=[&](){
   static array<int,V> seen{};
   static int stamp=0;
   int bfs[900],n=1;
   bfs[0]=after[parent];seen[after[parent]]=++stamp;
   for(int i=0;i<n;i++)for(int p:nb[bfs[i]])if(p>=0&&!testwall[p]&&seen[p]!=stamp)
    seen[p]=stamp,bfs[n++]=p;
   int nc=0;for(int i=0;i<hm;i++)if(!captiveHuman(i)&&group[after[i]]==root)nc+=seen[after[i]]==stamp;
   return make_pair(nc,n);
  };
  int goal=0;for(int i=0;i<hm;i++)if(!captiveHuman(i)&&group[after[i]]==root)goal++;
  auto pass=reach();
  if(pass.first==goal)return q;
  for(int l=0;l<hm;l++){
   int best=-1;
   auto bmake=pass;
   for(int i=0;i<hm;i++)if(q[i]>='a'){
    int p=nb[humans[i]][find(BUILD,BUILD+4,q[i])-BUILD];
    testwall[p]=0;auto s=reach();testwall[p]=1;
    if(bmake<s)bmake=s,best=i;
   }
   if(best<0){for(int i=0;i<hm;i++)if(q[i]>='a')q[i]='.';break;}
   int p=nb[humans[best]][find(BUILD,BUILD+4,q[best])-BUILD];
   testwall[p]=0;q[best]='.';pass=bmake;
   if(pass.first==goal)break;
  }
  return q;
 }
 bool advance(){
  for(int i=0;i<(int)humans.size();i++) if(act[i]!='.'){
   if(islower(act[i]))wall[nb[humans[i]][find(BUILD,BUILD+4,act[i])-BUILD]]=1;
   else humans[i]=nb[humans[i]][find(MOVE,MOVE+4,act[i])-MOVE];
  }
  string m;
  for(auto &p:pets){
   if(!(cin>>m))return false;
   p.prev=p.p;
   for(char x:m)if(x!='.')p.p=nb[p.p][find(MOVE,MOVE+4,x)-MOVE];
   p.vx=p.vx*.77f+(p.p/S-p.prev/S)*.18f;
   p.vy=p.vy*.77f+(p.p%S-p.prev%S)*.18f;
  }
  return true;
 }
};
}
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 Game g;g.init(argc,argv);
 for(;g.turn<300;g.turn++){
  g.act=g.repair(g.plan());
  if(g.fmode>=3)g.compressedFlow();
  cout<<g.act<<'\n'<<flush;
  if(!g.advance())break;
 }
#ifdef HAB_DIAG
 array<int,6> active{}; g.workers[0].bfs.run(g.humans[0],[&](int p){return !g.wall[p];});
 int a=0;for(int p=0;p<V;p++)a+=g.workers[0].bfs.distance(p)>=0;
 for(auto pp:g.pets)if(g.workers[0].bfs.distance(pp.p)>=0)active[pp.t]++;
 cerr<<"flow "<<g.fsel<<" "<<g.fg<<" "<<g.fdone<<endl;
 cerr<<"diag "<<a;for(int i=1;i<=5;i++)cerr<<" "<<active[i]; cerr<<"\n";
#endif
}
