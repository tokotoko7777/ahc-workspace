#ifndef AHC_SORTED_BOTTLENECK_ASSIGNMENT_HPP
#define AHC_SORTED_BOTTLENECK_ASSIGNMENT_HPP
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

// Match sorted targets to a subsequence of sorted values, minimizing largest
// absolute distance, then total absolute distance. A returned index is a
// column of values, one per target. Keep magnitudes and total distance below
// numeric_limits<Int>::max(). Time and reconstruction memory O(NM).
template<class Int>
std::vector<int> sorted_bottleneck_assignment(
    const std::vector<Int>& targets,const std::vector<Int>& values) {
  const int n=int(targets.size()),m=int(values.size());
  assert(n<=m && std::is_sorted(targets.begin(),targets.end()) &&
         std::is_sorted(values.begin(),values.end()));
  const auto inf=std::numeric_limits<Int>::max();
  std::vector<Int> prev(m+1),next(m+1);
  auto cost=[&](int i,int j){
    return targets[i]>values[j]?targets[i]-values[j]:values[j]-targets[i];
  };
  for(int i=0;i<n;++i){
    next[0]=inf;
    for(int j=0;j<m;++j)
      next[j+1]=std::min(next[j],std::max(prev[j],cost(i,j)));
    prev.swap(next);
  }
  const Int maximum=prev[m];
  std::fill(prev.begin(),prev.end(),Int{0});
  std::vector<char> parent((n+1)*(m+1));
  for(int i=0;i<n;++i){
    next[0]=inf;
    for(int j=0;j<m;++j){
      next[j+1]=next[j];
      const Int delta=cost(i,j);
      if(prev[j]!=inf && delta<=maximum && prev[j]+delta<next[j+1]){
        next[j+1]=prev[j]+delta;
        parent[(i+1)*(m+1)+j+1]=1;
      }
    }
    prev.swap(next);
  }
  std::vector<int> result(n);
  for(int i=n,j=m;i>0;--j){
    assert(j>0);
    if(parent[i*(m+1)+j])result[--i]=j-1;
  }
  return result;
}
#endif
