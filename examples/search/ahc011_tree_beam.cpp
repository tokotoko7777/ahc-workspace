// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;

// 提出時は次の3行を、それぞれのhpp全文へ置き換える。
#include "../../library/batched-timer.hpp"
#include "../../library/fixed-vector.hpp"
#include "../../library/tree-beam-search.hpp"

// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc011_tree_beam.cpp
// Official problem: https://atcoder.jp/contests/ahc011/tasks/ahc011_a

constexpr int MAX_N = 10;
constexpr int MAX_CELLS = MAX_N * MAX_N;
constexpr array<int, 4> DR = {-1, 1, 0, 0};
constexpr array<int, 4> DC = {0, 0, -1, 1};
constexpr array<int, 4> OPPOSITE = {1, 0, 3, 2};
constexpr array<char, 4> COMMAND = {'U', 'D', 'L', 'R'};

#ifndef AHC011_SEARCH_TIME_MS
#define AHC011_SEARCH_TIME_MS 2500.0
#endif

#ifndef AHC011_BEAM_WIDTH
#define AHC011_BEAM_WIDTH 0
#endif

struct SlidingTreeProblem {
  struct State {
    // TODO(AHC011): DFS中に1個だけ持つ可変状態。
    array<unsigned char, MAX_CELLS> board{};
    uint64_t hash = 0;
    long long priority = 0;
    int largest_tree = 0;
    int empty_cell = 0;
    int previous_direction = -1;
  };

  struct Move {
    // TODO(AHC011): 次のスライド1手。
    int direction = 0;

    // apply時に書き、revertで使うundo情報。
    long long old_priority = 0;
    int old_largest_tree = 0;
    int old_previous_direction = -1;
  };

  using Score = long long;

  struct UnionFind {
    array<int, MAX_CELLS> parent{};

    UnionFind() { parent.fill(-1); }

    int root(int vertex) {
      while (parent[vertex] >= 0) {
        if (parent[parent[vertex]] >= 0) {
          parent[vertex] = parent[parent[vertex]];
        }
        vertex = parent[vertex];
      }
      return vertex;
    }

    int size(int vertex) { return -parent[root(vertex)]; }

    int unite(int first, int second) {
      first = root(first);
      second = root(second);
      if (first == second) return first;
      if (parent[first] > parent[second]) swap(first, second);
      parent[first] += parent[second];
      parent[second] = first;
      return first;
    }
  };

  struct Evaluation {
    long long priority = 0;
    int largest_tree = 0;
  };

  int n = 0;
  int turn_limit = 0;
  int cell_count = 0;
  State initial;
  array<array<uint64_t, 16>, MAX_CELLS> zobrist{};

  static int hexadecimal_value(char character) {
    if ('0' <= character && character <= '9') return character - '0';
    return character - 'a' + 10;
  }

  static uint64_t splitmix64(uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
  }

  bool inside(int row, int column) const {
    return 0 <= row && row < n && 0 <= column && column < n;
  }

  uint64_t board_hash(
      const array<unsigned char, MAX_CELLS>& board) const {
    uint64_t result = 0;
    for (int cell = 0; cell < cell_count; ++cell) {
      result ^= zobrist[cell][board[cell]];
    }
    return result;
  }

  void read_input() {
    cin >> n >> turn_limit;
    cell_count = n * n;
    for (int cell = 0; cell < cell_count; ++cell) {
      for (int tile = 0; tile < 16; ++tile) {
        zobrist[cell][tile] = splitmix64(
            static_cast<uint64_t>(cell * 16 + tile + 1));
      }
    }

    for (int row = 0; row < n; ++row) {
      string line;
      cin >> line;
      for (int column = 0; column < n; ++column) {
        const int cell = row * n + column;
        initial.board[cell] = static_cast<unsigned char>(
            hexadecimal_value(line[column]));
        if (initial.board[cell] == 0) initial.empty_cell = cell;
      }
    }
    initial.hash = board_hash(initial.board);
    const Evaluation evaluation = evaluate_board(initial.board);
    initial.priority = evaluation.priority;
    initial.largest_tree = evaluation.largest_tree;
  }

  // 公式と同じく、閉路を含まない連結成分だけを木として数える。
  Evaluation evaluate_board(
      const array<unsigned char, MAX_CELLS>& board) const {
    UnionFind union_find;
    array<unsigned char, MAX_CELLS> cyclic{};
    int matched_edges = 0;

    const auto add_edge = [&](int first, int second) {
      ++matched_edges;
      const int first_root = union_find.root(first);
      const int second_root = union_find.root(second);
      if (first_root == second_root) {
        cyclic[first_root] = 1;
        return;
      }
      const bool has_cycle = cyclic[first_root] || cyclic[second_root];
      const int root = union_find.unite(first_root, second_root);
      cyclic[root] = static_cast<unsigned char>(has_cycle);
    };

    for (int row = 0; row < n; ++row) {
      for (int column = 0; column < n; ++column) {
        const int cell = row * n + column;
        const int tile = board[cell];
        if (tile == 0) continue;
        if (column + 1 < n && (tile & 4) != 0 &&
            (board[cell + 1] & 1) != 0) {
          add_edge(cell, cell + 1);
        }
        if (row + 1 < n && (tile & 8) != 0 &&
            (board[cell + n] & 2) != 0) {
          add_edge(cell, cell + n);
        }
      }
    }

    int tree_square_sum = 0;
    int cyclic_square_sum = 0;
    Evaluation result;
    for (int cell = 0; cell < cell_count; ++cell) {
      if (board[cell] == 0 || union_find.root(cell) != cell) continue;
      const int component_size = union_find.size(cell);
      const int square = component_size * component_size;
      if (cyclic[cell]) {
        cyclic_square_sum += square;
      } else {
        result.largest_tree = max(result.largest_tree, component_size);
        tree_square_sum += square;
      }
    }

    // TODO(AHC011): ビームに残す順位値。公式scoreだけでは同点が多いので、
    // 一致辺、木成分の二乗和、閉路罰則で完成途中の局面を区別する。
    result.priority =
        800LL * matched_edges + 15LL * tree_square_sum -
        30LL * cyclic_square_sum + 1500LL * result.largest_tree;
    return result;
  }

  // TODO(AHC011): 現在の空きマスから可能な最大3手を返す。
  // FixedVectorなので、各State展開時のheap確保は起きない。
  FixedVector<Move, 4> generate_moves(const State& state) const {
    FixedVector<Move, 4> moves;
    const int row = state.empty_cell / n;
    const int column = state.empty_cell % n;
    for (int direction = 0; direction < 4; ++direction) {
      if (state.previous_direction != -1 &&
          direction == OPPOSITE[state.previous_direction]) {
        continue;
      }
      if (inside(row + DR[direction], column + DC[direction])) {
        moves.push_back(Move{direction});
      }
    }
    return moves;
  }

  // TODO(AHC011): スライド、hash、評価cacheを1手だけ進める。
  void apply_move(State& state, Move& move) const {
    move.old_priority = state.priority;
    move.old_largest_tree = state.largest_tree;
    move.old_previous_direction = state.previous_direction;

    const int old_empty = state.empty_cell;
    const int old_row = old_empty / n;
    const int old_column = old_empty % n;
    const int next_empty =
        (old_row + DR[move.direction]) * n +
        old_column + DC[move.direction];
    const int moved_tile = state.board[next_empty];
    state.hash ^= zobrist[old_empty][0] ^ zobrist[next_empty][moved_tile] ^
                  zobrist[old_empty][moved_tile] ^ zobrist[next_empty][0];
    swap(state.board[old_empty], state.board[next_empty]);
    state.empty_cell = next_empty;
    state.previous_direction = move.direction;

    const Evaluation evaluation = evaluate_board(state.board);
    state.largest_tree = evaluation.largest_tree;
    state.priority = evaluation.priority +
                     static_cast<long long>(state.hash & 511ULL);
  }

  // TODO(AHC011): apply直前の盤面・hash・全cacheへ完全に戻す。
  void revert_move(State& state, const Move& move) const {
    const int next_empty = state.empty_cell;
    const int next_row = next_empty / n;
    const int next_column = next_empty % n;
    const int old_empty =
        (next_row - DR[move.direction]) * n +
        next_column - DC[move.direction];
    const int moved_tile = state.board[old_empty];
    state.hash ^= zobrist[old_empty][moved_tile] ^ zobrist[next_empty][0] ^
                  zobrist[old_empty][0] ^ zobrist[next_empty][moved_tile];
    swap(state.board[old_empty], state.board[next_empty]);
    state.empty_cell = old_empty;
    state.previous_direction = move.old_previous_direction;
    state.priority = move.old_priority;
    state.largest_tree = move.old_largest_tree;
  }

  Score evaluate(const State& state) const { return state.priority; }

  // TODO(AHC011): 同じ深さの同一盤面を1個にまとめる。
  // 直前の逆操作は元の親盤面へ戻るだけなので、keyは盤面hashだけでよい。
  uint64_t make_key(const State& state) const {
    return state.hash;
  }

  bool is_complete(const State& state) const {
    return state.largest_tree == cell_count - 1;
  }

  int official_score(int largest_tree, int move_count) const {
    if (largest_tree < cell_count - 1) {
      return static_cast<int>(llround(
          500000.0 * largest_tree / (cell_count - 1)));
    }
    return static_cast<int>(llround(
        500000.0 * (2.0 - static_cast<double>(move_count) / turn_limit)));
  }

  void validate_or_throw(
      const vector<Move>& answer, int expected_largest_tree) const {
    if (static_cast<int>(answer.size()) > turn_limit) {
      throw runtime_error("AHC011: answer exceeded the turn limit");
    }
    State state = initial;
    for (Move move : answer) {
      const auto legal_moves = generate_moves(state);
      bool legal = false;
      for (const Move& candidate : legal_moves) {
        if (candidate.direction == move.direction) legal = true;
      }
      if (!legal) throw runtime_error("AHC011: illegal slide in answer");
      apply_move(state, move);
    }
    const Evaluation full = evaluate_board(state.board);
    if (full.largest_tree != expected_largest_tree ||
        state.hash != board_hash(state.board)) {
      throw runtime_error("AHC011: restored answer does not match score");
    }
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  SlidingTreeProblem problem;
  problem.read_input();
  const int automatic_width = max(900, 4200 - 350 * (problem.n - 6));
  const int beam_width =
      AHC011_BEAM_WIDTH > 0 ? AHC011_BEAM_WIDTH : automatic_width;

  TreeBeamRunner<SlidingTreeProblem> beam(
      problem, problem.initial, problem.initial.priority, beam_width);
  beam.reserve_nodes(
      1U + static_cast<size_t>(beam_width) *
               static_cast<size_t>(min(problem.turn_limit, 300)));
  beam.reserve_candidates(static_cast<size_t>(beam_width) * 3U);

  vector<SlidingTreeProblem::Move> best_answer;
  vector<SlidingTreeProblem::Move> candidate_answer;
  int best_largest_tree = problem.initial.largest_tree;
  BatchedTimer timer(AHC011_SEARCH_TIME_MS, 1);

  while (beam.depth() < problem.turn_limit && !timer.is_over()) {
    bool found_complete = false;
    const bool advanced = beam.step_with_key_and_observe(
        [&](int parent_rank,
            const SlidingTreeProblem::Move& move,
            const SlidingTreeProblem::State& state,
            long long) {
          if (state.largest_tree <= best_largest_tree) return;
          best_largest_tree = state.largest_tree;
          beam.restore_candidate(parent_rank, move, candidate_answer);
          best_answer = candidate_answer;
          found_complete = problem.is_complete(state);
        });
    if (!advanced || found_complete) break;
  }

  problem.validate_or_throw(best_answer, best_largest_tree);
#ifdef LOCAL
  cerr << "score="
       << problem.official_score(
              best_largest_tree, static_cast<int>(best_answer.size()))
       << " depth=" << beam.depth()
       << " generated=" << beam.last_generated_count() << '\n';
#endif
  for (const SlidingTreeProblem::Move& move : best_answer) {
    cout << COMMAND[move.direction];
  }
  cout << '\n';
}
