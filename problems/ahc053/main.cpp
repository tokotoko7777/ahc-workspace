#include <bits/stdc++.h>
#include "../../library/sorted-bottleneck-assignment.hpp"
using namespace std;
using i64=long long;
#ifndef AHC053_MATCH
#define AHC053_MATCH 0
#endif
#ifndef AHC053_PAIR
#define AHC053_PAIR 1
#endif
int main(){
  ios::sync_with_stdio(false);cin.tie(nullptr);
  int n,m;i64 l,u;if(!(cin>>n>>m>>l>>u))return 0;
  if(AHC053_PAIR && n==500 && m==50){
    // A sealed shared deck: one anchor tier and three paired correction pools.
    assert(n==500 && m==50);
    constexpr int slots=140, stretches=3;
    array<vector<i64>,stretches> pool;
    array<i64,stretches> center{}, radius{};
    i64 R=(u-l)/2;
    radius[0]=(R+3)/4;
    for(int i=1;i<stretches;++i)radius[i]=(radius[i-1]+9)/10;
    i64 later=0;
    mt19937_64 gen(8582053);
    for(int i=0;i<stretches;++i){
      const auto r=radius[i];center[i]=r+1;later+=2*center[i];
      for(int j=0;j<slots;++j){
        const i64 left=2*r*j/slots,right=2*r*(j+1)/slots;
        pool[i].push_back(1+left+i64(gen()%max(1LL,right-left)));
      }
    }
    vector<i64> anchors;
    const int count=n-slots*stretches;
    for(int i=0;i<count;++i)anchors.push_back(l-later+(u-l)*i/(count-1));
    for(auto x:anchors){assert(1<=x&&x<=u);cout<<x<<' ';}
    for(int i=0;i<stretches;++i)
      for(int j=0;j<slots;++j){
        const auto x=pool[i][j];assert(1<=x&&x<=u);
        cout<<x<<(i+1==stretches&&j+1==slots?'\n':' ');
      }
    cout.flush();
    vector<i64> remain(m);
    for(auto& x:remain)cin>>x;
    vector<i64> targets=remain;
    for(auto& x:targets)x-=later;
    auto picked=sorted_bottleneck_assignment(targets,anchors);
    vector<int> assignment(n);
    for(int j=0;j<m;++j){int k=picked[j];assignment[k]=j+1;remain[j]-=anchors[k];}
    for(int stage=0;stage<stretches;++stage){
      later-=2*center[stage];
      const auto& values=pool[stage];
      vector<int> avail(slots);iota(avail.begin(),avail.end(),0);
      vector<char> done(m);
      for(int step=0;step<m;++step){
        int take=-1,a=-1,b=-1;i64 worst=-1;
        for(int j=0;j<m;++j)if(!done[j]){
          const i64 target=remain[j]-later;
          i64 err=LLONG_MAX;int aa=-1,bb=-1;
          for(int left=0,right=int(avail.size())-1;left<right;){
            const i64 sum=values[avail[left]]+values[avail[right]],d=abs(sum-target);
            if(d<err){err=d;aa=left;bb=right;}
            if(sum<target)++left;else --right;
          }
          if(err>worst){worst=err;take=j;a=aa;b=bb;}
        }
        assert(take>=0&&a>=0&&a<b);
        const int offset=count+stage*slots;
        for(int v:{avail[a],avail[b]}){
          assignment[offset+v]=take+1;remain[take]-=values[v];
        }
        done[take]=true;avail.erase(avail.begin()+b);avail.erase(avail.begin()+a);
      }
    }
    i64 error=0;for(auto x:remain)error+=abs(x);
    cerr<<"error="<<error<<'\n';
    for(int i=0;i<n;++i)cout<<assignment[i]<<(i+1==n?'\n':' ');
    cout.flush();
    return 0;
  }
  constexpr int levels=7;
  assert(n>=levels*m);
  vector<vector<i64>> card(levels);
  const i64 pivot=(l+u)/2;
  array<i64,levels> radius{},center{};
  radius[0]=(u-l+1)/2;
  for(int i=1;i<levels;++i){
    radius[i]=max(1LL,(radius[i-1]+5)/6);
    center[i]=radius[i]+1;
  }
  center[0]=pivot-accumulate(center.begin()+1,center.end(),0LL);
  for(int j=0;j<levels;++j){
    int count=n/levels+(j<n%levels);
    for(int s=0;s<count;++s){
      i64 x=center[j]-radius[j]+2*radius[j]*s/(count-1);
      assert(1<=x&&x<=u);
      card[j].push_back(x);
      cout<<x<<(j==levels-1&&s==count-1?'\n':' ');
    }
  }
  cout.flush();
  vector<i64> remain(m);
  for(auto& x:remain)cin>>x;
  i64 later=accumulate(center.begin()+1,center.end(),0LL);
  vector<int> assignment(n);
  int offset=0;
  for(int stage=0;stage<levels;++stage){
    if(stage>0)later-=center[stage];
    const auto& values=card[stage];
    if(AHC053_MATCH){
      vector<int> sorted(m);iota(sorted.begin(),sorted.end(),0);
      sort(sorted.begin(),sorted.end(),[&](int a,int b){return pair{remain[a],a}<pair{remain[b],b};});
      vector<i64> target;
      for(int i:sorted)target.push_back(remain[i]-later);
      auto picked=sorted_bottleneck_assignment(target,values);
      for(int j=0;j<m;++j){int p=sorted[j],v=picked[j];assignment[offset+v]=p+1;remain[p]-=values[v];}
    }else{
      vector<char> taken(values.size());
      for(int j=0;j<m;++j){
        int best=-1;
        for(int v=0;v<int(values.size());++v) if(!taken[v] &&
          (best<0||abs(remain[j]-later-values[v])<abs(remain[j]-later-values[best])))best=v;
        assert(best>=0);taken[best]=true;assignment[offset+best]=j+1;remain[j]-=values[best];
      }
    }
    offset+=values.size();
  }
  i64 error=0;
  for(auto x:remain)error+=abs(x);
  cerr<<"error="<<error<<'\n';
  for(int i=0;i<n;++i)cout<<assignment[i]<<(i+1==n?'\n':' ');
  cout.flush();
}
