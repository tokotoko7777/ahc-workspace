#include <bits/stdc++.h>
using namespace std;
#ifndef AHC040_MIXED
#define AHC040_MIXED 0
#endif
namespace {
using Clock=chrono::steady_clock;
using Dim=pair<int,int>;
struct Move{int rect,rot;char dir;int anchor;};
struct Pose{int x0,y0,x1,y1;};
bool overlap(int a,int b,int c,int d){return max(a,c)<min(b,d);}
Pose slide(Dim original,const vector<Pose>& used,Move m){
  auto [width,height]=original;
  if(m.rot)swap(width,height);
  int x=0,y=0;
  if(m.dir=='U'){
    if(m.anchor>=0)x=used[m.anchor].x1;
    for(auto q:used)if(overlap(x,x+width,q.x0,q.x1))y=max(y,q.y1);
  }else{
    if(m.anchor>=0)y=used[m.anchor].y1;
    for(auto q:used)if(overlap(y,y+height,q.y0,q.y1))x=max(x,q.x1);
  }
  return {x,y,x+width,y+height};
}
vector<Move> shelf(const vector<Dim>& dims,int turn){
  mt19937 rng(401001+turn);
  double area=0;
  for(auto [w,h]:dims)area+=double(w)*h;
  const double factor=.77+.035*(turn%17);
  const int bound=int(factor*sqrt(area));
  int used=0,prev=-1;
  vector<Move> plan;
  for(int i=0;i<int(dims.size());++i){
    auto [a,b]=dims[i];
    // Prefer a horizontal long side, sometimes try the other orientation.
    const int rotate=int(a<b)^(int(rng()%100)<turn%6*7);
    const int w=rotate?b:a;
    if(used+w>bound){used=0;prev=-1;}
    const char direction=(turn%4==1)?'L':'U';
    plan.push_back({i,rotate^(direction=='L'),direction,prev});
    used+=w;prev=i;
  }
  return plan;
}
vector<Dim> dimensions(const vector<Dim>& observed,int sigma,int turn){
  mt19937 rng(402040+turn);
  vector<Dim> adjusted=observed;
  int amplitude=turn%5==0?0:sigma*(2+turn%4)/4;
  for(auto& [w,h]:adjusted){
    auto one=[&](int v){
      int perturb=amplitude==0?0:int(rng()%(2*amplitude+1))-amplitude;
      return clamp(v+perturb,10000,100000);
    };
    w=one(w);h=one(h);
  }
  return adjusted;
}
vector<Move> mixed(const vector<Dim>& dims,int turn){
  mt19937 rng(404040+turn);
  vector<Pose> placed;
  vector<Move> plan;
  int w=0,h=0;
  const double ratio=0.82+0.03*(turn%14);
  const double compact=.13+.04*(turn%6);
  const double balance=.25+.07*(turn%4);
  const double compact_origin=.015*(turn%5);
  for(int i=0;i<int(dims.size());++i){
    double best=1e100;
    Pose best_pose{};
    Move selected{};
    vector<int> trial{-1};
    if(i<=28){
      for(int j=0;j<i;++j)trial.push_back(j);
    }else{
      const int offset=rng()%i;
      for(int k=0;k<24;++k)trial.push_back((k*i/24+offset)%i);
      trial.push_back(i-1);
    }
    for(int rot=0;rot<2;++rot)for(char side:{'U','L'})for(int anchor:trial){
      Move t{i,rot,side,anchor};
      Pose p=slide(dims[i],placed,t);
      const int W=max(w,p.x1),H=max(h,p.y1);
      const double score=W+H+balance*abs(W-ratio*H)+compact*(double(W)*H)/(W+H)
                     +compact_origin*(p.x0+p.y0) + double(rng()%1500)*(turn%3)/12.;
      if(score<best){best=score;selected=t;best_pose=p;}
    }
    w=max(w,best_pose.x1);h=max(h,best_pose.y1);
    plan.push_back(selected);placed.push_back(best_pose);
  }
  return plan;
}
}
#ifndef AHC040_TEST
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(2300);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  int n,T,sigma;if(!(cin>>n>>T>>sigma))return 0;
  vector<Dim> obs(n);
  for(auto& [a,b]:obs)cin>>a>>b;
  for(int t=0;t<T;++t){
    const auto estimate=dimensions(obs,sigma,t);
    const bool choose_mixed=AHC040_MIXED&&t%3!=0&&Clock::now()<deadline;
    auto plan=choose_mixed?mixed(estimate,t):shelf(estimate,t);
    cout<<plan.size()<<'\n';
    for(auto m:plan)cout<<m.rect<<' '<<m.rot<<' '<<m.dir<<' '<<m.anchor<<'\n';
    cout.flush();
    int w,h;if(!(cin>>w>>h))return 0;
  }
}
#endif
