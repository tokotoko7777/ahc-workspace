#include "../library/small-bottleneck-assignment.hpp"
#include <cassert>
int main() {
  // Gate A: workers arrive at 1, 3, 7. Gate B: 4, 6, unreachable.
  const int arrivals[] = {1,3,7, 4,6,-1};
  const auto r = small_bottleneck_assignment::solve(2,3,arrivals);
  assert(r && r->latest == 4 && r->sum == 7);
  assert(r->worker_for_job[0] == 1 && r->worker_for_job[1] == 0);
}
