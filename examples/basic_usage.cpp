#include <cassert>
#include <iostream>
#include <random>
#include <vector>

#include "library/time-based-simulated-annealing.hpp"
#include "library/prefix-replay.hpp"
#include "library/axis-aligned-rectangle.hpp"
#include "library/largest-empty-rectangle.hpp"

// Input-free examples. Run from the repository root:
// g++ -O2 -std=c++17 -Wall -Wextra -I . examples/basic_usage.cpp -o /tmp/ahc_basic_usage
// /tmp/ahc_basic_usage

void annealing_example() {
  // Maximize -(x - 37)^2: the optimum is x = 37, score = 0.
  const auto score = [](int x) { return -1LL * (x - 37) * (x - 37); };
  std::mt19937_64 rng(123);
  std::uniform_int_distribution<int> direction(0, 1);
  int current = 0;
  long long current_score = score(current);
  int best = current;
  long long best_score = current_score;

  // Milliseconds, initial/final temperatures, acceptance seed, clock interval.
  TimeBasedSimulatedAnnealing sa(50.0, 20.0, 0.1, 456, 1);
  while (!sa.is_over()) {
    const int candidate = current + (direction(rng) ? 1 : -1);
    if (candidate < -100 || candidate > 100) continue;
    const long long candidate_score = score(candidate);
    if (sa.accept(candidate_score - current_score)) {
      current = candidate;
      current_score = candidate_score;
      if (current_score > best_score) {
        best = current;
        best_score = current_score;
      }
    }
  }
  assert(best_score == score(best) && best_score >= score(0));
  // A wall-clock budget can change the iteration count across machines.
  std::cout << "annealing: x=" << best << " score=" << best_score << '\n';
}

void replay_example() {
  struct State { long long sum = 0; };
  const auto step = [](State& state, const int& action) { state.sum += action; };
  PrefixReplay<State, int> replay(State{}, 2);
  std::vector<int> actions{1, 2, 3, 4};
  replay.evaluate(actions, step);
  replay.commit();

  actions[2] = 8;
  const State& trial = replay.evaluate(actions, step);
  assert(trial.sum == 15 && replay.current_end().sum == 10);
  assert(replay.last_replayed_actions() == 2);
  std::cout << "replay: sum=" << trial.sum
            << " replayed_actions=" << replay.last_replayed_actions() << '\n';
  replay.commit();  // Do not use the trial reference after commit/discard.

  actions[0] = -100;
  replay.evaluate(actions, step);
  replay.discard();
  assert(replay.current_end().sum == 15);
}

void rectangle_example() {
  using Rect = AxisAlignedRectangle<int>;
  const Rect a{0, 0, 4, 3};
  const Rect touching{4, 0, 6, 3};
  assert(a.is_valid() && a.area() == 12);
  assert(a.contains(3, 2) && !a.contains(4, 2));
  assert(!a.overlaps(touching));
  std::cout << "rectangle: area=" << a.area()
            << " touching_overlap=" << a.overlaps(touching) << '\n';

  const Rect bounds{0, 0, 10, 8};
  const std::vector<Rect> obstacles{{4, 0, 6, 8}};
  LargestEmptyRectangleWorkspace<Rect> workspace;
  const Rect empty = largest_empty_rectangle(bounds, 1, 2, obstacles, workspace);
  assert(empty.contains(1, 2) && empty.area() == 32);
  assert(!empty.overlaps(obstacles[0]));
  std::cout << "empty rectangle: " << empty.left << ' ' << empty.bottom << ' '
            << empty.right << ' ' << empty.top << " area=" << empty.area() << '\n';
}

int main() {
  annealing_example();
  replay_example();
  rectangle_example();
}
