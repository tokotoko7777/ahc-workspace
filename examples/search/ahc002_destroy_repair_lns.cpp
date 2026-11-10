// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;

// 提出時は次の1行を、このhppの全文へ置き換える。
#include "../../library/large-neighborhood-search.hpp"

// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc002_destroy_repair_lns.cpp
// Official problem: https://atcoder.jp/contests/ahc002/tasks/ahc002_a

constexpr int BOARD_SIZE = 50;
constexpr int CELL_COUNT = BOARD_SIZE * BOARD_SIZE;
constexpr array<int, 4> DR = {-1, 1, 0, 0};
constexpr array<int, 4> DC = {0, 0, -1, 1};

#ifndef AHC002_SEARCH_TIME_MS
#define AHC002_SEARCH_TIME_MS 1870.0
#endif

#ifndef AHC002_INITIAL_TIME_MS
#define AHC002_INITIAL_TIME_MS 170.0
#endif

struct TilePathProblem {
  // TODO(AHC002): 現在解と、近傍生成・差分評価に使うcache。
  struct State {
    vector<int> path;
    vector<int> prefix_score;
    array<unsigned char, CELL_COUNT> used_tile{};
    int score = 0;
  };

  using Score = int;

  array<int, CELL_COUNT> tile{};
  array<int, CELL_COUNT> point{};
  array<array<int, 4>, CELL_COUNT> next{};
  array<int, CELL_COUNT> next_count{};
  int start_cell = 0;
  uint64_t seed = 0;

  TilePathProblem() {
    for (int row = 0; row < BOARD_SIZE; ++row) {
      for (int column = 0; column < BOARD_SIZE; ++column) {
        const int cell = row * BOARD_SIZE + column;
        if (row > 0) next[cell][next_count[cell]++] = cell - BOARD_SIZE;
        if (row + 1 < BOARD_SIZE) {
          next[cell][next_count[cell]++] = cell + BOARD_SIZE;
        }
        if (column > 0) next[cell][next_count[cell]++] = cell - 1;
        if (column + 1 < BOARD_SIZE) {
          next[cell][next_count[cell]++] = cell + 1;
        }
      }
    }
  }

  void read_input() {
    int start_row = 0;
    int start_column = 0;
    cin >> start_row >> start_column;
    for (int& value : tile) cin >> value;
    for (int& value : point) cin >> value;
    start_cell = start_row * BOARD_SIZE + start_column;

    // 入力だけからseedを作る。同じ入力なら乱数列を再現できる。
    seed = 0x9e3779b97f4a7c15ULL;
    for (int cell = 0; cell < CELL_COUNT; ++cell) {
      seed ^= static_cast<uint64_t>(tile[cell] * 101 + point[cell]);
      seed = seed * 0xbf58476d1ce4e5b9ULL + 0x94d049bb133111ebULL;
    }
  }

  static int random_int(mt19937_64& engine, int upper_bound) {
    assert(upper_bound > 0);
    return static_cast<int>(engine() % static_cast<uint64_t>(upper_bound));
  }

  struct CandidateCell {
    int cell = 0;
    int priority = 0;
  };

  // startから、未使用tileだけを通る末尾をランダム貪欲で作る。
  // 戻り値にstart自身は含まない。
  vector<int> grow_tail(
      int start,
      array<unsigned char, CELL_COUNT>& used,
      mt19937_64& engine,
      int noise_width) const {
    vector<int> tail;
    tail.reserve(CELL_COUNT);
    int current = start;

    while (true) {
      array<CandidateCell, 4> candidates{};
      int candidate_count = 0;
      for (int index = 0; index < next_count[current]; ++index) {
        const int to = next[current][index];
        if (used[tile[to]]) continue;

        used[tile[to]] = 1;
        int onward_count = 0;
        int best_next_point = 0;
        for (int next_index = 0;
             next_index < next_count[to];
             ++next_index) {
          const int next_cell = next[to][next_index];
          if (used[tile[next_cell]]) continue;
          ++onward_count;
          best_next_point = max(best_next_point, point[next_cell]);
        }
        used[tile[to]] = 0;

        const int noise = random_int(engine, noise_width);
        int priority = 4 * point[to] + 45 * onward_count +
                       best_next_point + noise;
        if (onward_count == 0) priority -= 600;
        candidates[candidate_count++] = {to, priority};
      }

      if (candidate_count == 0) break;
      int chosen = 0;
      for (int index = 1; index < candidate_count; ++index) {
        if (candidates[chosen].priority < candidates[index].priority) {
          chosen = index;
        }
      }
      current = candidates[chosen].cell;
      used[tile[current]] = 1;
      tail.push_back(current);
    }
    return tail;
  }

  struct SegmentRepair {
    const TilePathProblem& problem;
    mt19937_64& engine;
    array<unsigned char, CELL_COUNT>& used;
    int target = 0;
    int max_edges = 0;
    int expansion_limit = 0;
    int expansions = 0;
    int best_score = 0;
    vector<int> route;
    vector<int> best_middle;

    SegmentRepair(
        const TilePathProblem& problem_arg,
        mt19937_64& engine_arg,
        array<unsigned char, CELL_COUNT>& used_arg,
        int target_arg,
        int max_edges_arg,
        int expansion_limit_arg,
        int old_score,
        vector<int> old_middle)
        : problem(problem_arg),
          engine(engine_arg),
          used(used_arg),
          target(target_arg),
          max_edges(max_edges_arg),
          expansion_limit(expansion_limit_arg),
          best_score(old_score),
          best_middle(std::move(old_middle)) {
      route.reserve(static_cast<size_t>(max_edges));
    }

    int distance(int first, int second) const {
      return abs(first / BOARD_SIZE - second / BOARD_SIZE) +
             abs(first % BOARD_SIZE - second % BOARD_SIZE);
    }

    void search(int current, int used_edges, int score) {
      if (++expansions > expansion_limit) return;
      const int remaining = max_edges - used_edges;
      if (distance(current, target) > remaining) return;

      array<CandidateCell, 4> candidates{};
      int candidate_count = 0;
      for (int index = 0; index < problem.next_count[current]; ++index) {
        const int to = problem.next[current][index];
        if (to == target) {
          if (score > best_score) {
            best_score = score;
            best_middle = route;
          }
          continue;
        }
        if (used_edges + 1 >= max_edges || used[problem.tile[to]]) continue;
        const int noise = TilePathProblem::random_int(engine, 120);
        candidates[candidate_count++] = {
            to, 5 * problem.point[to] - 8 * distance(to, target) + noise};
      }

      // 候補は最大4件なので、小さい交換sortで分岐順だけ決める。
      for (int first = 0; first < candidate_count; ++first) {
        for (int second = first + 1; second < candidate_count; ++second) {
          if (candidates[first].priority < candidates[second].priority) {
            swap(candidates[first], candidates[second]);
          }
        }
      }

      for (int index = 0; index < candidate_count; ++index) {
        if (expansions >= expansion_limit) break;
        const int to = candidates[index].cell;
        used[problem.tile[to]] = 1;
        route.push_back(to);
        search(to, used_edges + 1, score + problem.point[to]);
        route.pop_back();
        used[problem.tile[to]] = 0;
      }
    }
  };

  int path_score(const vector<int>& path) const {
    int result = 0;
    for (int cell : path) result += point[cell];
    return result;
  }

  void rebuild_cache(State& state) const {
    state.used_tile.fill(0);
    state.prefix_score.assign(state.path.size() + 1, 0);
    for (int index = 0; index < static_cast<int>(state.path.size()); ++index) {
      const int cell = state.path[index];
      state.used_tile[tile[cell]] = 1;
      state.prefix_score[index + 1] =
          state.prefix_score[index] + point[cell];
    }
    state.score = state.prefix_score.back();
  }

  // TODO(AHC002): 多点スタートで必ず合法な初期解を作る。
  State make_initial_state(double time_limit_ms) const {
    mt19937_64 engine(seed);
    const auto started = chrono::steady_clock::now();
    State best;
    best.path = {start_cell};
    rebuild_cache(best);

    int tries = 0;
    do {
      array<unsigned char, CELL_COUNT> used{};
      used[tile[start_cell]] = 1;
      vector<int> path{start_cell};
      const int noise_width = 80 + random_int(engine, 321);
      vector<int> tail = grow_tail(
          start_cell, used, engine, noise_width);
      path.insert(path.end(), tail.begin(), tail.end());
      const int score = path_score(path);
      if (best.score < score) {
        best.path = std::move(path);
        rebuild_cache(best);
      }
      ++tries;
    } while (tries < 24 ||
             chrono::duration<double, milli>(
                 chrono::steady_clock::now() - started).count() <
                 time_limit_ms);
    return best;
  }

  // 問題固有の修復用作業領域。current/bestには保持しない。
  array<unsigned char, CELL_COUNT> repair_used{};
  vector<int> repair_suffix, repair_old_middle;
  int repair_old_score = 0, repair_target = 0, repair_edges = 0;
  int repair_expansions = 0, repair_noise = 0;
  bool repair_tail = true;

  // TODO(AHC002): 壊す区間を選び、残すprefix/suffixと使用済みタイルを準備する。
  // 近傍の種類・DFS上限は既存SA例と同じ。LNSへの横展開を検証する。
  // Runnerの乱数seedの割当・受理方法は異なるため、旧版との逐次一致は主張しない。
  void destroy(const State& current, State& candidate,
               mt19937_64& engine, double progress) {
    const int size = static_cast<int>(current.path.size());
    repair_used = current.used_tile;
    repair_suffix.clear();
    repair_old_middle.clear();
    repair_tail = size < 4 || random_int(engine, 10) < 7;
    if (size <= 1) {
      candidate = current;
      repair_noise = 1;
      return;
    }
    if (repair_tail) {
      const int limit = min(size - 1, 24 + static_cast<int>((1.0 - progress) * 650.0));
      const int removed = 1 + random_int(engine, limit);
      const int cut = size - 1 - removed;
      for (int i = cut + 1; i < size; ++i) repair_used[tile[current.path[i]]] = 0;
      repair_noise = 45 + static_cast<int>((1.0 - progress) * 260.0);
      candidate.path.assign(current.path.begin(), current.path.begin() + cut + 1);
      candidate.score = current.prefix_score[cut + 1];
      return;
    }
    const int max_span = min(size - 1, 5 + static_cast<int>((1.0 - progress) * 13.0));
    const int span = 2 + random_int(engine, max(1, max_span - 1));
    const int left = random_int(engine, size - span), right = left + span;
    repair_old_score = 0;
    for (int i = left + 1; i < right; ++i) {
      const int cell = current.path[i];
      repair_used[tile[cell]] = 0;
      repair_old_middle.push_back(cell);
      repair_old_score += point[cell];
    }
    repair_target = current.path[right];
    repair_edges = min(30, span + 2 + static_cast<int>((1.0 - progress) * 8.0));
    repair_expansions = 220 + static_cast<int>((1.0 - progress) * 650.0);
    candidate.path.assign(current.path.begin(), current.path.begin() + left + 1);
    repair_suffix.assign(current.path.begin() + right, current.path.end());
    candidate.score = current.score - repair_old_score;
  }

  // TODO(AHC002): tailはランダム貪欲、内部区間はノード数制限付きDFSで修復する。
  // AHC059は距離の最小化だったが、こちらは得点の最大化。絶対得点を返す。
  optional<Score> repair(State& candidate, mt19937_64& engine, double,
                         long double threshold) const {
    // 使えるタイルを作業コピーへ。State全体はコピーしない。
    auto used = repair_used;
    if (repair_tail) {
      const auto tail = grow_tail(candidate.path.back(), used, engine, repair_noise);
      for (int cell : tail) candidate.score += point[cell];
      candidate.path.insert(candidate.path.end(), tail.begin(), tail.end());
    } else {
      SegmentRepair search(*this, engine, used, repair_target, repair_edges,
                           repair_expansions, repair_old_score, repair_old_middle);
      search.search(candidate.path.back(), 0, 0);
      candidate.score += search.best_score;
      candidate.path.insert(candidate.path.end(), search.best_middle.begin(), search.best_middle.end());
      candidate.path.insert(candidate.path.end(), repair_suffix.begin(), repair_suffix.end());
    }
    // 完成した得点で採用されないと確定したら、cache再構築を省ける。
    if (candidate.score < threshold) return nullopt;
    const int score = candidate.score;
    rebuild_cache(candidate);
    assert(candidate.score == score);
    return score;
  }

  bool is_valid(const State& state) const {
    if (state.path.empty() || state.path.front() != start_cell) return false;
    array<unsigned char, CELL_COUNT> used{};
    int score = 0;
    for (int index = 0; index < static_cast<int>(state.path.size()); ++index) {
      const int cell = state.path[index];
      if (cell < 0 || cell >= CELL_COUNT || used[tile[cell]]) return false;
      used[tile[cell]] = 1;
      score += point[cell];
      if (index > 0) {
        const int difference = abs(cell - state.path[index - 1]);
        if (difference != 1 && difference != BOARD_SIZE) return false;
        if (difference == 1 &&
            cell / BOARD_SIZE != state.path[index - 1] / BOARD_SIZE) {
          return false;
        }
      }
    }
    return score == state.score;
  }

  void print_answer(const State& answer) const {
    string commands;
    commands.reserve(answer.path.size() - 1);
    for (int index = 1; index < static_cast<int>(answer.path.size()); ++index) {
      const int difference = answer.path[index] - answer.path[index - 1];
      if (difference == -BOARD_SIZE) commands += 'U';
      if (difference == BOARD_SIZE) commands += 'D';
      if (difference == -1) commands += 'L';
      if (difference == 1) commands += 'R';
    }
    cout << commands << '\n';
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  const auto whole_started = chrono::steady_clock::now();
  TilePathProblem problem;
  problem.read_input();
  TilePathProblem::State initial =
      problem.make_initial_state(AHC002_INITIAL_TIME_MS);
  const int initial_score = initial.score;
  const double setup_ms = chrono::duration<double, milli>(
                              chrono::steady_clock::now() - whole_started)
                              .count();
  const double annealing_ms = max(1.0, AHC002_SEARCH_TIME_MS - setup_ms);

  LnsOptions options;
  options.time_limit_ms = annealing_ms;
  options.seed = problem.seed ^ 0x123456789abcdef0ULL;
  options.maximize = true;
#ifndef AHC002_LNS_MODE
#define AHC002_LNS_MODE 2
#endif
  options.acceptance = static_cast<LnsAcceptance>(AHC002_LNS_MODE);
  options.start_temperature = 2400.0;
  options.end_temperature = 8.0;
  options.start_margin = 3000.0;
  options.end_margin = 10.0;
  options.clock_interval = 16;
  LargeNeighborhoodSearch<TilePathProblem> runner(
      problem, std::move(initial), initial_score, options);
  while (runner.step()) {
    if ((runner.iterations() & 511ULL) == 0 &&
        runner.current_score() + 6000 < runner.best_score()) {
      runner.restart_from_best();
    }
  }
  cerr << "iterations=" << runner.iterations() << " accepted=" << runner.accepted()
       << " pruned=" << runner.rejected_repairs() << '\n';

  assert(problem.is_valid(runner.best_state()));
  problem.print_answer(runner.best_state());
}
