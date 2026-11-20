#ifndef AHC_REFLECTION_LAUNCH_RANGE_HPP
#define AHC_REFLECTION_LAUNCH_RANGE_HPP
#include <algorithm>
#include <cstdlib>
#include <utility>

// For one axis of a bounded grid [0,n), minimize
// |start-launch|+|goal-(2*center-launch)| where both reflected positions
// lie in [0,n). The result is an inclusive interval of all minimizers.
inline std::pair<int,int> reflection_launch_range(int n,int start,int goal,int center){
  const int lo=std::max(0,2*center-(n-1)),hi=std::min(n-1,2*center);
  const int mirror=2*center-goal;
  return {std::clamp(std::min(start,mirror),lo,hi),
          std::clamp(std::max(start,mirror),lo,hi)};
}
#endif
