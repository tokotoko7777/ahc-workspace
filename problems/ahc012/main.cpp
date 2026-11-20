#include <bits/stdc++.h>
#include "../../library/capped-size-histogram.hpp"
#if defined(AHC012_TARGET_CUT_DIAGNOSTIC) || defined(AHC012_TARGET_CUT)
#include "../../library/capped-partition-sweep.hpp"
#endif
using namespace std;

#ifndef AHC012_DIAGONAL
#define AHC012_DIAGONAL 0
#endif
#ifndef AHC012_MS
#define AHC012_MS 2710
#endif

namespace {
using Clock = chrono::steady_clock;
using Cuts = array<vector<int>,3>;

struct Input {
  int n,k,target=0;
  vector<int> demand=vector<int>(11);
  vector<array<int,3>> pos;
  array<vector<pair<int,int>>,3> order;
  void read() {
    cin>>n>>k;
    for(int d=1;d<=10;++d){cin>>demand[d];target+=demand[d];}
    pos.resize(n);
    for(int i=0;i<n;++i){
      auto& a=pos[i];
      cin>>a[0]>>a[1];
      a[2]=a[0]+a[1];
      for(int t=0;t<3;++t)order[t].emplace_back(a[t],i);
    }
    for(auto& a:order)sort(a.begin(),a.end());
  }
};

struct State {
  const Input& in;
  Cuts cuts;
  array<int,3> stride;
  vector<int> key, count;
  CappedSizeHistogram hist;

  State(const Input& input,Cuts c)
      : in(input),cuts(std::move(c)),
        stride{1,int(cuts[0].size())+1,int((cuts[0].size()+1)*(cuts[1].size()+1))},
        key(in.n), count(stride[2]*(cuts[2].size()+1)),hist(in.demand) {
    for(int i=0;i<in.n;++i) {
      for(int d=0;d<3;++d){
        int bin=lower_bound(cuts[d].begin(),cuts[d].end(),in.pos[i][d])-cuts[d].begin();
        key[i]+=stride[d]*bin;
      }
      ++count[key[i]];
    }
    for(int v:count)hist.replace_size(0,v);
  }

  int value() const {return hist.value();}

  void move_cut(int d,int i,int now) {
    int old=cuts[d][i];
    assert(old!=now);
    const auto& a=in.order[d];
    auto l=upper_bound(a.begin(),a.end(),pair{min(old,now),in.n});
    auto r=upper_bound(l,a.end(),pair{max(old,now),in.n});
    int shift=now>old?-stride[d]:stride[d];
    for(;l!=r;++l){
      int& j=key[l->second];
      hist.replace_size(count[j],count[j]-1);
      --count[j];
      j+=shift;
      hist.replace_size(count[j],count[j]+1);
      ++count[j];
    }
    cuts[d][i]=now;
  }
};

constexpr int cap[3]={10000,10000,14142};

Cuts initial_cuts(int n,int diag,mt19937& rng){
  Cuts result;
  for(int d=0;d<3;++d){
    const int num=d<2?n:diag;
    const int range=d<2?10000:10900;
    const int perturb=range/max(1,num)/6;
    for(int j=1;j<=num;++j)
      result[d].push_back(-range + 2*range*j/(num+1)
                         + int(rng()%(2*perturb+1))-perturb);
  }
  return result;
}

struct Solution {Cuts cuts;int value=-1;};

Solution solve(const Input& in,Clock::time_point stop){
  const auto start=Clock::now();
  Solution best;
  mt19937 rng(120432);
  constexpr int stages=6;
  constexpr double scale[stages]={0.94,1.02,1.10,0.88,0.99,1.16};
  constexpr int diag_count[stages]={0,0,0,6,9,12};
  for(int stage=0;stage<stages;++stage){
    const auto begin=Clock::now();
    if(begin>=stop)break;
    const auto deadline=start+(stop-start)*(stage+1)/stages;
    const int diag=AHC012_DIAGONAL?diag_count[stage]:0;
    const int side=clamp(int(lround(sqrt(4.0*in.target / numbers::pi)*scale[stage]-0.65*diag)),
                         3,(in.k-diag)/2);
    State s(in,initial_cuts(side,diag,rng));
    auto remember=[&] {
      if(s.value()>best.value) best={s.cuts,s.value()};
    };
    remember();
    const int axes=diag?3:2;
    double temp=2.5;
    int jump=500;
    for(int it=0;;++it){
      if((it&127)==0){
        auto now=Clock::now();
        if(now>=deadline||best.value==in.target)break;
        double fraction=double((now-begin).count())/(deadline-begin).count();
        temp=2.5*pow(0.075,max(0.0,fraction));
        jump=120+int(580*max(0.0,1.0-fraction));
      }
      int d=rng()%axes, i=rng()%s.cuts[d].size();
      int old=s.cuts[d][i];
      const int low=i>0?s.cuts[d][i-1]+1:-cap[d];
      const int high=i+1<int(s.cuts[d].size())?s.cuts[d][i+1]-1:cap[d];
      int range=(rng()%6==0)?(d<2?1600:2200):jump;
      int now=clamp(old+int(rng()%(2*range+1))-range,low,high);
      if(now==old)continue;
      int before=s.value();
      s.move_cut(d,i,now);
      if(s.value()>=before || generate_canonical<double,32>(rng)<exp((s.value()-before)/temp))
        remember();
      else
        s.move_cut(d,i,old);
    }
    if(best.value==in.target)break;
  }
  return best;
}
#if defined(AHC012_TARGET_CUT_DIAGNOSTIC) || defined(AHC012_TARGET_CUT)
#include "target-cut.hpp"
#endif
} // namespace

#ifndef AHC012_NO_MAIN
int main(){
  auto start=Clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  Input in{};
  in.read();
  auto solution=solve(in,start+chrono::milliseconds(AHC012_MS));
  const auto& c=solution.cuts;
  vector<array<int,4>> lines;
  constexpr int B=1000000;
  // y (or x) lies strictly between -B and B in the cake. Thus the cut
  // lands between consecutive integral projections everywhere relevant.
  for(int t:c[0])lines.push_back({t,-B,t+1,B});
  for(int t:c[1])lines.push_back({-B,t,B,t+1});
  for(int t:c[2])lines.push_back({t+B,-B,t+1-B,B});
#if defined(AHC012_TARGET_CUT_DIAGNOSTIC) || defined(AHC012_TARGET_CUT)
  auto compared=target_cut(in,solution,start+chrono::milliseconds(2880));
#ifdef AHC012_TARGET_CUT_DIAGNOSTIC
  cerr<<"frozen="<<solution.value<<" axis="<<compared.axis_value
      <<" all="<<compared.all_value<<" normals="<<compared.normals<<'\n';
  auto log_extra=[&](optional<TargetCut> cut){
    if(!cut){cerr<<"-\n";return;}
    auto l=cut->endpoints();
    cerr<<l[0]<<' '<<l[1]<<' '<<l[2]<<' '<<l[3]<<'\n';
  };
  log_extra(compared.axis);
  log_extra(compared.all);
#endif
  if(compared.all)lines.push_back(compared.all->endpoints());
#endif
  cout<<lines.size()<<'\n';
  for(auto [a,b,p,q]:lines)cout<<a<<' '<<b<<' '<<p<<' '<<q<<'\n';
#if defined(AHC012_STATS) && !defined(AHC012_TARGET_CUT_DIAGNOSTIC)
#ifdef AHC012_TARGET_CUT
  cerr<<"served="<<compared.all_value<<" diagonal="<<c[2].size()<<'\n';
#else
  cerr<<"served="<<solution.value<<" diagonal="<<c[2].size()<<'\n';
#endif
#endif
}
#endif
