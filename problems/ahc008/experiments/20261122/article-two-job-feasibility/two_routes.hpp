#pragma once
#include "last_capture.hpp"
namespace tworoutes {
using namespace lastcap;
struct Path {int a=-1,b=-1,sa=-1,sb=-1,t=0,firstTravel=0,secondTravel=0;};
struct Result {int time=INF;std::vector<Path> work;};
inline Result choose(const std::vector<std::vector<Path>>& choices,int k){
 assert(0<k&&k<=10);
 const int n=1<<k;
 int m=choices.size();
 std::vector<int> dp(n,INF),next(n);
 std::vector<std::vector<std::pair<short,short>>> prev(m,std::vector<std::pair<short,short>>(n,{-1,-1}));
 dp[0]=0;
 for(int i=0;i<m;i++){
  std::fill(next.begin(),next.end(),INF);
  for(int s=0;s<n;s++)if(dp[s]<INF)for(int j=0;j<(int)choices[i].size();j++){
   const auto &p=choices[i][j];
   int b=(p.a>=0?1<<p.a:0)|(p.b>=0?1<<p.b:0);
   if(s&b)continue;
   int value=std::max(p.t,dp[s]);
   if(value<next[s|b])next[s|b]=value,prev[i][s|b]={short(s),short(j)};
  }
  dp.swap(next);
 }
 Result res;res.time=dp[n-1];
 if(res.time>=INF)return res;
 res.work.resize(m);
 for(int i=m-1,s=n-1;i>=0;i--){
  auto [p,j]=prev[i][s];assert(p>=0&&j>=0);res.work[i]=choices[i][j];s=p;
 }
 return res;
}
inline Result solve(const Board& exterior,const std::vector<int>& cut,const std::vector<int>& start){
 const int m=start.size(),k=cut.size();
 if(k>2*m||k>10||k==0)return {};
 std::vector<std::vector<int>> stance(k);
 std::array<int,V> index;index.fill(-1);
 std::vector<BFS> nav;
 nav.reserve(4*k);
 for(int j=0;j<k;j++)for(int q:neighbors(cut[j]))if(q>=0&&exterior[q]){
  stance[j].push_back(q);
  if(index[q]<0){index[q]=nav.size();nav.emplace_back();nav.back().run(q,exterior);}
 }
 std::vector<std::vector<Path>> options(m);
 for(int i=0;i<m;i++){
  auto &op=options[i];op.emplace_back();
  BFS d;d.run(start[i],exterior);
  for(int j=0;j<k;j++){
   Path single;single.t=INF;
   for(int a:stance[j])if(d.d[a]>=0&&d.d[a]+1<single.t)
    single={j,-1,a,-1,d.d[a]+1,d.d[a],0};
   if(single.t<INF)op.push_back(single);
   for(int l=j+1;l<k;l++){
    Path pair;pair.t=INF;
    for(int b=0;b<2;b++){
     int one=b?l:j,two=b?j:l;
     for(int a:stance[one])if(d.d[a]>=0)for(int e:stance[two]){
      int mid=nav[index[a]].d[e];
      if(mid>=0&&d.d[a]+mid+2<pair.t)pair={one,two,a,e,d.d[a]+mid+2,d.d[a],mid};
     }
    }
    if(pair.t<INF)op.push_back(pair);
   }
  }
 }
 return choose(options,k);
}
}
