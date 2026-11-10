// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../library/tree-beam-search.hpp"
#include "../../library/radix-heap.hpp"

// AHC021 "Pyramid Sorting" の公式入力分布と公式得点を使う。
// https://atcoder.jp/contests/ahc021/tasks/ahc021_a
//
// 小さい番号から「親が全て確定済みのマス」へ運ぶ。
// 同じ距離なら大きい球を下へ押し下げる経路を高く評価し、
// 次の球をどの前線マスへ運ぶかをビームで比較する。
//
// 【問題に合わせて書き換える場所】
//   PyramidProblem の State / Move と5関数。
//   generate_moves / apply_move / revert_move / evaluate / make_key
//
// 【ライブラリが担当する場所】
//   Stateは1個だけ保持する。履歴木のDFS、apply/revertの呼び分け、
//   key重複除去、上位N個の選択、465世代のループはTreeBeamRunnerが行う。

constexpr int N = 30;
constexpr int CELL_COUNT = N * (N + 1) / 2;
constexpr int MAX_OPERATIONS = 10000;

struct PyramidProblem {
  // TODO: 【問題ごと】1手とundoに必要な情報をMoveへ書く。
  struct Move {
    // path[0]に対象の小さい球がいる。隣へ順番にswapし、
    // path.back()を今回の確定マスにする。
    std::vector<std::uint16_t> path;
    int rank_cost = 0;
  };

  // TODO: 【問題ごと】現在状態と、差分更新するscore・hash・cacheを書く。
  struct State {
    std::array<std::uint16_t, CELL_COUNT> value{};
    std::array<std::uint16_t, CELL_COUNT> position_of_value{};
    std::array<std::uint8_t, CELL_COUNT> fixed{};
    std::array<std::uint32_t, N> frontier{}; // 親が全て確定済みの未確定マス。
    int next_value = 0;
    int operations = 0;
    long long rank_cost = 0;
    std::uint64_t hash = 0;
  };

  // TODO: 【問題ごと】候補順位の型を選ぶ。この例は小さい方が良い。
  using Score = long long;

  // TODO: 【問題ごと】全Stateで共通の入力・隣接表・事前計算を置く。
  std::array<int, CELL_COUNT> row{};
  std::array<int, CELL_COUNT> column{};
  std::array<std::vector<int>, CELL_COUNT> adjacent;
  // 探索用の作業buffer。Stateの一部ではなく、各候補生成でclearして容量を再利用。
  mutable RadixHeap<int> path_queue;

  PyramidProblem() {
    for (int r = 0; r < N; ++r) {
      for (int c = 0; c <= r; ++c) {
        const int vertex = id(r, c);
        row[vertex] = r;
        column[vertex] = c;
      }
    }
    for (int r = 0; r < N; ++r) {
      for (int c = 0; c <= r; ++c) {
        const int vertex = id(r, c);
        const auto add = [&](int next_row, int next_column) {
          if (next_row < 0 || next_row >= N || next_column < 0 ||
              next_column > next_row) {
            return;
          }
          adjacent[vertex].push_back(id(next_row, next_column));
        };
        add(r, c - 1);
        add(r, c + 1);
        add(r - 1, c - 1);
        add(r - 1, c);
        add(r + 1, c);
        add(r + 1, c + 1);
      }
    }
  }

  static int id(int row, int column) {
    return row * (row + 1) / 2 + column;
  }

  static std::uint64_t hash_token(int position, int value) {
    std::uint64_t x = static_cast<std::uint64_t>(position) * CELL_COUNT +
                      value + 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
  }

  // TODO: 【問題ごと】必ず合法な初期Stateを作る。
  State initial_state(const std::array<int, CELL_COUNT>& permutation) const {
    State state;
    state.frontier[0] = 1;
    for (int position = 0; position < CELL_COUNT; ++position) {
      const int value = permutation[position];
      state.value[position] = static_cast<std::uint16_t>(value);
      state.position_of_value[value] =
          static_cast<std::uint16_t>(position);
      state.hash ^= hash_token(position, value);
    }
    return state;
  }

  bool parents_are_fixed(const State& state, int vertex) const {
    const int r = row[vertex];
    const int c = column[vertex];
    if (r == 0) return true;
    if (c > 0 && !state.fixed[id(r - 1, c - 1)]) return false;
    if (c < r && !state.fixed[id(r - 1, c)]) return false;
    return true;
  }

  // 小さい球が上へ動く時、交換相手の大きい球が下へ下がるほど嬉しい。
  int edge_cost(const State& state, int from, int to) const {
    if (row[to] < row[from]) return CELL_COUNT - state.value[to];
    return CELL_COUNT;
  }

  void set_frontier(State& state, int vertex, bool present) const {
    const std::uint32_t bit = std::uint32_t{1} << column[vertex];
    if (present) state.frontier[row[vertex]] |= bit;
    else state.frontier[row[vertex]] &= ~bit;
  }

  // TODO: 【問題ごと】現在Stateから試す合法Moveを全て返す。
  std::vector<Move> generate_moves(const State& state) const {
    if (state.next_value == CELL_COUNT) return {};
    const int start = state.position_of_value[state.next_value];
    std::array<int, CELL_COUNT> targets;
    int target_count = 0;
    for (int r = 0; r < N; ++r) {
      std::uint32_t mask = state.frontier[r];
      while (mask) {
        const int c = __builtin_ctz(mask);
        mask &= mask - 1;
        targets[target_count++] = id(r, c);
      }
    }
    if (target_count == 0) throw std::runtime_error("missing pyramid frontier");
    int unsettled_targets = target_count;
    constexpr int INF = std::numeric_limits<int>::max() / 4;
    std::array<int, CELL_COUNT> distance;
    std::array<int, CELL_COUNT> previous;
    distance.fill(INF);
    previous.fill(-1);
    auto& queue = path_queue;
    queue.clear();
    distance[start] = 0;
    // (距離, 頂点番号)の辞書順を整数1個で表し、同距離の経路も従来と同じにする。
    // 辺費用は1以上なので、追加keyは最後にpopしたkey以上になる。
    queue.push(start, start);
    while (!queue.empty()) {
      const auto [packed_distance, vertex] = queue.pop();
      const int current_distance = static_cast<int>(packed_distance / CELL_COUNT);
      if (distance[vertex] != current_distance) continue;
      // 必要な行き先の最短路が全て確定したら打ち切る。非負辺なので近似ではない。
      if ((state.frontier[row[vertex]] >> column[vertex]) & 1U) {
        if (--unsettled_targets == 0) break;
      }
      for (int next : adjacent[vertex]) {
        if (state.fixed[next]) continue;
        const int next_distance =
            current_distance + edge_cost(state, vertex, next);
        if (next_distance < distance[next]) {
          distance[next] = next_distance;
          previous[next] = vertex;
          queue.push(static_cast<std::uint64_t>(next_distance) * CELL_COUNT + next, next);
        }
      }
    }

    std::vector<Move> moves;
    moves.reserve(N);
    for (int index = 0; index < target_count; ++index) {
      const int target = targets[index];
      if (distance[target] == INF) continue;
      Move move;
      move.rank_cost = distance[target];
      // 経路の長さだけ数えて1回で確保。push_backの容量拡張を候補ごとに繰り返さない。
      int path_size = 0;
      for (int vertex = target; vertex != -1; vertex = previous[vertex]) ++path_size;
      move.path.resize(path_size);
      for (int vertex = target, path_index = path_size; vertex != -1; vertex = previous[vertex]) {
        move.path[--path_index] = static_cast<std::uint16_t>(vertex);
      }
      if (move.path.empty() || move.path.front() != start) continue;
      moves.push_back(std::move(move));
    }
    std::sort(moves.begin(), moves.end(), [](const Move& left,
                                              const Move& right) {
      if (left.rank_cost != right.rank_cost) {
        return left.rank_cost < right.rank_cost;
      }
      return left.path.back() < right.path.back();
    });
    return moves;
  }

  void swap_vertices(State& state, int first, int second) const {
    const int first_value = state.value[first];
    const int second_value = state.value[second];
    state.hash ^= hash_token(first, first_value);
    state.hash ^= hash_token(second, second_value);
    state.hash ^= hash_token(first, second_value);
    state.hash ^= hash_token(second, first_value);
    std::swap(state.value[first], state.value[second]);
    state.position_of_value[first_value] =
        static_cast<std::uint16_t>(second);
    state.position_of_value[second_value] =
        static_cast<std::uint16_t>(first);
  }

  // TODO: 【問題ごと】盤面と全cacheをMove 1手分だけ差分更新する。
  void apply_move(State& state, Move& move) const {
    for (std::size_t i = 1; i < move.path.size(); ++i) {
      swap_vertices(state, move.path[i - 1], move.path[i]);
    }
    const int target = move.path.back();
    state.fixed[target] = 1;
    set_frontier(state, target, false);
    if (row[target] + 1 < N) {
      for (int delta = 0; delta < 2; ++delta) {
        const int child = id(row[target] + 1, column[target] + delta);
        if (parents_are_fixed(state, child)) set_frontier(state, child, true);
      }
    }
    ++state.next_value;
    state.operations += static_cast<int>(move.path.size()) - 1;
    state.rank_cost += move.rank_cost;
  }

  // TODO: 【問題ごと】apply直前と完全に同じStateへ戻す。
  void revert_move(State& state, const Move& move) const {
    const int target = move.path.back();
    state.rank_cost -= move.rank_cost;
    state.operations -= static_cast<int>(move.path.size()) - 1;
    --state.next_value;
    if (row[target] + 1 < N) {
      // apply前は親targetが未確定なので、この2マスは必ず前線の外だった。
      set_frontier(state, id(row[target] + 1, column[target]), false);
      set_frontier(state, id(row[target] + 1, column[target] + 1), false);
    }
    set_frontier(state, target, true);
    state.fixed[target] = 0;
    for (std::size_t i = move.path.size(); i > 1; --i) {
      swap_vertices(state, move.path[i - 1], move.path[i - 2]);
    }
  }

  // TODO: 【問題ごと】現在Stateの順位値そのものを返す。差分値ではない。
  Score evaluate(const State& state) const { return state.rank_cost; }
  // TODO: 【必要な問題だけ】同じ未来を持つ局面が同じになるkeyを返す。
  std::uint64_t make_key(const State& state) const { return state.hash; }

  int count_errors(const State& state) const {
    int errors = 0;
    for (int r = 0; r + 1 < N; ++r) {
      for (int c = 0; c <= r; ++c) {
        const int upper = id(r, c);
        errors += state.value[upper] > state.value[id(r + 1, c)];
        errors += state.value[upper] > state.value[id(r + 1, c + 1)];
      }
    }
    return errors;
  }

  int official_score(const State& state) const {
    const int errors = count_errors(state);
    if (errors == 0) return 100000 - 5 * state.operations;
    return 50000 - 50 * errors;
  }

  void validate(const State& state) const {
    if (state.next_value != CELL_COUNT || state.operations > MAX_OPERATIONS ||
        count_errors(state) != 0) {
      throw std::runtime_error("constructed pyramid is not a legal solution");
    }
    std::uint64_t expected_hash = 0;
    for (int position = 0; position < CELL_COUNT; ++position) {
      const int value = state.value[position];
      if (state.position_of_value[value] != position ||
          !state.fixed[position]) {
        throw std::runtime_error("position or fixed cache is inconsistent");
      }
      expected_hash ^= hash_token(position, value);
    }
    if (expected_hash != state.hash) {
      throw std::runtime_error("incremental hash is inconsistent");
    }
    for (std::uint32_t mask : state.frontier) {
      if (mask != 0) throw std::runtime_error("completed pyramid has frontier entries");
    }
  }
};

struct PyramidResult {
  int score = 0;
  int operations = 0;
  double milliseconds = 0.0;
  std::vector<PyramidProblem::Move> answer;
};

#ifndef AHC021_FINAL_SCORE_SELECTION
#define AHC021_FINAL_SCORE_SELECTION 1
#endif

PyramidResult solve_case(const std::array<int, CELL_COUNT>& input,
                         int width) {
  PyramidProblem problem;
  const PyramidProblem::State initial = problem.initial_state(input);
  TreeBeamRunner<PyramidProblem> beam(
      problem, initial, problem.evaluate(initial), width, false);
  beam.reserve_nodes(1 + static_cast<std::size_t>(width) * CELL_COUNT);
  beam.reserve_candidates(static_cast<std::size_t>(width) * N);

  const auto start = std::chrono::steady_clock::now();
  const int advanced = beam.run_with_key(CELL_COUNT);
  if (advanced != CELL_COUNT) {
    throw std::runtime_error("beam stopped before placing every value");
  }

  int answer_rank = 0;
  auto answer_rank_score = beam.best_score();
#if AHC021_FINAL_SCORE_SELECTION
  // TODO(AHC021): 完成解を比較する本来の得点。全マス確定済みなので交換回数が少ない方が良い。
  // 途中評価rank_costの1位が正式scoreでも1位とは限らない。Stateのコピーはせず巡回する。
  int fewest_operations = std::numeric_limits<int>::max();
  beam.for_each_state([&](int rank, const PyramidProblem::State& state) {
    if (state.operations < fewest_operations ||
        (state.operations == fewest_operations && rank < answer_rank)) {
      fewest_operations = state.operations;
      answer_rank = rank;
      answer_rank_score = problem.evaluate(state);
    }
  });
#endif
  const auto finish = std::chrono::steady_clock::now();
  std::vector<PyramidProblem::Move> answer = beam.restore(answer_rank);
  PyramidProblem::State check = initial;
  for (PyramidProblem::Move move : answer) problem.apply_move(check, move);
  problem.validate(check);
  if (problem.evaluate(check) != answer_rank_score) {
    throw std::runtime_error("restored answer and rank score disagree");
  }
  return {problem.official_score(check),
          check.operations,
          std::chrono::duration<double, std::milli>(finish - start).count(),
          std::move(answer)};
}

#ifndef AHC021_BEAM_WIDTH
#define AHC021_BEAM_WIDTH 300 // TODO(AHC021): 提出環境で時間を測り、ビーム幅を調整する。
#endif

// TODO(AHC021): 入出力だけを問題に合わせて書く。ライブラリ本体の変更は不要。
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::array<int, CELL_COUNT> input{};
  for (int& value : input) if (!(std::cin >> value)) return 1;
  auto sorted = input;
  std::sort(sorted.begin(), sorted.end());
  for (int i = 0; i < CELL_COUNT; ++i) if (sorted[i] != i) return 1;
  const auto result = solve_case(input, AHC021_BEAM_WIDTH);
  const PyramidProblem problem;
  std::cout << result.operations << '\n';
  for (const auto& move : result.answer) {
    for (std::size_t i = 1; i < move.path.size(); ++i) {
      const int from = move.path[i - 1], to = move.path[i];
      std::cout << problem.row[from] << ' ' << problem.column[from] << ' '
                << problem.row[to] << ' ' << problem.column[to] << '\n';
    }
  }
  return 0;
}
