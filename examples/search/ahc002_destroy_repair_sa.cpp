// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;

// 提出時は次の1行を、このhppの全文へ置き換える。
#include "../../library/time-based-simulated-annealing.hpp"

// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc002_destroy_repair_sa.cpp
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

  // TODO(AHC002): destroy/repairで作った次の合法経路を1個持つ。
  // 採用時はapply_moveがnext_pathをStateへmoveするため、余計な全体copyはない。
  struct Move {
    vector<int> next_path;
    int next_score = 0;
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
      const int shortest = distance(current, target);
      if (shortest > remaining) return;
      // TODO(AHC002): 1マス最大99点。終点の点は固定部分に含まれるので数えない。
      // グリッドの偶奇で到達歩数の上限を1だけ縮められる場合もある。
      // この上限以下しか取れない枝は、現在のbest_middleを改善できない。
      const int additional_cells = remaining - 1 - ((remaining - shortest) & 1);
      if (score + 99 * additional_cells <= best_score) return;

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

  // TODO(AHC002): 近傍を作る場所。
  // 20%は末尾を作り直し、80%は内部区間をDFSでつなぎ直す。
  optional<Move> propose_move(
      const State& state, mt19937_64& engine, double progress) const {
    if (state.path.size() <= 1) return nullopt;
    Move move;
    const int path_size = static_cast<int>(state.path.size());

    if (path_size < 4 || random_int(engine, 10) < 2) {
      // TODO(AHC002): 序盤ほど大きく壊し、終盤ほど小さくする。
      const int changing_limit = min(
          path_size - 1,
          24 + static_cast<int>((1.0 - progress) * 650.0));
      const int removed = 1 + random_int(engine, changing_limit);
      const int cut = path_size - 1 - removed;

      auto used = state.used_tile;
      for (int index = cut + 1; index < path_size; ++index) {
        used[tile[state.path[index]]] = 0;
      }
      const int noise_width =
          45 + static_cast<int>((1.0 - progress) * 260.0);
      vector<int> tail = grow_tail(
          state.path[cut], used, engine, noise_width);
      move.next_path.assign(
          state.path.begin(), state.path.begin() + cut + 1);
      move.next_path.insert(
          move.next_path.end(), tail.begin(), tail.end());
      move.next_score = state.prefix_score[cut + 1];
      for (int cell : tail) move.next_score += point[cell];
      return move;
    }

    const int max_span = min(
        path_size - 1,
        5 + static_cast<int>((1.0 - progress) * 13.0));
    const int span = 2 + random_int(engine, max(1, max_span - 1));
    const int left = random_int(engine, path_size - span);
    const int right = left + span;

    auto used = state.used_tile;
    vector<int> old_middle;
    int old_middle_score = 0;
    for (int index = left + 1; index < right; ++index) {
      const int cell = state.path[index];
      used[tile[cell]] = 0;
      old_middle.push_back(cell);
      old_middle_score += point[cell];
    }

    const int extra_edges = 2 + static_cast<int>((1.0 - progress) * 8.0);
    const int edge_limit = min(30, span + extra_edges);
    const int expansion_limit =
        220 + static_cast<int>((1.0 - progress) * 650.0);
    SegmentRepair repair(
        *this, engine, used, state.path[right], edge_limit, expansion_limit,
        old_middle_score, std::move(old_middle));
    repair.search(state.path[left], 0, 0);

    move.next_path.assign(
        state.path.begin(), state.path.begin() + left + 1);
    move.next_path.insert(
        move.next_path.end(),
        repair.best_middle.begin(), repair.best_middle.end());
    move.next_path.insert(
        move.next_path.end(), state.path.begin() + right, state.path.end());
    move.next_score =
        state.score - old_middle_score + repair.best_score;
    return move;
  }

  // TODO(AHC002): Runnerへは改善量を返す。経路全体の再計算は不要。
  optional<Score> evaluate_move(const State& state, const Move& move,
                                double /* threshold */) const {
    // TODO: 差分は既にO(1)で分かるので、閾値を使わず正確な改善量を返す。
    return move.next_score - state.score;
  }

  // 採用候補だけを反映する。next_pathのbuffer所有権をStateへ移す。
  void apply_move(State& state, Move& move) const {
    state.path = std::move(move.next_path);
    rebuild_cache(state);
    assert(state.score == move.next_score);
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

  TimeBasedAnnealingRunner<TilePathProblem> runner(
      problem, std::move(initial), initial_score,
      annealing_ms,
      2400.0, 8.0,  // TODO(AHC002): 開始温度、終了温度。
      problem.seed ^ 0x123456789abcdef0ULL,
      16);  // TODO(AHC002): 時計確認間隔。近傍が重いので小さめにする。

  while (runner.step()) {
    // destroy後の低得点領域へ深く潜った時は、最良経路から再出発する。
    if ((runner.iterations() & 511ULL) == 0 &&
        runner.current_score() + 6000 < runner.best_score()) {
      runner.restart_from_best();
    }
  }

  assert(problem.is_valid(runner.best_state()));
  problem.print_answer(runner.best_state());
  return 0;
}
