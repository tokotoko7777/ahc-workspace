#pragma once
#include <cassert>
#include <utility>
#include <vector>

// Transform value(i,j) to max_{u,v} value(u,v) - penalty * L1((x_i,y_j),(x_u,y_v)).
// Axes are sorted, need not be uniformly spaced. Values and source IDs are
// row-major with second coordinate contiguous. Ties are kept as encountered.
template<class Score,class Coordinate>
void manhattan_max_transform(const std::vector<Coordinate>& xs,const std::vector<Coordinate>& ys,
                             Score penalty,std::vector<Score>& value,std::vector<int>& source) {
  const int n=xs.size(),m=ys.size();
  assert(penalty>=0 && value.size()==xs.size()*ys.size() && source.size()==value.size());
  auto relax=[&](int to,int from,Coordinate delta){
    if(source[from]<0)return;
    const Score proposed=value[from]-penalty*delta;
    if(source[to]<0 || proposed>value[to]){
      value[to]=proposed;source[to]=source[from];
    }
  };
  for(int x=0;x<n;++x){
    for(int y=1;y<m;++y)relax(x*m+y,x*m+y-1,ys[y]-ys[y-1]);
    for(int y=m-2;y>=0;--y)relax(x*m+y,x*m+y+1,ys[y+1]-ys[y]);
  }
  for(int x=1;x<n;++x)for(int y=0;y<m;++y)
    relax(x*m+y,(x-1)*m+y,xs[x]-xs[x-1]);
  for(int x=n-2;x>=0;--x)for(int y=0;y<m;++y)
    relax(x*m+y,(x+1)*m+y,xs[x+1]-xs[x]);
}
