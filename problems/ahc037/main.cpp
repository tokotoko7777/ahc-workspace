#include <bits/stdc++.h>
#include "../../library/agglomerative-pair-merge.hpp"
using namespace std;
#ifndef AHC037_REPARENT
#define AHC037_REPARENT 0
#endif
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
vector<Operation> reparent(
    const vector<Point>& targets,const vector<Operation>& original,bool junctions){
  vector<Point> points=targets;
  points.push_back(Root);
  for(auto [a,b]:original){points.push_back(a);points.push_back(b);}
  sort(points.begin(),points.end(),[](auto a,auto b){return pair{weight(a),a}<pair{weight(b),b};});
  points.erase(unique(points.begin(),points.end()),points.end());
  map<Point,int> idx;
  for(int i=0;i<int(points.size());++i)idx[points[i]]=i;
  vector<char> terminal(points.size());
  for(auto p:targets)terminal[idx.at(p)]=true;
  vector<int> parent(points.size()),children(points.size());
  for(auto [a,b]:original){
    const int x=idx.at(a),y=idx.at(b);
    if(x<y&&weight(points[x])>weight(points[parent[y]]))parent[y]=x;
  }
  for(int i=1;i<int(points.size());++i){
    int best=parent[i];
    for(int j=i-1;j>0&&weight(points[j])>weight(points[best]);--j)
      if(precedes(points[j],points[i])&&(junctions||terminal[j])){best=j;break;}
    parent[i]=best;
    ++children[best];
  }
  vector<char> keep(points.size(),true);
  for(int i=int(points.size())-1;i>0;--i)if(!terminal[i]&&!children[i]){
    keep[i]=false;--children[parent[i]];
  }
  // Contract unary auxiliary nodes. Walking up each original ancestor chain
  // preserves additive monotone distance and reaches an earlier retained ID.
  for(int i=1;i<int(points.size());++i)if(!terminal[i]&&children[i]==1)keep[i]=false;
  for(int i=1;i<int(points.size());++i)if(!keep[parent[i]])
    parent[i]=parent[parent[i]];
  vector<Operation> result;
  for(int i=1;i<int(points.size());++i)if(keep[i])result.push_back({points[parent[i]],points[i]});
  return result;
}

}
#ifndef AHC037_TEST
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
  const auto joined=cost(answer);
  if(AHC037_JOIN&&AHC037_REPARENT){
    auto revised=reparent(points,answer,AHC037_REPARENT==1);
    if(cost(revised)<joined)answer=std::move(revised);
  }
  cout<<answer.size()<<'\n';
  for(auto [a,b]:answer)cout<<a.first<<' '<<a.second<<' '<<b.first<<' '<<b.second<<'\n';
  cerr<<"base="<<base<<" joined="<<joined<<" cost="<<cost(answer)<<'\n';
}
#endif
