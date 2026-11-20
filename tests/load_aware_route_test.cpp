#include "../library/load-aware-route.hpp"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <random>
#include <vector>

using namespace std;
struct Point{int loc,delta;};
LoadAwareRoute brute(const vector<Point>& points){
  if(points.empty())return {};
  LoadAwareRoute r;
  r.first=points.front().loc;r.last=points.back().loc;
  for(int i=0;i<int(points.size());++i){
    r.net+=points[i].delta;
    r.min_prefix=min(r.net,r.min_prefix);
    r.max_prefix=max(r.net,r.max_prefix);
    if(i+1<int(points.size())){
      int d=abs(points[i+1].loc-points[i].loc);
      r.distance+=d;r.carry_work+=r.net*d;
    }
  }
  return r;
}
void equal(const LoadAwareRoute& a,const LoadAwareRoute& b){
  assert(a.first==b.first&&a.last==b.last&&a.net==b.net&&
    a.min_prefix==b.min_prefix&&a.max_prefix==b.max_prefix&&
    a.distance==b.distance&&a.carry_work==b.carry_work);
}
int main(){
  mt19937 rng(34141);
  auto dist=[](int a,int b){return abs(a-b);};
  auto sum=[&](auto begin,auto end){
    LoadAwareRoute result;
    for(auto it=begin;it!=end;++it)
      result=join_load_routes(result,LoadAwareRoute::at(it->loc,it->delta),dist);
    return result;
  };
  for(int t=0;t<2500;++t){
    vector<Point> path(rng()%24);
    for(auto& q:path)q={int(rng()%100),int(rng()%241)-120};
    size_t left=rng()%(path.size()+1),right=rng()%(path.size()+1);
    if(left>right)swap(left,right);
    auto A=sum(path.begin(),path.begin()+left),B=sum(path.begin()+left,path.begin()+right),
         C=sum(path.begin()+right,path.end());
    auto a=join_load_routes(join_load_routes(A,B,dist),C,dist);
    auto b=join_load_routes(A,join_load_routes(B,C,dist),dist);
    equal(a,b);equal(a,brute(path));
    reverse(path.begin(),path.end());
    equal(a.reversed(),brute(path));
    const long long load=rng()%800;
    assert(a.feasible(load)==(a.min_prefix+load>=0));
    assert(a.travel_cost(100,load)==a.carry_work+(100+load)*a.distance);
    reverse(path.begin(),path.end());
    rotate(path.begin(),path.begin()+left,path.begin()+right);
    equal(brute(path),join_load_routes(join_load_routes(B,A,dist),C,dist));
  }
}
