#ifndef AHC_REFLECTION_LAUNCH_RANGE_HPP
#define AHC_REFLECTION_LAUNCH_RANGE_HPP
#include <algorithm>
#include <cstdlib>
#include <optional>
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
// Two consecutive reflections compose to a translation. The launch must
// leave all three positions (before, between, after) within [0,n).
inline std::optional<std::pair<int,int>> double_reflection_launch_range(
    int n,int start,int goal,int first,int second){
  const int delta=2*(second-first);
  const int lo=std::max({0,2*first-(n-1),-delta});
  const int hi=std::min({n-1,2*first,n-1-delta});
  if(lo>hi)return std::nullopt;
  return std::pair{std::clamp(std::min(start,goal-delta),lo,hi),
                   std::clamp(std::max(start,goal-delta),lo,hi)};
}
#endif
