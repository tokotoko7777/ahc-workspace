#pragma once
// Include after DeliveryProblem. A diagnostic on frozen near-local optima.
namespace ahc006_batch {

struct Result {
  DeliveryProblem::State greedy, beam;
  int trials = 0, greedy_improved = 0, beam_improved = 0;
};

inline Result diagnose(const DeliveryProblem& problem,
                       const DeliveryProblem::State& initial,
                       std::chrono::steady_clock::time_point stop) {
  struct Partial {
    std::vector<int> route;
    int cost;
  };
  const auto dist = [&](int a, int b) { return problem.distance(a, b); };
  auto cost = [&](const auto& route) {
    int value = 0;
    for (int i = 1; i < int(route.size()); ++i) value += dist(route[i - 1], route[i]);
    return value;
  };
  auto state = [&](const Partial& partial) {
    DeliveryProblem::State s;
    std::copy(partial.route.begin(), partial.route.end(), s.route.begin());
    for (int i = 1; i + 1 < ROUTE_SIZE; ++i) s.selected[s.route[i] / 2] = true;
    s.position.fill(-1);
    DeliveryProblem::refresh_positions(s, 1, ROUTE_SIZE - 2);
    s.cost = partial.cost;
    assert(problem.is_valid(s.route) && cost(s.route) == s.cost);
    return s;
  };
  Result result{initial, initial};
  constexpr int width = 4;
  constexpr int shortlist = 48;
  constexpr int trial_count = 32;
  for (int trial = 0; trial < trial_count; ++trial) {
    if (std::chrono::steady_clock::now() >= stop) break;
    int begin = 1 + trial * 37 % (ROUTE_SIZE - 10);
    int k = 3 + (trial % 2);
    std::vector<int> removed;
    for (int i = begin; int(removed.size()) < k; ++i) {
      int id = initial.route[i] / 2;
      if (std::find(removed.begin(), removed.end(), id) == removed.end())
        removed.push_back(id);
    }
    std::bitset<ORDER_COUNT> cut;
    for (int id : removed) cut[id] = true;
    Partial root{};
    for (int node : initial.route)
      if (node == DEPOT_EVENT || !cut[node / 2]) root.route.push_back(node);
    root.cost = cost(root.route);

    // Only consider requests spatially similar to the removed directed requests.
    std::vector<std::pair<int, int>> keys;
    for (int id = 0; id < ORDER_COUNT; ++id) {
      if (initial.selected[id] && !cut[id]) continue;
      int bias = INT_MAX;
      for (int q : removed)
        bias = std::min(bias, dist(2 * id, 2 * q) + dist(2 * id + 1, 2 * q + 1));
      keys.emplace_back(bias, id);
    }
    std::partial_sort(keys.begin(), keys.begin() + shortlist, keys.end());
    keys.resize(shortlist);
    auto repair = [&](int w) -> std::optional<Partial> {
      std::vector<Partial> frontier{root};
      for (int depth = 0; depth < k; ++depth) {
        if (std::chrono::steady_clock::now() >= stop) return std::nullopt;
        using Option = std::tuple<int, int, int, int, int>; // cost, arm path, order, gaps
        std::vector<Option> options;
        for (int j = 0; j < int(frontier.size()); ++j) {
          const auto& p = frontier[j];
          std::bitset<ORDER_COUNT> used;
          for (int event : p.route) if (event != DEPOT_EVENT) used[event / 2] = true;
          for (const auto& [bias, id] : keys) {
            if (used[id]) continue;
            auto gap = best_ordered_pair_insertion(p.route, 2 * id, 2 * id + 1, dist);
            options.emplace_back(p.cost + gap.delta, j, id, gap.first_gap, gap.second_gap);
          }
        }
        std::sort(options.begin(), options.end());
        std::vector<Partial> next;
        for (const auto& [v, j, id, a, b] : options) {
          Partial p{frontier[j].route, v};
          p.route.insert(p.route.begin() + b, 2 * id + 1);
          p.route.insert(p.route.begin() + a, 2 * id);
          assert(cost(p.route) == p.cost);
          if (std::none_of(next.begin(), next.end(),
                           [&](const auto& q) { return p.route == q.route; }))
            next.push_back(std::move(p));
          if (int(next.size()) == w) break;
        }
        frontier = std::move(next);
      }
      return std::move(frontier.front());
    };
    const auto a = repair(1);
    const auto b = repair(width);
    if (!a || !b) break; // compare matched completed trials only
    ++result.trials;
    result.greedy_improved += a->cost < initial.cost;
    result.beam_improved += b->cost < initial.cost;
    if (a->cost < result.greedy.cost) result.greedy = state(*a);
    if (b->cost < result.beam.cost) result.beam = state(*b);
  }
  return result;
}
} // namespace ahc006_batch
