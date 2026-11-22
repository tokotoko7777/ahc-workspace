# Minimize longest assignment on a fixed construction walk

[TERRY’s AHC008 write-up](https://blog.terry-u16.net/entry/ahc008) describes
assigning contiguous sections of a prescribed construction walk to workers.
This independent helper accelerates the fixed-walk version when shortest
distances to consecutive task stances satisfy the one-sided triangle bound.
It is a library proposal; no solver is using it yet.

```cpp
std::vector<route_makespan::Cost> work = {1, 1, 1, 1};
std::vector<route_makespan::Cost> walk = {1, 1, 1, 0};
std::vector<std::vector<route_makespan::Cost>> approaches = {{0,1,2,3}, {3,2,1,0}};
auto plan = route_makespan::solve(work, walk, approaches); // makespan 4
```

For a time threshold, two pointers compute the maximum reachable next task
index for every starting task and worker. Starting later never raises the
cost of a fixed end index: the route's saved travel plus work covers the
difference in initial approach distances. A subset DP therefore needs only
the furthest achievable prefix for each used-worker subset. Binary search
returns the smallest threshold; parent pointers recover disjoint half-open
segments and workers. Per threshold: (O(M L+M,2^M)) time and
(O(M L+2^M)) memory.

This models static movement on an executable prescribed walk only.
Walls added by multiple people and animals can change allowed paths on an
interactive board, so validate actual commands and replan if necessary.

```sh
g++ -std=c++23 test.cpp -o test
./test  # 3000 randomized generated cases vs exhaustive small schedules
```
