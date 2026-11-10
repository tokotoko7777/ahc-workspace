// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;
#include "../../library/iterated-local-search.hpp"
// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc059_ils.cpp
// 問題: https://atcoder.jp/contests/ahc059/tasks/ahc059_a
// 同じkitのahc059_lns.cppのProblemを流用。大きい摂動と小さい局所探索を分離。
#define AHC059_LNS_PRECOMPUTE 1
#define AHC059_LNS_SPAN 15
#ifndef AHC059_ILS_STALL
#define AHC059_ILS_STALL 64
#endif
#ifndef AHC059_ILS_WALK
#define AHC059_ILS_WALK 0
#endif
struct CardPairProblem {
  // TODO(問題依存): Stateは「カードを取る順番」。要素は元のマス番号。
  // 同じ番号の2枚が交差しない順番だけを保持する。X(置く)は使わない。
  struct State {
    vector<int> order;
    int cost = 0; // (0,0)出発、最後のカードで終了。帰りの距離は含めない。
  };
  using Score = int; // 距離をそのまま返す。Options.maximize=false。
  int n = 0, pairs = 0;
  vector<int> label;
  vector<array<int, 2>> cells;
  vector<int> distances;
  vector<int> removed; // 破壊・修復間だけ使う作業領域。最良解には不要。
  uint64_t seed = 0x123456789abcdefULL;
  bool precompute = AHC059_LNS_PRECOMPUTE;
  int operator_id = -1; // -1=従来版、0..3=区間4/8/15/30、4=離れた4ペア

  void read_input(istream& input = cin) {
    input >> n;
    if (n < 2 || n > 20 || n % 2) throw runtime_error("invalid N");
    pairs = n * n / 2;
    label.resize(n * n);
    cells.resize(pairs);
    vector<int> count(pairs);
    for (int cell = 0; cell < n * n; ++cell) {
      int id;
      if (!(input >> id) || id < 0 || id >= pairs || count[id] == 2) {
        throw runtime_error("invalid card input");
      }
      label[cell] = id;
      cells[id][count[id]++] = cell;
      seed = (seed ^ static_cast<uint64_t>(id + 1)) * 0x9e3779b97f4a7c15ULL;
    }
    if (precompute) {
      distances.resize(n * n * n * n);
      for (int a = 0; a < n * n; ++a) {
        for (int b = 0; b < n * n; ++b) {
          distances[a * n * n + b] = abs(a / n - b / n) + abs(a % n - b % n);
        }
      }
    }
    removed.reserve(pairs);
  }
  int distance(int a, int b) const {
    return precompute ? distances[a * n * n + b] :
        abs(a / n - b / n) + abs(a % n - b % n);
  }
  int route_cost(const vector<int>& order) const {
    int cost = 0, previous = 0;
    for (int cell : order) { cost += distance(previous, cell); previous = cell; }
    return cost;
  }
  struct Insertion { int delta, first_gap, second_gap, first_cell, second_cell; };

  // TODO(問題依存): 1ペアを合法に入れる最小増分をO(現在のカード枚数)で求める。
  // 挿入する2箇所では「山札の中身」が一致する必要がある。
  // 有効な括弧列なので、山札の一番上のペア番号が同じなら祖先も同じ。
  // 各山札状態ごとに、先に置くカードの挿入増分の最小値だけを保持する。
  // 同じ隙間に2枚とも入れる場合は、辺が共有されるので別計算する。
  Insertion best_insertion(const vector<int>& order, int id) const {
    const int infinity = 1000000;
    const int a = cells[id][0], b = cells[id][1];
    array<int, 201> min_a, min_b, pos_a{}, pos_b{}, parent{};
    min_a.fill(infinity); min_b.fill(infinity);
    Insertion best{infinity, 0, 0, a, b};
    int context = 0; // 空の山札=0、トップのペア番号+1=それ以外。
    const int size = static_cast<int>(order.size());
    auto consider = [&](int delta, int i, int j, int x, int y) {
      if (delta < best.delta) best = {delta, i, j, x, y};
    };
    for (int gap = 0; gap <= size; ++gap) {
      const int before = gap == 0 ? 0 : order[gap - 1];
      const int after = gap == size ? -1 : order[gap];
      const int old_edge = after < 0 ? 0 : distance(before, after);
      const int a_after = after < 0 ? 0 : distance(a, after);
      const int b_after = after < 0 ? 0 : distance(b, after);
      const int a_before = distance(before, a), b_before = distance(before, b);
      const int da = a_before + a_after - old_edge;
      const int db = b_before + b_after - old_edge;
      if (min_a[context] != infinity) {
        consider(min_a[context] + db, pos_a[context], gap, a, b);
      }
      if (min_b[context] != infinity) {
        consider(min_b[context] + da, pos_b[context], gap, b, a);
      }
      consider(a_before + distance(a, b) + b_after - old_edge, gap, gap, a, b);
      consider(b_before + distance(a, b) + a_after - old_edge, gap, gap, b, a);
      if (da < min_a[context]) { min_a[context] = da; pos_a[context] = gap; }
      if (db < min_b[context]) { min_b[context] = db; pos_b[context] = gap; }
      if (gap < size) {
        const int next_context = label[order[gap]] + 1;
        if (next_context == context) context = parent[context];
        else { parent[next_context] = context; context = next_context; }
      }
    }
    return best;
  }
  void insert_pair(State& state, int id) const {
    const Insertion choice = best_insertion(state.order, id);
    // 後ろから挿入すれば元のgap位置がずれない。同じgapでも順番はfirst,second。
    state.order.insert(state.order.begin() + choice.second_gap, choice.second_cell);
    state.order.insert(state.order.begin() + choice.first_gap, choice.first_cell);
    state.cost += choice.delta;
  }
  State make_initial_state() const {
    State state;
    state.order.reserve(n * n);
    vector<int> ids(pairs);
    iota(ids.begin(), ids.end(), 0);
    mt19937_64 engine(seed);
    shuffle(ids.begin(), ids.end(), engine);
    for (int id : ids) insert_pair(state, id);
    return state;
  }
  // TODO(問題依存): 区間内に現れるペアを2枚とも除く。残りは必ず合法。
  void destroy(const State& current, State& candidate,
               mt19937_64& engine, double /* progress */) {
    const int size = static_cast<int>(current.order.size());
    array<bool, 200> erase{};
    removed.clear();
    if (operator_id == 4) {
      // TODO(問題依存): 離れたペアを選ぶ別の壊し方。重複なしで最大4組。
      while (static_cast<int>(removed.size()) < min(4, pairs)) {
        const int id = static_cast<int>(engine() % static_cast<uint64_t>(pairs));
        if (!erase[id]) { erase[id] = true; removed.push_back(id); }
      }
    } else {
      constexpr int spans[] = {4, 8, 15, 30};
      const int span = min(size, operator_id < 0 ? AHC059_LNS_SPAN : spans[operator_id]);
      const int left = static_cast<int>(engine() % static_cast<uint64_t>(size - span + 1));
      for (int i = left; i < left + span; ++i) {
        const int id = label[current.order[i]];
        if (!erase[id]) { erase[id] = true; removed.push_back(id); }
      }
    }
    candidate.order.clear(); // capacityは捨てない。current全体もコピーしない。
    for (int cell : current.order) if (!erase[label[cell]]) candidate.order.push_back(cell);
    candidate.cost = route_cost(candidate.order);
    // 打ち切りのON/OFFで次の乱数列が変わらないよう、乱数は修復前に消費する。
    shuffle(removed.begin(), removed.end(), engine);
  }
  // TODO(問題依存): 修復成功時だけ、完成候補の絶対距離を返す。
  optional<Score> repair(State& candidate, mt19937_64&, double,
                         long double threshold) const {
    for (int id : removed) {
      // Manhattan距離の三角不等式により、挿入で距離は減らない。
      // だから途中の距離が上限を超えたら、この修復は採用され得ない。
      if (candidate.cost > threshold) return nullopt;
      insert_pair(candidate, id);
    }
    return candidate.cost;
  }
  bool is_valid(const State& state) const {
    vector<bool> used(n * n);
    vector<int> stack;
    for (int cell : state.order) {
      if (cell < 0 || cell >= n * n || used[cell]) return false;
      used[cell] = true;
      const int id = label[cell];
      if (!stack.empty() && stack.back() == id) stack.pop_back();
      else stack.push_back(id);
    }
    return stack.empty() && route_cost(state.order) == state.cost;
  }
  void print_answer(const State& answer) const {
    int row = 0, col = 0;
    for (int cell : answer.order) {
      const int target_row = cell / n, target_col = cell % n;
      while (row < target_row) { cout << "D\n"; ++row; }
      while (row > target_row) { cout << "U\n"; --row; }
      while (col < target_col) { cout << "R\n"; ++col; }
      while (col > target_col) { cout << "L\n"; --col; }
      cout << "Z\n";
    }
  }

  State local_scratch;
  void perturb(const State& current, State& candidate, mt19937_64& rng, const IlsBudget&) {
    operator_id = 3; // 大きい変更: 区間30に出るペアを除いて修復。
    destroy(current, candidate, rng, 0);
    repair(candidate, rng, 0, numeric_limits<long double>::infinity());
  }
  Score local_search(State& state, mt19937_64& rng, const IlsBudget& budget) {
    operator_id = 0; // 小さい改善: 区間4。失敗が続くまで、同点も採用して歩く。
    int failures = 0;
    while (failures < AHC059_ILS_STALL && !budget.expired()) {
      destroy(state, local_scratch, rng, budget.progress());
      const auto score = repair(local_scratch, rng, 0, state.cost);
      const bool improved = score && *score < state.cost;
      if (score && *score <= state.cost) swap(state, local_scratch);
      failures = improved ? 0 : failures + 1;
      ++local_trials;
    }
    return state.cost; // 打ち切り時にも常に合法な完成解。
  }
  uint64_t local_trials = 0;
};

#ifndef AHC059_ILS_NO_MAIN
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  const auto started = chrono::steady_clock::now();
  CardPairProblem problem; problem.read_input();
  auto initial = problem.make_initial_state();
  const int score = initial.cost;
  IlsOptions options;
  options.maximize = false; options.seed = problem.seed;
  options.accept_worse = AHC059_ILS_WALK;
  options.time_limit_ms = max(1.0, 1850.0 -
      chrono::duration<double, milli>(chrono::steady_clock::now() - started).count());
  IteratedLocalSearch<CardPairProblem> search(problem, std::move(initial), score, options);
  search.run();
  assert(problem.is_valid(search.best_state()));
  problem.print_answer(search.best_state());
  cerr << "iterations=" << search.iterations() << " local_trials=" << problem.local_trials
       << " cost=" << search.best_score() << '\n';
}
#endif
