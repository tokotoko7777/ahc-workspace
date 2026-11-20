#ifndef AHC062_TWO_LANE_HPP
#define AHC062_TWO_LANE_HPP
#include <algorithm>
#include <cassert>
#include <vector>

// Two cell-disjoint paths by choosing one cell per column in each paired
// row, moving in opposite passes. Two border columns on each side provide
// the connectors. Reflections and transpose preserve the exact cover.
inline std::vector<int> two_lane_king_path(int n,const std::vector<int>& w,int flags){
  assert(n>=6 && n%2==0 && int(w.size())==n*n);
  auto id=[&](int y,int x){
    if(flags&1)y=n-1-y;
    if(flags&2)x=n-1-x;
    if(flags&4)std::swap(y,x);
    return y*n+x;
  };
  auto border=[&](bool right){return right?n-2:1;};
  auto outer=[&](bool right){return right?n-1:0;};
  auto win=[&](int y,int x){return w[id(y,x)]<=w[id(y+1,x)]?y:y+1;};
  std::vector<int> low,high;
  low.reserve(n*n/2);high.reserve(n*n/2);
  int first_low_row=win(0,2),last_low_row=-1;
  const int pairs=n/2;
  for(int b=0;b<pairs;++b){
    const bool right=b%2==0;
    const int y=2*b;
    for(int i=2;i<n-2;++i){
      const int x=right?i:n-1-i;
      const int z=win(y,x);
      low.push_back(id(z,x));
      high.push_back(id(2*y+1-z,x));
      last_low_row=z;
    }
    if(b+1==pairs)break;
    const int e=border(right),f=outer(right);
    // Each port can reach either connector entrance because the two
    // available entrance rows differ by one; same on the exit side.
    auto& outer_route=(flags&8)?high:low;
    auto& inner_route=(flags&8)?low:high;
    outer_route.push_back(id(y,e));
    for(int z=y;z<y+4;++z)outer_route.push_back(id(z,f));
    outer_route.push_back(id(y+3,e));
    inner_route.push_back(id(y+1,e));inner_route.push_back(id(y+2,e));
  }
  auto hook=[&](int y,bool right,int row){
    const int other=2*y+1-row,e=border(right),f=outer(right);
    for(int v:{id(row,e),id(row,f),id(other,f),id(other,e)})low.push_back(v);
  };
  // Connect low end to high end via the unclaimed bottom 2x2 border,
  // walk the high lane backwards, and consume the top 2x2 on the way out.
  hook(n-2,(pairs-1)%2==0,last_low_row);
  low.insert(low.end(),high.rbegin(),high.rend());
  hook(0,false,1-first_low_row);
  assert(int(low.size())==n*n);
  return low;
}
#endif
