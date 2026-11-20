#include <bits/stdc++.h>
#include "../../library/implicit-multi-source-dijkstra.hpp"
using namespace std;
#ifndef AHC018_PROBES
#define AHC018_PROBES 0
#endif

namespace {
using Clock=chrono::steady_clock;
constexpr int Ys[4]={-1,1,0,0},Xs[4]={0,0,-1,1};
struct Solver {
  Clock::time_point stop=Clock::now()+chrono::milliseconds(4470);
  int n,w,k,c;
  vector<int> sources,houses,hardness;
  vector<char> hole,wet;
  vector<int> roots,samples;
  bool ended=false;

  void read() {
    cin>>n>>w>>k>>c;
    sources.resize(w);
    houses.resize(k);
    for(auto* v:{&sources,&houses})for(int& p:*v){
      int y,x;cin>>y>>x;p=y*n+x;
    }
    hardness.assign(n*n,0);
    hole.assign(n*n,0);
    wet.assign(n*n,0);
  }

  template<class F>
  void neighbors(int p,F visit) const {
    for(int d=0;d<4;++d){
      int y=p/n+Ys[d],x=p%n+Xs[d];
      if(0<=y&&y<n&&0<=x&&x<n)visit(y*n+x);
    }
  }

  int local_prediction(int p,int fallback) const {
    int count=0,total=0;
    neighbors(p,[&](int q){if(hardness[q])total+=hardness[q],++count;});
    return count?total/count:fallback;
  }

  void drill(int p,int prediction) {
    if(hole[p]||ended)return;
    prediction=clamp(local_prediction(p,prediction),10,5000);
    int first=clamp(int(prediction*0.70)-c/4,10,4500);
    int next=clamp(int(sqrt(2.0*(c+2)*(max(16,prediction/4)))),9,1200);
    int total=0;
    while(!ended){
      const int power=Clock::now()>=stop?5000:total==0?first:next;
      cout<<p/n<<' '<<p%n<<' '<<power<<endl;
      int reply=-1;
      if(!(cin>>reply) || reply==-1){ended=true;return;}
      total+=power;
      if(reply>0){
        hole[p]=1;
        hardness[p]=clamp(total-power/2,10,5000);
        if(reply==2)ended=true;
        return;
      }
      if(total>prediction)
        next=min(1600,(next*6+4)/5);
    }
  }

  void water_from(int root) {
    if(wet[root]||!hole[root])return;
    wet[root]=1;
    roots.push_back(root);
    for(int i=int(roots.size())-1;i<int(roots.size());++i){
      neighbors(roots[i],[&](int p){
        if(!wet[p]&&hole[p]){
          wet[p]=1;
          roots.push_back(p);
        }
      });
    }
  }

  static constexpr int step=5;
  vector<int> field() const {
    const int side=(n-1)/step+1;
    vector<double> coarse(side*side);
    for(int y=0;y<side;++y)for(int x=0;x<side;++x){
      double logsum=0.004*log(490.0), weight=0.004;
      for(int p:samples){
        int dy=y*step-p/n,dx=x*step-p%n;
        double q=1.0/(49+dy*dy+dx*dx);
        logsum+=q*log(double(hardness[p]));
        weight+=q;
      }
      coarse[y*side+x]=exp(logsum/weight);
    }
    vector<int> model(n*n);
    for(int y=0;y<n;++y)for(int x=0;x<n;++x){
      const int a=y/step,b=x/step, u=min(a+1,side-1),v=min(b+1,side-1);
      double fy=(y%step)/double(step),fx=(x%step)/double(step);
      auto row=[&](int r){return (1-fx)*coarse[r*side+b]+fx*coarse[r*side+v];};
      model[y*n+x]=max(10,int((1-fy)*row(a)+fy*row(u)));
    }
    return model;
  }

  void probe() {
    int y0=n-1,y1=0,x0=n-1,x1=0;
    for(const auto* v:{&sources,&houses})for(int p:*v){
      y0=min(y0,p/n);y1=max(y1,p/n);
      x0=min(x0,p%n);x1=max(x1,p%n);
    }
    y0=max(0,y0-19);x0=max(0,x0-19);
    y1=min(n-1,y1+19);x1=min(n-1,x1+19);
    constexpr int spacing=32;
    for(int y=(y0+y1)%spacing/2; y<=y1;y+=spacing)
      for(int x=(x0+x1)%spacing/2;x<=x1;x+=spacing){
        if(Clock::now()>=stop||ended)return;
        if(y<y0||x<x0)continue;
        const int p=y*n+x;
        if(hole[p])continue;
        drill(p,480);
        if(!ended)samples.push_back(p);
      }
    for(int p:sources)water_from(p);
    // A probe may connect to an already-wet source via previous probe holes.
    for(int p:samples)if(hole[p]&&!wet[p]){
      bool near=false;
      neighbors(p,[&](int q){near|=wet[q];});
      if(near)water_from(p);
    }
  }

  void solve() {
    for(int p:sources){
      drill(p,480);
      if(ended)return;
      samples.push_back(p);
      water_from(p);
    }
    for(int p:houses){
      drill(p,480);
      if(ended)return;
      samples.push_back(p);
    }
    if(AHC018_PROBES)probe();
    if(ended)return;
    vector<char> is_house(n*n);
    for(int p:houses)is_house[p]=1;
    while(!ended){
      const bool estimated=AHC018_PROBES && Clock::now()<stop;
      vector<int> model=estimated?field():vector<int>{};
      auto path=implicit_multi_source_dijkstra<int>(
          n*n,roots,INT_MAX/4,[&](int v,auto add){
            neighbors(v,[&](int q){
              const int cost=hole[q]?0:estimated?model[q]+2*c:1;
              add(q,cost);
            });
          },[&](int p){return is_house[p]&&!wet[p];});
      if(!path)return;
      int observed=0;
      for(int p:path->nodes){
        const bool new_hole=!hole[p];
        drill(p,estimated?model[p]:480);
        if(ended)return;
        if(estimated&&new_hole&&++observed%9==0)samples.push_back(p);
        water_from(p);
      }
    }
  }
};
} // namespace

int main(){
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  Solver solver{};
  solver.read();
  solver.solve();
}
