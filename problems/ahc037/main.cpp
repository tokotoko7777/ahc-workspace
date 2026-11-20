#include <bits/stdc++.h>
#include "../../library/agglomerative-pair-merge.hpp"
using namespace std;
#ifndef AHC037_JOIN
#define AHC037_JOIN 1
#endif
namespace {
using Point=pair<int,int>;
using Operation=pair<Point,Point>;
constexpr Point Root{0,0};
long long weight(Point p){return static_cast<long long>(p.first)+p.second;}
Point meet(Point a,Point b){return {min(a.first,b.first),min(a.second,b.second)};}
bool precedes(Point a,Point b){return a.first<=b.first&&a.second<=b.second;}
long long cost(const vector<Operation>& ops){
  long long sum=0;
  for(auto [a,b]:ops){assert(precedes(a,b));sum+=weight(b)-weight(a);}
  return sum;
}
vector<Operation> terminal_only(vector<Point> points){
  sort(points.begin(),points.end(),[](Point a,Point b){
    return pair{weight(a),a}<pair{weight(b),b};
  });
  vector<Operation> result;
  for(int i=0;i<int(points.size());++i){
    Point target=points[i],p=Root;
    if(target==Root)continue;
    for(int j=0;j<i;++j)if(precedes(points[j],target)&&weight(points[j])>weight(p))
      p=points[j];
    result.emplace_back(p,target);
  }
  return result;
}
vector<Operation> join(const vector<Point>& points){
  auto tree=agglomerative_pair_merge(points,
    [](Point a,Point b){
      auto p=meet(a,b);
      // A larger shared bottom-left point frees both open paths earlier.
      // Among ties, prefer a direct dependency over a synthetic node.
      return pair{weight(p),p==a||p==b};
    },meet);
  vector<Operation> result;
  if(tree.values.empty())return result;
  vector<int> pending{int(tree.values.size())-1};
  if(tree.values.back()!=Root)result.push_back({Root,tree.values.back()});
  while(!pending.empty()){
    int p=pending.back();pending.pop_back();
    auto [a,b]=tree.children[p];
    if(a<0)continue;
    for(int v:{a,b}){
      if(tree.values[v]!=tree.values[p])result.push_back({tree.values[p],tree.values[v]});
      pending.push_back(v);
    }
  }
  return result;
}
}
int main(){
  ios::sync_with_stdio(false);cin.tie(nullptr);
  int n;if(!(cin>>n))return 0;
  vector<Point> points(n);
  for(auto& [x,y]:points)cin>>x>>y;
  auto answer=terminal_only(points);
  const auto base=cost(answer);
  if(AHC037_JOIN){
    auto variant=join(points);
    if(cost(variant)<base)answer=std::move(variant);
  }
  cout<<answer.size()<<'\n';
  for(auto [a,b]:answer)cout<<a.first<<' '<<a.second<<' '<<b.first<<' '<<b.second<<'\n';
  cerr<<"base="<<base<<" cost="<<cost(answer)<<'\n';
}
