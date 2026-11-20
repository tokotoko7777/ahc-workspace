#include <bits/stdc++.h>
#include "../../library/sorted-bottleneck-assignment.hpp"
using namespace std;
using i64=long long;
#ifndef AHC053_MATCH
#define AHC053_MATCH 0
#endif
int main(){
  ios::sync_with_stdio(false);cin.tie(nullptr);
  int n,m;i64 l,u;if(!(cin>>n>>m>>l>>u))return 0;
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
