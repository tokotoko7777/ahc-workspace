#pragma once
#include <algorithm>
#include <utility>

// Ordered events at locations with signed changes to a carried load.
// The edge immediately after a location uses the load after its change.
// Distances must be symmetric for reversed() and nonnegative for both.
struct LoadAwareRoute {
  int first=-1,last=-1;
  long long net=0,min_prefix=0,max_prefix=0;
  long long distance=0,carry_work=0;

  static LoadAwareRoute at(int location,long long change){
    return {location,location,change,std::min(0LL,change),std::max(0LL,change),0,0};
  }
  bool empty()const{return first==-1;}
  bool feasible(long long entering=0)const{return entering+min_prefix>=0;}
  long long travel_cost(long long base_per_unit,long long entering=0)const{
    return (base_per_unit+entering)*distance+carry_work;
  }
  LoadAwareRoute reversed()const{
    if(empty())return *this;
    return {last,first,net,net-max_prefix,net-min_prefix,distance,net*distance-carry_work};
  }
};

template<class Dist>
LoadAwareRoute join_load_routes(const LoadAwareRoute& a,const LoadAwareRoute& b,Dist dist){
  if(a.empty())return b;
  if(b.empty())return a;
  const long long edge=dist(a.last,b.first);
  return {a.first,b.last,a.net+b.net,std::min(a.min_prefix,a.net+b.min_prefix),
          std::max(a.max_prefix,a.net+b.max_prefix),a.distance+edge+b.distance,
          a.carry_work+b.carry_work+a.net*(edge+b.distance)};
}
