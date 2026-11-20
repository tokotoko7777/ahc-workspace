#define AHC006_NO_MAIN
#include "../../examples/search/ahc006_sa.cpp"

#include "batch-repair.hpp"

namespace {
DeliveryProblem::State make_spatial_seed(const DeliveryProblem& problem,
                                         DeliveryProblem::State best) {
  const auto dist = [&](int a, int b) { return problem.distance(a, b); };
  // Off-center starts can discover a dense directed pocket even if its
  // requests are individually unattractive when scored from the depot.
  for (int x : {300, 400, 500}) for (int y : {300, 400, 500}) {
    vector<pair<int, int>> ranked;
    for (int id = 0; id < ORDER_COUNT; ++id) {
      const auto& o = problem.orders[id];
      int a = abs(o.pickup_x - x) + abs(o.pickup_y - y);
      int b = abs(o.delivery_x - x) + abs(o.delivery_y - y);
      ranked.emplace_back(2 * max(a, b) + min(a, b), id);
    }
    constexpr int kPool = 128;
    partial_sort(ranked.begin(), ranked.begin() + kPool, ranked.end());
    ranked.resize(kPool);

    DeliveryProblem::State state;
    vector<int> route{DEPOT_EVENT, DEPOT_EVENT};
    for (int i = 0; i < CHOSEN_COUNT; ++i) {
      int best_rank = INT_MAX, best_id = -1;
      OrderedPairInsertion<int> gaps{};
      for (const auto& [bias, id] : ranked) {
        if (state.selected[id]) continue;
        auto p = best_ordered_pair_insertion(route, 2 * id, 2 * id + 1, dist);
        int value = 20 * p.delta + bias;
        if (value < best_rank)
          best_rank = value, best_id = id, gaps = p;
      }
      state.selected[best_id] = true;
      route.insert(route.begin() + gaps.second_gap, 2 * best_id + 1);
      route.insert(route.begin() + gaps.first_gap, 2 * best_id);
    }
    copy(route.begin(), route.end(), state.route.begin());
    state.cost = problem.route_cost(state.route);
    if (state.cost < best.cost) {
      state.position.fill(-1);
      DeliveryProblem::refresh_positions(state, 1, ROUTE_SIZE - 2);
      best = state;
    }
  }
  return best;
}
} // namespace

int main() {
  const auto start = chrono::steady_clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  DeliveryProblem problem;
  problem.read_input();
  auto initial = problem.make_initial_state();
  const int inherited_cost = initial.cost;
#if defined(AHC006_SPATIAL_SEED) && AHC006_SPATIAL_SEED
  initial = make_spatial_seed(problem, initial);
#endif
  assert(problem.is_valid(initial.route));
  const auto setup_ms =
      chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
#ifdef AHC006_BATCH_REPAIR_DIAGNOSTIC
  constexpr double sa_limit = 1740.0;
#else
  constexpr double sa_limit = SEARCH_EXAMPLE_TIME_LIMIT_MS;
#endif
  TimeBasedAnnealingRunner<DeliveryProblem> runner(
      problem, initial, -initial.cost,
      max(0.001, sa_limit - setup_ms), 120.0, 1.0,
      20211115, 64);
  runner.run();
  auto answer = runner.best_state();
#ifdef AHC006_BATCH_REPAIR_DIAGNOSTIC
  const int frozen = answer.cost;
  auto r = ahc006_batch::diagnose(problem, answer, start + chrono::milliseconds(1910));
  cerr << "frozen=" << frozen << " greedy=" << r.greedy.cost
       << " beam=" << r.beam.cost << " trials=" << r.trials
       << " greedy_improved=" << r.greedy_improved << " beam_improved=" << r.beam_improved << '\n';
  answer = r.beam;
#endif
  assert(problem.is_valid(answer.route));
  assert(problem.route_cost(answer.route) == answer.cost);
  print_answer(problem, answer);
#ifdef AHC006_STATS
  cerr << "inherited=" << inherited_cost << " initial=" << initial.cost
       << " setup_ms=" << setup_ms << " iterations=" << runner.iterations()
       << " cost=" << answer.cost << '\n';
#endif
}
