#pragma GCC optimize("O3")
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

namespace transit {
struct DogEdgeTransit {
 static constexpr int S=30,V=S*S;
 static constexpr uint16_t INF=65535;
 static constexpr char MOVES[]="UDLR", BUILDS[]="udlr";
 static constexpr char WAIT='.';
 static int md(int p,int q){return std::abs(p/S-q/S)+std::abs(p%S-q%S);}
 static std::array<int,4> nbs(int p){return {p>=S?p-S:V,p+S<V?p+S:V,p%S>0?p-1:V,p%S+1<S?p+1:V};}
 static int dir(int p,int q){auto ns=nbs(p);auto at=std::find(ns.begin(),ns.end(),q);assert(at!=ns.end()); return int(at-ns.begin());}
 using Grid=std::array<unsigned char,V>;
 using Dist=std::array<uint16_t,V>;
 static Dist bfs(const Grid& wall,int s){
  Dist d;d.fill(INF);d[s]=0;
  int q[V],l=0,r=1;q[0]=s;
  for(;l<r;l++)for(int u:nbs(q[l]))if(u<V&&!wall[u]&&d[u]==INF)d[u]=d[q[l]]+1,q[r++]=u;
  return d;
 }
 static int move(int p,const Dist& d,const Grid& wall){
  for(int q:nbs(p))if(q<V&&!wall[q]&&d[q]<d[p])return q;
  return V;
 }
 struct Shape{
  std::array<int,6> wall{},lane{};
  int a=-1,b=-1;
  bool in(int p)const{return std::find(lane.begin()+1,lane.end()-1,p)!=lane.end()-1;}
  bool core(int p)const{return lane[2]==p||lane[3]==p;}
 };
 struct PlanState{
  Shape z;
  int guard=-1, elapsed=0, max_turns=75, wall_turns=0, guard_turns=0, all_staffed_turns=0;
  int builds=0, walks=0, waits=0, captured_dogs=0,captured_other=0;
  bool closed=false, finished=false;
  bool choose(const std::vector<int>& hp,const std::vector<int>& pp,const std::vector<int>& types,
              const Grid& wall){
   double best=-1e30;
   Shape keep;
   for(int side=0;side<4;side++)for(int c=2;c<=22;c++)for(bool r:{false,true}){
    auto cv=[side](int d,int t){return side==0?d*S+t:side==1?(S-1-d)*S+t:side==2?t*S+d:t*S+S-1-d;};
    Shape cand;for(int j=0;j<6;j++)cand.wall[j]=cv(1,c+j),cand.lane[j]=cv(0,c+j);
    cand.a=cv(0,c-1);cand.b=cv(0,c+6);
    if(r){std::swap(cand.a,cand.b);std::reverse(cand.lane.begin(),cand.lane.end());}
    if(wall[cand.a]||wall[cand.b]||std::any_of(cand.lane.begin(),cand.lane.end(),[&](int p){return wall[p];}))continue;
    auto onwall=[&](int p){return std::find(cand.wall.begin(),cand.wall.end(),p)!=cand.wall.end();};
    if(std::any_of(pp.begin(),pp.end(),onwall)||std::any_of(hp.begin(),hp.end(),onwall))continue;
    int build=0, far=0, ast=V;
    for(int x:cand.wall)if(!wall[x]){int d=V;for(int h:hp)d=std::min(d,md(h,x));build+=d;}
    for(int h:hp)far+=md(h,cand.b),ast=std::min(ast,md(h,cand.a));
    double dogs=0;
    for(int j=0;j<int(pp.size());j++)if(types[j]==4){
     int p=pp[j];double dp=md(p,cand.a);
     double side=md(p,cand.a)+2<md(p,cand.b)?1.:std::find(cand.lane.begin(),cand.lane.end(),p)!=cand.lane.end()?.6:.15;
     dogs+=side/(6.+dp);
    }
    double v=dogs*280.-build*.14-double(far)/hp.size()*.30-ast*.22;
    if(v>best)best=v,keep=cand;
   }
   z=keep;
   return z.a>=0;
  }
  std::string plan(const std::vector<int>& hs,const std::vector<int>& ps,
                   const std::vector<int>& types,const Grid& wall){
   if(finished||elapsed==max_turns||std::find(types.begin(),types.end(),4)==types.end()){
    finished=true;return {};
   }
   if(z.a<0&&!choose(hs,ps,types,wall)){finished=true;return {};}
   ++elapsed;
   const int m=hs.size(),a=z.a,b=z.b;
   std::string act(m,WAIT);
   Grid forbidden{},will{},move_block=wall;
   for(int p:ps){
    forbidden[p]=1;
    for(int q:nbs(p))if(q<V)forbidden[q]=1;
   }
   for(int p:hs)forbidden[p]=1;
   if(std::all_of(z.wall.begin(),z.wall.end(),[&](int p){return wall[p];})){
    Dist da=bfs(wall,a),db=bfs(wall,b);
    if(guard<0) {
     guard=0;
     for(int j=1;j<m;j++)if(int(da[hs[j]])-int(db[hs[j]])<int(da[hs[guard]])-int(db[hs[guard]]))guard=j;
    }
    int near=hs[guard]==a, far=0, far_first=-1;
    for(int j=0;j<m;j++)if(j!=guard&&hs[j]==b){
     if(far_first<0)far_first=j;
     ++far;
    }
    guard_turns+=near&&far;
    all_staffed_turns+=near&&far==m-1;
    if(near&&far&&!forbidden[z.lane[0]]&&!forbidden[z.lane[5]]&&
       std::none_of(hs.begin(),hs.end(),[&](int p){return z.in(p);})){
      int dogs=0,other=0;
      for(int j=0;j<int(ps.size());j++)if(z.in(ps[j])){
       if(types[j]==4)++dogs;
       else ++other;
      }
      if(dogs>0){
       act[guard]=BUILDS[dir(a,z.lane[0])];act[far_first]=BUILDS[dir(b,z.lane[5])];
       captured_dogs=dogs;captured_other=other;closed=finished=true;
      }
    }
    if(!closed)for(int j=0;j<m;j++){
     const auto& d=j==guard?da:db;
     int q=move(hs[j],d,wall);
     if(q<V)act[j]=MOVES[dir(hs[j],q)];
    }
   }else{
    ++wall_turns;
    for(int p:z.wall)move_block[p]=true;
    std::vector<Dist> d; for(int h:hs)d.push_back(bfs(move_block,h));
    std::vector<std::tuple<int,int,int,int>> cand;
    for(int p:z.wall)if(!wall[p])for(int q:nbs(p))if(q<V&&!move_block[q]){
      const int shade=z.in(q)?2:0;
      for(int j=0;j<m;j++)if(d[j][q]!=INF&&(!forbidden[p]||d[j][q]!=0))
       cand.emplace_back(int(d[j][q])*8+shade+(forbidden[p]?4:0),j,p,q);
    }
    std::sort(cand.begin(),cand.end());
    Grid assigned{};
    std::vector<int> w(m,-1),s(m,b);
    for(auto [cost,j,p,q]:cand)if(w[j]<0&&!assigned[p]){w[j]=p;s[j]=q;assigned[p]=1;}
    for(int j=0;j<m;j++)if(w[j]>=0&&hs[j]==s[j]){act[j]=BUILDS[dir(hs[j],w[j])];will[w[j]]=1;}
    for(int j=0;j<m;j++)if(act[j]==WAIT){
     auto d2=bfs(move_block,s[j]);int q=move(hs[j],d2,move_block);
     if(q<V&&!will[q])act[j]=MOVES[dir(hs[j],q)];
    }
   }
   for(char c:act){
    if(c==WAIT)++waits;
    else if(c>='a')++builds;
    else ++walks;
   }
   return act;
  }
 };
};
}

using namespace std;
using D=transit::DogEdgeTransit;
int main(int argc,char**argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,m,r,c,t;
 if(!(cin>>n))return 0; vector<int> ps(n),ts(n);
 for(int j=0;j<n;j++){cin>>r>>c>>t;ps[j]=(r-1)*30+c-1;ts[j]=t;}
 cin>>m;vector<int> hs(m);
 for(int j=0;j<m;j++){cin>>r>>c;hs[j]=(r-1)*30+c-1;}
 D::Grid wall{};D::PlanState z;
 if(argc>1)z.max_turns=atoi(argv[1]);
 for(int turn=0;turn<300;turn++){
  string act=z.plan(hs,ps,ts,wall);
  if(act.empty())act=string(m,'.');
  cout<<act<<'\n'<<flush;
  for(int j=0;j<m;j++)if(act[j]!='.'){
   auto ns=D::nbs(hs[j]);
   if(act[j]>='a')wall[ns[string_view(D::BUILDS).find(act[j])]]=1;
   else hs[j]=ns[string_view(D::MOVES).find(act[j])];
  }
  for(int j=0;j<n;j++){
   string s;if(!(cin>>s))return 0;
   for(char letter:s)if(letter!='.')ps[j]=D::nbs(ps[j])[string_view(D::MOVES).find(letter)];
  }
 }
 auto largest=D::bfs(wall,hs[0]);
 int residual=0,res_dogs=0;
 for(int j=0;j<n;j++)if(largest[ps[j]]!=D::INF){++residual;res_dogs+=ts[j]==4;}
 if(getenv("TRANSIT_DIAG"))cerr<<"TRANSIT "<<z.elapsed<<" "<<z.wall_turns<<" "<<z.guard_turns<<" "<<z.all_staffed_turns<<" "<<z.builds<<" "<<z.walks<<" "<<z.waits<<" "<<z.captured_dogs<<" "<<z.captured_other<<" "<<residual<<" "<<res_dogs<<"\n";
}
