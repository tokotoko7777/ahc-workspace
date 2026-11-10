// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;

// 提出時は次のincludeを、各hppの全文へ置き換える。
#include "../../library/time-based-simulated-annealing.hpp"
#include "../../library/route-utils.hpp"
#include "../../library/ordered-pair-insertion.hpp"

// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc006_sa.cpp
// Official problem: https://atcoder.jp/contests/ahc006/tasks/ahc006_a

// ============================================================================
// ここから問題ごとに書く部分。定数、入力型、Problem、出力を含む。
// ============================================================================
constexpr int ORDER_COUNT = 1000;
constexpr int CHOSEN_COUNT = 50;
constexpr int ROUTE_SIZE = 2 * CHOSEN_COUNT + 2;
constexpr int DEPOT_EVENT = 2 * ORDER_COUNT;

#ifndef SEARCH_EXAMPLE_TIME_LIMIT_MS
#define SEARCH_EXAMPLE_TIME_LIMIT_MS 1850.0
#endif
#ifndef AHC006_PRECOMPUTE_DISTANCE
#define AHC006_PRECOMPUTE_DISTANCE 1
#endif

struct Order {
  int pickup_x;
  int pickup_y;
  int delivery_x;
  int delivery_y;
};

struct DeliveryProblem {
  using Route = array<int, ROUTE_SIZE>;

  // TODO(AHC006): 現在解と、近傍評価に必要なcacheをStateへ置く。
  struct State {
    Route route{};
    array<bool, ORDER_COUNT> selected{};
    // TODO: 差分制約チェック用。eventの現在位置。未選択なら-1。
    array<int, DEPOT_EVENT> position{};
    int cost = 0;
  };

  // TODO: 採用時に必要な変更だけ持つ。候補の経路コピーは持たない。
  enum Kind { Relocate, Swap, Reverse, Replace };
  struct Move {
    Kind kind = Relocate;
    int first = 0, second = 0;
    int delta = 0; // 新しい距離 - 現在距離（小さいほど良い）。
    int removed_order = -1;
    int inserted_order = -1;
    int first_gap = 0, second_gap = 0;
  };

  // Runnerでは大きいほど良い値にするため、scoreは距離の符号を反転する。
  using Score = int;

  array<Order, ORDER_COUNT> orders{};
#if AHC006_PRECOMPUTE_DISTANCE
  // 任意の前計算。距離<=1600なのでuint16_tで十分（差分はintに戻す）。約8MB。
  vector<uint16_t> distances;
#endif

  void read_input() {
    for (Order& order : orders) {
      cin >> order.pickup_x >> order.pickup_y
          >> order.delivery_x >> order.delivery_y;
    }
    prepare_distances();
  }

  pair<int, int> point(int event) const {
    if (event == DEPOT_EVENT) return {400, 400};
    const Order& order = orders[event / 2];
    if (event % 2 == 0) return {order.pickup_x, order.pickup_y};
    return {order.delivery_x, order.delivery_y};
  }

  int direct_distance(int first, int second) const {
    const auto [x1, y1] = point(first);
    const auto [x2, y2] = point(second);
    return abs(x1 - x2) + abs(y1 - y2);
  }

  void prepare_distances() {
#if AHC006_PRECOMPUTE_DISTANCE
    distances.resize((DEPOT_EVENT + 1) * (DEPOT_EVENT + 1));
    for (int a = 0; a <= DEPOT_EVENT; ++a)
      for (int b = 0; b <= DEPOT_EVENT; ++b)
        distances[a * (DEPOT_EVENT + 1) + b] = direct_distance(a, b);
#endif
  }

  int distance(int first, int second) const {
#if AHC006_PRECOMPUTE_DISTANCE
    return static_cast<int>(distances[first * (DEPOT_EVENT + 1) + second]);
#else
    return direct_distance(first, second);
#endif
  }

  int route_cost(const Route& route) const {
    int cost = 0;
    for (int i = 1; i < ROUTE_SIZE; ++i) {
      cost += distance(route[i - 1], route[i]);
    }
    return cost;
  }

  // 先行制約を守る最良2位置を線形走査する。二重ループ・候補コピー不要。
  void insert_order_best(vector<int>& route, int order_id) const {
    const int pickup = 2 * order_id;
    const int delivery = pickup + 1;
    const auto best = best_ordered_pair_insertion(route, pickup, delivery,
        [&](int a, int b) { return distance(a, b); });
    route.insert(route.begin() + best.second_gap, delivery);
    route.insert(route.begin() + best.first_gap, pickup);
  }

  bool is_valid(const Route& route) const {
    if (route.front() != DEPOT_EVENT || route.back() != DEPOT_EVENT) {
      return false;
    }
    bitset<ORDER_COUNT> picked_up;
    bitset<ORDER_COUNT> delivered;
    int used = 0;
    for (int position = 1; position + 1 < ROUTE_SIZE; ++position) {
      const int event = route[position];
      if (event < 0 || event >= DEPOT_EVENT) return false;
      const int order = event / 2;
      if (event % 2 == 0) {
        if (picked_up[order]) return false;
        picked_up[order] = true;
        ++used;
      } else {
        if (!picked_up[order] || delivered[order]) return false;
        delivered[order] = true;
      }
    }
    return used == CHOSEN_COUNT && picked_up == delivered;
  }

  // TODO(AHC006): 必ず合法な初期解を作る。
  State make_initial_state() const {
    vector<int> order_ids(ORDER_COUNT);
    iota(order_ids.begin(), order_ids.end(), 0);
    sort(order_ids.begin(), order_ids.end(), [&](int first, int second) {
      const auto single_cost = [&](int order_id) {
        const int pickup = 2 * order_id;
        const int delivery = pickup + 1;
        return distance(DEPOT_EVENT, pickup) + distance(pickup, delivery) +
               distance(delivery, DEPOT_EVENT);
      };
      const int left = single_cost(first);
      const int right = single_cost(second);
      return left != right ? left < right : first < second;
    });

    State state;
    vector<int> route{DEPOT_EVENT, DEPOT_EVENT};
    for (int i = 0; i < CHOSEN_COUNT; ++i) {
      state.selected[order_ids[i]] = true;
      insert_order_best(route, order_ids[i]);
    }
    copy(route.begin(), route.end(), state.route.begin());
    state.position.fill(-1);
    refresh_positions(state, 1, ROUTE_SIZE - 2);
    state.cost = route_cost(state.route);
    assert(is_valid(state.route));
    return state;
  }

  static int random_int(mt19937_64& engine, int left, int right) {
    return uniform_int_distribution<int>(left, right - 1)(engine);
  }

  static void refresh_positions(State& state, int left, int right) {
    for (int i = left; i <= right; ++i) state.position[state.route[i]] = i;
  }

  // 2位置を除いた仮想経路。参照するだけなので候補ごとのコピー・確保なし。
  struct WithoutPair {
    const Route& route;
    int first, second; // first < second
    int size() const { return ROUTE_SIZE - 2; }
    int operator[](int i) const {
      if (i >= first) ++i;
      if (i >= second) ++i;
      return route[i];
    }
  };

  // swapで位置が変わる2イベントについてだけ先行制約を調べれば十分。
  static bool swap_is_valid(const State& state, int first, int second) {
    const auto valid = [&](int from, int to) {
      const int event = state.route[from];
      int counterpart = state.position[event ^ 1];
      if (counterpart == to) counterpart = from;
      return event % 2 == 0 ? to < counterpart : counterpart < to;
    };
    return valid(first, second) && valid(second, first);
  }

  // TODO(AHC006): pickup-before-deliveryを壊さない近傍を1個作る。
  optional<Move> propose_move(
      const State& state, mt19937_64& engine, double progress) const {
    Move move;
    const auto dist = [&](int a, int b) { return distance(a, b); };
    const int neighborhood = random_int(engine, 0, 100);

    if (neighborhood < 40) {
      // 1イベントを、対応するイベントとの前後関係を守って移動する。
      const int from = random_int(engine, 1, ROUTE_SIZE - 1);
      const int event = state.route[from];
      const int counterpart_position = state.position[event ^ 1] -
                                        (state.position[event ^ 1] > from);
      const int to = event % 2 == 0
                         ? random_int(engine, 1, counterpart_position + 1)
                         : random_int(
                               engine, counterpart_position + 1,
                               ROUTE_SIZE - 1);
      move.kind = Relocate;
      move.first = from; move.second = to;
      move.delta = route_relocate_delta(state.route, from, to, dist);
    } else if (neighborhood < 70) {
      const int first = random_int(engine, 1, ROUTE_SIZE - 1);
      const int second = random_int(engine, 1, ROUTE_SIZE - 1);
      if (first == second) return nullopt;
      if (!swap_is_valid(state, first, second)) return nullopt;
      move.kind = Swap;
      move.first = first; move.second = second;
      move.delta = route_swap_delta(state.route, first, second, dist);
    } else if (neighborhood < 95) {
      const int left = random_int(engine, 1, ROUTE_SIZE - 2);
      // 前半は広め、終盤は狭い区間を試す。
      const int maximum_length =
          max(2, static_cast<int>(12.0 - 8.0 * progress));
      const int right_limit = min(ROUTE_SIZE - 2, left + maximum_length);
      if (left >= right_limit) return nullopt;
      const int right = random_int(engine, left + 1, right_limit + 1);
      // 両端が区間内にある注文だけが先行制約を壊す。区間外は調べない。
      for (int i = left; i <= right; ++i) {
        const int event = state.route[i];
        if (event % 2 == 0 && state.position[event + 1] <= right) return nullopt;
      }
      move.kind = Reverse;
      move.first = left; move.second = right;
      move.delta = route_reverse_delta(state.route, left, right, dist);
    } else {
      // 選択注文を1件外し、未選択注文を最良の2位置へ挿入する。
      const int position = random_int(engine, 1, ROUTE_SIZE - 1);
      move.kind = Replace;
      move.removed_order = state.route[position] / 2;
      do {
        move.inserted_order = random_int(engine, 0, ORDER_COUNT);
      } while (state.selected[move.inserted_order]);

      const int p = state.position[2 * move.removed_order];
      const int d = state.position[2 * move.removed_order + 1];
      move.first = p; move.second = d;
      const Route& r = state.route;
      // 隣接する2点を削除する場合、共有辺を二重計上しない。
      const int removal = d == p + 1
          ? dist(r[p - 1], r[d + 1]) - dist(r[p - 1], r[p]) -
            dist(r[p], r[d]) - dist(r[d], r[d + 1])
          : route_removal_delta(r, p, dist) + route_removal_delta(r, d, dist);
      const auto best = best_ordered_pair_insertion(WithoutPair{r, p, d},
          2 * move.inserted_order, 2 * move.inserted_order + 1, dist);
      move.first_gap = best.first_gap; move.second_gap = best.second_gap;
      move.delta = removal + best.delta;
    }

    return move;
  }

  // TODO(AHC006): 正なら改善となる差分を返す。Stateは変更しない。
  optional<Score> evaluate_move(const State& /* state */, const Move& move,
                                double /* threshold */) const {
    // TODO: 正確な改善量を返す。今回はproposeで辺差分が確定しているので-negateだけ。
    // 閾値による途中打切りは不要。通常近傍O(1)、注文入替の最良挿入探索はO(n)。
    return -move.delta;
  }

  // TODO(AHC006): 採用された近傍だけをStateへ反映する。
  void apply_move(State& state, const Move& move) const {
    Route& r = state.route;
    const int a = move.first, b = move.second;
    if (move.kind == Relocate) {
      if (a < b) rotate(r.begin() + a, r.begin() + a + 1, r.begin() + b + 1);
      else if (a > b) rotate(r.begin() + b, r.begin() + a, r.begin() + a + 1);
      refresh_positions(state, min(a, b), max(a, b));
    } else if (move.kind == Swap) {
      swap(r[a], r[b]);
      state.position[r[a]] = a; state.position[r[b]] = b;
    } else if (move.kind == Reverse) {
      reverse(r.begin() + a, r.begin() + b + 1);
      refresh_positions(state, a, b);
    } else {
      int n = 0;
      for (int i = 0; i < ROUTE_SIZE; ++i)
        if (i != a && i != b) r[n++] = r[i];
      // secondを先に入れる。2位置は「2点を除いた元経路」のgap添字。
      for (int i = n; i > move.second_gap; --i) r[i] = r[i - 1];
      r[move.second_gap] = 2 * move.inserted_order + 1;
      ++n;
      for (int i = n; i > move.first_gap; --i) r[i] = r[i - 1];
      r[move.first_gap] = 2 * move.inserted_order;
      state.position[2 * move.removed_order] = -1;
      state.position[2 * move.removed_order + 1] = -1;
      state.selected[move.removed_order] = false;
      state.selected[move.inserted_order] = true;
      refresh_positions(state, 1, ROUTE_SIZE - 2);
    }
    state.cost += move.delta;
  }
};

void print_answer(
    const DeliveryProblem& problem, const DeliveryProblem::State& answer) {
  array<bool, ORDER_COUNT> already_output{};
  vector<int> chosen_orders;
  for (int position = 1; position + 1 < ROUTE_SIZE; ++position) {
    const int order = answer.route[position] / 2;
    if (!already_output[order]) {
      already_output[order] = true;
      chosen_orders.push_back(order);
    }
  }

  cout << chosen_orders.size();
  for (int order : chosen_orders) cout << ' ' << order + 1;
  cout << '\n';

  cout << ROUTE_SIZE;
  for (int event : answer.route) {
    const auto [x, y] = problem.point(event);
    cout << ' ' << x << ' ' << y;
  }
  cout << '\n';
}

// ============================================================================
// ここから下は探索の呼び出し。時計・温度・採否・best保存はRunnerが担当する。
// ============================================================================
#ifndef AHC006_NO_MAIN
int main() {
  const auto start = chrono::steady_clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  DeliveryProblem problem;
  problem.read_input();
  DeliveryProblem::State initial = problem.make_initial_state();

  TimeBasedAnnealingRunner<DeliveryProblem> runner(
      problem, initial, -initial.cost,
      max(0.001, SEARCH_EXAMPLE_TIME_LIMIT_MS -
          chrono::duration<double, milli>(chrono::steady_clock::now() - start).count()),
      120.0, 1.0,  // TODO(AHC006): 開始温度、終了温度。
      20211115, 64);
  runner.run();

  const DeliveryProblem::State& answer = runner.best_state();
  assert(problem.is_valid(answer.route));
  assert(problem.route_cost(answer.route) == answer.cost);
  print_answer(problem, answer);
#ifdef AHC006_STATS
  cerr << "iterations=" << runner.iterations() << " accepted=" << runner.accepted_moves()
       << " cost=" << answer.cost << '\n';
#endif
}
#endif
