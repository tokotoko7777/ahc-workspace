// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>

#include "../../library/cost-tree-beam-search.hpp"

using namespace std;

// AHC038 "Tree Robot Arm" を、世代飛ばし木上ビームで解く完全な例。
//
// ビームの1手は公式出力の1ターンではなく、
// 「ある指を、次に拾う/置くマスまで運ぶ」までの複数ターンです。
// そのため、経過ターン数が候補ごとに違う CostTreeBeamRunner を使います。
//
// 問題に合わせて試行錯誤する場所には TODO(AHC038) を付けました。
// 探索木・上位N個の選抜・状態をコピーしないDFSはhpp側の仕事です。

constexpr int MAX_N = 30;
constexpr int MAX_CELLS = MAX_N * MAX_N;
constexpr int MAX_LEAVES = 14;
constexpr array<int, 4> DR = {0, 1, 0, -1};
constexpr array<int, 4> DC = {1, 0, -1, 0};

struct Cell {
  int row = 0;
  int column = 0;
};

// ============================================================================
// ここから問題ごとの型。
// ============================================================================

struct State {
  // supply: 今は1だが目標は0のマス。空の指ならここから拾える。
  // demand: 今は0だが目標は1のマス。保持中の指ならここへ置ける。
  bitset<MAX_CELLS> supply;
  bitset<MAX_CELLS> demand;
  int supply_count = 0;
  int demand_count = 0;

  int root_row = 0;
  int root_column = 0;
  array<unsigned char, MAX_LEAVES> direction{};
  unsigned int holding_mask = 0;
  uint64_t hash = 0;
};

struct Move {
  // TODO(AHC038): ここが「次に何をするか」の定義。
  // leaf番の指を、cellへdirectionの向きで到着させ、Pを行う。
  int leaf = -1;
  int direction = 0;
  Cell goal_root;
  Cell cell;

  // 公式出力で何ターン掛かるか。CostTreeBeamSearchの世代差になる。
  int advance = 1;

  // 初回apply時に作り、探索木へそのまま保存する公式命令列。
  // 再訪時は同じ列を再生するので、apply/revertが完全に一致する。
  vector<string> commands;
};

struct Answer {
  vector<int> lengths;
  int initial_root_row = 0;
  int initial_root_column = 0;
  vector<Move> moves;

  int turns() const {
    int result = 0;
    for (const Move& move : moves) result += move.advance;
    return result;
  }
};

class Ahc038BeamProblem {
 public:
  // CostTreeBeamRunnerが読む、問題ごとの3型。
  using State = ::State;
  using Move = ::Move;
  using Score = long long;

  void read(istream& input) {
    input >> n_ >> takoyaki_count_ >> vertex_limit_;
    initial_board_.resize(n_);
    target_board_.resize(n_);
    for (string& row : initial_board_) input >> row;
    for (string& row : target_board_) input >> row;

    // TODO(AHC038): 腕の形を変えるならここ。
    // この例は根から長さ1,2,...の葉を生やす星型。
    const int leaf_count = min(vertex_limit_ - 1, n_ / 2);
    lengths_.resize(leaf_count);
    iota(lengths_.begin(), lengths_.end(), 1);
  }

  int leaf_count() const { return static_cast<int>(lengths_.size()); }

  bool inside(int row, int column) const {
    return 0 <= row && row < n_ && 0 <= column && column < n_;
  }

  int cell_id(int row, int column) const { return row * n_ + column; }

  static uint64_t hash_token(int kind, int index) {
    uint64_t value =
        (static_cast<uint64_t>(kind) << 32) ^ static_cast<unsigned int>(index);
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
  }

  uint64_t compute_hash(const State& state) const {
    constexpr int SUPPLY = 1;
    constexpr int DEMAND = 2;
    constexpr int ROOT = 3;
    constexpr int DIRECTION = 4;
    constexpr int HOLDING = 5;
    uint64_t hash = hash_token(
        ROOT, cell_id(state.root_row, state.root_column));
    for (int id = 0; id < n_ * n_; ++id) {
      if (state.supply.test(id)) hash ^= hash_token(SUPPLY, id);
      if (state.demand.test(id)) hash ^= hash_token(DEMAND, id);
    }
    for (int leaf = 0; leaf < leaf_count(); ++leaf) {
      hash ^= hash_token(DIRECTION,
                         4 * leaf + state.direction[leaf]);
      if (is_holding(state, leaf)) hash ^= hash_token(HOLDING, leaf);
    }
    return hash;
  }

  bool is_holding(const State& state, int leaf) const {
    return (state.holding_mask >> leaf) & 1U;
  }

  bool is_supply(const State& state, int row, int column) const {
    return inside(row, column) &&
           state.supply.test(cell_id(row, column));
  }

  bool is_demand(const State& state, int row, int column) const {
    return inside(row, column) &&
           state.demand.test(cell_id(row, column));
  }

  bool can_act(const State& state, int leaf, int row, int column) const {
    return is_holding(state, leaf) ? is_demand(state, row, column)
                                   : is_supply(state, row, column);
  }

  bool finished(const State& state) const {
    return state.supply_count == 0 && state.demand_count == 0 &&
           state.holding_mask == 0;
  }

  Cell leaf_position(const State& state, int leaf) const {
    const int direction = state.direction[leaf];
    return {state.root_row + DR[direction] * lengths_[leaf],
            state.root_column + DC[direction] * lengths_[leaf]};
  }

  State make_initial_state() const {
    State state;
    for (int row = 0; row < n_; ++row) {
      for (int column = 0; column < n_; ++column) {
        if (initial_board_[row][column] == '1' &&
            target_board_[row][column] == '0') {
          state.supply.set(cell_id(row, column));
          ++state.supply_count;
        }
        if (initial_board_[row][column] == '0' &&
            target_board_[row][column] == '1') {
          state.demand.set(cell_id(row, column));
          ++state.demand_count;
        }
      }
    }

    // 全て右向きで始まるため、最初のターンで同時に拾いやすい根を選ぶ。
    int best_count = -1;
    int best_nearest = numeric_limits<int>::max();
    for (int root_row = 0; root_row < n_; ++root_row) {
      for (int root_column = 0; root_column < n_; ++root_column) {
        int count = 0;
        int nearest = 2 * n_;
        for (int length : lengths_) {
          count += is_supply(state, root_row, root_column + length);
        }
        for (int id = 0; id < n_ * n_; ++id) {
          if (!state.supply.test(id)) continue;
          const int row = id / n_;
          const int column = id % n_;
          nearest = min(nearest, abs(root_row - row) +
                                     abs(root_column - column));
        }
        if (count > best_count ||
            (count == best_count && nearest < best_nearest)) {
          best_count = count;
          best_nearest = nearest;
          state.root_row = root_row;
          state.root_column = root_column;
        }
      }
    }
    state.hash = compute_hash(state);
    return state;
  }

  static int rotation_distance(int from, int to) {
    const int clockwise = (to - from + 4) % 4;
    return min(clockwise, 4 - clockwise);
  }

  array<int, MAX_CELLS> make_distance_map(
      const bitset<MAX_CELLS>& targets) const {
    constexpr int INF = 10000;
    array<int, MAX_CELLS> distance;
    distance.fill(INF);
    queue<int> que;
    for (int id = 0; id < n_ * n_; ++id) {
      if (!targets.test(id)) continue;
      distance[id] = 0;
      que.push(id);
    }
    while (!que.empty()) {
      const int id = que.front();
      que.pop();
      const int row = id / n_;
      const int column = id % n_;
      for (int direction = 0; direction < 4; ++direction) {
        const int next_row = row + DR[direction];
        const int next_column = column + DC[direction];
        if (!inside(next_row, next_column)) continue;
        const int next_id = cell_id(next_row, next_column);
        if (distance[next_id] <= distance[id] + 1) continue;
        distance[next_id] = distance[id] + 1;
        que.push(next_id);
      }
    }
    return distance;
  }

  vector<Move> generate_moves(const State& state) const {
    // TODO(AHC038): 分岐候補の作り方。増やすほど強くなりやすいが重くなる。
    constexpr int PER_LEAF = 2;
    constexpr int BRANCH_WIDTH = 16;

    struct RatedMove {
      tuple<int, int, int, int, int, int> key;
      Move move;
    };

    const auto distance_to_supply = make_distance_map(state.supply);
    const auto distance_to_demand = make_distance_map(state.demand);
    vector<RatedMove> candidates;
    candidates.reserve(leaf_count() * PER_LEAF);

    for (int leaf = 0; leaf < leaf_count(); ++leaf) {
      vector<RatedMove> per_leaf;
      const bitset<MAX_CELLS>& targets =
          is_holding(state, leaf) ? state.demand : state.supply;

      for (int id = 0; id < n_ * n_; ++id) {
        if (!targets.test(id)) continue;
        const int row = id / n_;
        const int column = id % n_;
        for (int direction = 0; direction < 4; ++direction) {
          const int goal_root_row =
              row - DR[direction] * lengths_[leaf];
          const int goal_root_column =
              column - DC[direction] * lengths_[leaf];
          if (!inside(goal_root_row, goal_root_column)) continue;

          const int move_turns = abs(state.root_row - goal_root_row) +
                                 abs(state.root_column - goal_root_column);
          const int rotate_turns =
              rotation_distance(state.direction[leaf], direction);
          const int advance = max(1, max(move_turns, rotate_turns));
          const int future_distance =
              is_holding(state, leaf) ? distance_to_supply[id]
                                      : distance_to_demand[id];
          const int load_priority = is_holding(state, leaf) ? 1 : 0;

          Move move;
          move.leaf = leaf;
          move.direction = direction;
          move.goal_root = {goal_root_row, goal_root_column};
          move.cell = {row, column};
          move.advance = advance;
          per_leaf.push_back(
              {{advance, load_priority, future_distance, move_turns,
                id, direction},
               std::move(move)});
        }
      }

      auto better = [](const RatedMove& a, const RatedMove& b) {
        return a.key < b.key;
      };
      if (static_cast<int>(per_leaf.size()) > PER_LEAF) {
        nth_element(per_leaf.begin(), per_leaf.begin() + PER_LEAF,
                    per_leaf.end(), better);
        per_leaf.resize(PER_LEAF);
      }
      move(per_leaf.begin(), per_leaf.end(), back_inserter(candidates));
    }

    auto better = [](const RatedMove& a, const RatedMove& b) {
      return a.key < b.key;
    };
    if (static_cast<int>(candidates.size()) > BRANCH_WIDTH) {
      nth_element(candidates.begin(), candidates.begin() + BRANCH_WIDTH,
                  candidates.end(), better);
      candidates.resize(BRANCH_WIDTH);
    }
    sort(candidates.begin(), candidates.end(), better);

    vector<Move> result;
    result.reserve(candidates.size());
    for (RatedMove& candidate : candidates) {
      result.push_back(std::move(candidate.move));
    }
    return result;
  }

  int distance_at(const array<int, MAX_CELLS>& distance,
                  int row, int column) const {
    if (inside(row, column)) return distance[cell_id(row, column)];
    const int clipped_row = min(n_ - 1, max(0, row));
    const int clipped_column = min(n_ - 1, max(0, column));
    return distance[cell_id(clipped_row, clipped_column)] +
           abs(row - clipped_row) + abs(column - clipped_column);
  }

  int choose_rotation(const State& state, int leaf, int next_root_row,
                      int next_root_column, const Cell& reserved,
                      const array<int, MAX_CELLS>& distance_to_supply,
                      const array<int, MAX_CELLS>& distance_to_demand) const {
    const array<int, MAX_CELLS>& distance =
        is_holding(state, leaf) ? distance_to_demand : distance_to_supply;
    int best_delta = 0;
    tuple<int, int, int> best_key{1, numeric_limits<int>::max(), 1};

    // 同点なら無回転を優先する。L/Rを無駄に往復しにくくなる。
    constexpr array<int, 3> DELTA = {0, -1, 1};
    for (int delta : DELTA) {
      const int next_direction =
          (static_cast<int>(state.direction[leaf]) + delta + 4) % 4;
      const int row = next_root_row + DR[next_direction] * lengths_[leaf];
      const int column =
          next_root_column + DC[next_direction] * lengths_[leaf];
      const bool is_reserved =
          row == reserved.row && column == reserved.column;
      const bool immediate =
          !is_reserved && can_act(state, leaf, row, column);
      const auto key = tuple(!immediate, distance_at(distance, row, column),
                             delta != 0);
      if (key < best_key) {
        best_key = key;
        best_delta = delta;
      }
    }
    return best_delta;
  }

  void apply_command(State& state, const string& command) const {
    constexpr int SUPPLY = 1;
    constexpr int DEMAND = 2;
    constexpr int ROOT = 3;
    constexpr int DIRECTION = 4;
    constexpr int HOLDING = 5;
    const int vertices = leaf_count() + 1;
    const int old_root_id = cell_id(state.root_row, state.root_column);
    if (command[0] == 'U') --state.root_row;
    if (command[0] == 'D') ++state.root_row;
    if (command[0] == 'L') --state.root_column;
    if (command[0] == 'R') ++state.root_column;
    assert(inside(state.root_row, state.root_column));
    const int next_root_id = cell_id(state.root_row, state.root_column);
    if (old_root_id != next_root_id) {
      state.hash ^= hash_token(ROOT, old_root_id);
      state.hash ^= hash_token(ROOT, next_root_id);
    }

    for (int leaf = 0; leaf < leaf_count(); ++leaf) {
      const int old_direction = state.direction[leaf];
      if (command[leaf + 1] == 'L') {
        state.direction[leaf] = static_cast<unsigned char>(
            (static_cast<int>(state.direction[leaf]) + 3) % 4);
      }
      if (command[leaf + 1] == 'R') {
        state.direction[leaf] = static_cast<unsigned char>(
            (static_cast<int>(state.direction[leaf]) + 1) % 4);
      }
      const int next_direction = state.direction[leaf];
      if (old_direction != next_direction) {
        state.hash ^= hash_token(DIRECTION, 4 * leaf + old_direction);
        state.hash ^= hash_token(DIRECTION, 4 * leaf + next_direction);
      }
    }

    for (int leaf = 0; leaf < leaf_count(); ++leaf) {
      if (command[vertices + leaf + 1] != 'P') continue;
      const Cell position = leaf_position(state, leaf);
      assert(inside(position.row, position.column));
      const int id = cell_id(position.row, position.column);
      if (is_holding(state, leaf)) {
        assert(state.demand.test(id));
        state.demand.reset(id);
        --state.demand_count;
        state.hash ^= hash_token(DEMAND, id);
        state.hash ^= hash_token(HOLDING, leaf);
        state.holding_mask ^= 1U << leaf;
      } else {
        assert(state.supply.test(id));
        state.supply.reset(id);
        --state.supply_count;
        state.hash ^= hash_token(SUPPLY, id);
        state.hash ^= hash_token(HOLDING, leaf);
        state.holding_mask ^= 1U << leaf;
      }
    }
#ifndef NDEBUG
    assert(state.hash == compute_hash(state));
#endif
  }

  void apply_move(State& state, Move& move) const {
    // 探索木をたどり直す時は、初回に作った命令をそのまま再生する。
    if (!move.commands.empty()) {
      for (const string& command : move.commands) {
        apply_command(state, command);
      }
      return;
    }

    const auto distance_to_supply = make_distance_map(state.supply);
    const auto distance_to_demand = make_distance_map(state.demand);
    move.commands.reserve(move.advance);

    for (int step = 0; step < move.advance; ++step) {
      char movement = '.';
      int next_root_row = state.root_row;
      int next_root_column = state.root_column;
      if (next_root_row < move.goal_root.row) {
        ++next_root_row;
        movement = 'D';
      } else if (next_root_row > move.goal_root.row) {
        --next_root_row;
        movement = 'U';
      } else if (next_root_column < move.goal_root.column) {
        ++next_root_column;
        movement = 'R';
      } else if (next_root_column > move.goal_root.column) {
        --next_root_column;
        movement = 'L';
      }

      vector<int> rotation_delta(leaf_count());
      const int clockwise =
          (move.direction - static_cast<int>(state.direction[move.leaf]) + 4) %
          4;
      if (clockwise == 1 || clockwise == 2) {
        rotation_delta[move.leaf] = 1;
      } else if (clockwise == 3) {
        rotation_delta[move.leaf] = -1;
      }
      for (int leaf = 0; leaf < leaf_count(); ++leaf) {
        if (leaf == move.leaf) continue;
        rotation_delta[leaf] = choose_rotation(
            state, leaf, next_root_row, next_root_column, move.cell,
            distance_to_supply, distance_to_demand);
      }

      const int vertices = leaf_count() + 1;
      string command(2 * vertices, '.');
      command[0] = movement;
      for (int leaf = 0; leaf < leaf_count(); ++leaf) {
        if (rotation_delta[leaf] == -1) command[leaf + 1] = 'L';
        if (rotation_delta[leaf] == 1) command[leaf + 1] = 'R';
      }

      // Pの候補座標は移動・回転後で計算する。ここでStateをコピーせず、
      // commandへPを書き終えてからapply_commandを1回だけ呼ぶ。
      const bool selected_must_act = step + 1 == move.advance;
      for (int leaf = 0; leaf < leaf_count(); ++leaf) {
        const int next_direction =
            (static_cast<int>(state.direction[leaf]) +
             rotation_delta[leaf] + 4) % 4;
        const Cell position{
            next_root_row + DR[next_direction] * lengths_[leaf],
            next_root_column + DC[next_direction] * lengths_[leaf]};
        const bool selected = leaf == move.leaf && selected_must_act;
        const bool reserved_for_other =
            leaf != move.leaf && position.row == move.cell.row &&
            position.column == move.cell.column;
        const bool opportunistic =
            leaf != move.leaf && !reserved_for_other &&
            can_act(state, leaf, position.row, position.column);
        if (selected || opportunistic) {
          command[vertices + leaf + 1] = 'P';
        }
      }
      apply_command(state, command);
      move.commands.push_back(std::move(command));
    }

    const Cell selected_position = leaf_position(state, move.leaf);
    if (selected_position.row != move.cell.row ||
        selected_position.column != move.cell.column) {
      throw runtime_error("AHC038: selected leaf missed its target");
    }
  }

  void revert_move(State& state, const Move& move) const {
    constexpr int SUPPLY = 1;
    constexpr int DEMAND = 2;
    constexpr int ROOT = 3;
    constexpr int DIRECTION = 4;
    constexpr int HOLDING = 5;
    const int vertices = leaf_count() + 1;
    for (auto it = move.commands.rbegin(); it != move.commands.rend(); ++it) {
      const string& command = *it;

      // 公式順の逆: まずP、次に回転、最後に根移動を戻す。
      for (int leaf = leaf_count() - 1; leaf >= 0; --leaf) {
        if (command[vertices + leaf + 1] != 'P') continue;
        const Cell position = leaf_position(state, leaf);
        assert(inside(position.row, position.column));
        const int id = cell_id(position.row, position.column);
        if (is_holding(state, leaf)) {
          // 順方向はpickupだった。
          assert(!state.supply.test(id));
          state.supply.set(id);
          ++state.supply_count;
          state.hash ^= hash_token(SUPPLY, id);
          state.hash ^= hash_token(HOLDING, leaf);
          state.holding_mask ^= 1U << leaf;
        } else {
          // 順方向はdropだった。
          assert(!state.demand.test(id));
          state.demand.set(id);
          ++state.demand_count;
          state.hash ^= hash_token(DEMAND, id);
          state.hash ^= hash_token(HOLDING, leaf);
          state.holding_mask ^= 1U << leaf;
        }
      }

      for (int leaf = leaf_count() - 1; leaf >= 0; --leaf) {
        const int old_direction = state.direction[leaf];
        if (command[leaf + 1] == 'L') {
          state.direction[leaf] = static_cast<unsigned char>(
              (static_cast<int>(state.direction[leaf]) + 1) % 4);
        }
        if (command[leaf + 1] == 'R') {
          state.direction[leaf] = static_cast<unsigned char>(
              (static_cast<int>(state.direction[leaf]) + 3) % 4);
        }
        const int next_direction = state.direction[leaf];
        if (old_direction != next_direction) {
          state.hash ^= hash_token(DIRECTION, 4 * leaf + old_direction);
          state.hash ^= hash_token(DIRECTION, 4 * leaf + next_direction);
        }
      }
      const int old_root_id = cell_id(state.root_row, state.root_column);
      if (command[0] == 'U') ++state.root_row;
      if (command[0] == 'D') --state.root_row;
      if (command[0] == 'L') ++state.root_column;
      if (command[0] == 'R') --state.root_column;
      const int next_root_id = cell_id(state.root_row, state.root_column);
      if (old_root_id != next_root_id) {
        state.hash ^= hash_token(ROOT, old_root_id);
        state.hash ^= hash_token(ROOT, next_root_id);
      }
#ifndef NDEBUG
      assert(state.hash == compute_hash(state));
#endif
    }
  }

  long long evaluate(const State& state) const {
    // TODO(AHC038): 最重要の評価関数。CostTreeBeamRunnerをminimizeで使う。
    // 同じ公式ターンへ到達した状態同士なら、まずPの実行数を優先する。
    // 同数なら、複数運搬しやすい「保持中の指が多い状態」を少し優先する。
    const long long remaining = state.supply_count + state.demand_count;
    const int holding = __builtin_popcount(state.holding_mask);
    const int center = n_ - 1;
    const int center_distance =
        abs(2 * state.root_row - center) +
        abs(2 * state.root_column - center);
    return remaining * 1'000'000LL - holding * 1'000LL + center_distance;
  }

  // TODO(AHC038): 1回のMoveが公式出力の何ターン分かを返す。
  // 必ず1以上。候補はこの到着ターンごとに別々の上位N件へ絞られる。
  int get_advance(const Move& move) const { return move.advance; }

  // TODO(AHC038): 同じ到着ターンで、以後の候補と評価が等価な局面のkey。
  // State::hashは盤面、根、全指の向き、保持状態をすべて含む。
  uint64_t make_key(const State& state) const { return state.hash; }

  Answer solve(int beam_width, double time_limit_ms) {
    const State initial = make_initial_state();
    Answer best;
    best.lengths = lengths_;
    best.initial_root_row = initial.root_row;
    best.initial_root_column = initial.root_column;

    // 幅1の貪欲解を必ず先に作る。時間切れでも合法な完成解を返せる。
    State greedy_state = initial;
    while (!finished(greedy_state)) {
      vector<Move> moves = generate_moves(greedy_state);
      if (moves.empty()) throw runtime_error("AHC038: no reachable event");
      Move move = std::move(moves.front());
      apply_move(greedy_state, move);
      best.moves.push_back(std::move(move));
      if (best.turns() > 100000) {
        throw runtime_error("AHC038: greedy answer exceeded 100000 turns");
      }
    }

    const int greedy_turns = best.turns();
    if (greedy_turns == 0 || beam_width <= 0 || time_limit_ms <= 0.0) {
      validate_or_throw(best);
      return best;
    }

    CostTreeBeamRunner<Ahc038BeamProblem> beam(
        *this, initial, evaluate(initial), beam_width, greedy_turns, false);
    beam.reserve_nodes(static_cast<size_t>(beam_width) *
                       static_cast<size_t>(2 * initial.supply_count + 4));
    beam.reserve_candidates(static_cast<size_t>(beam_width) * 16U);

    const auto started = chrono::steady_clock::now();
    vector<Move> candidate_path;
    while (beam.generation() < best.turns()) {
      const double elapsed_ms = chrono::duration<double, milli>(
                                    chrono::steady_clock::now() - started)
                                    .count();
      if (elapsed_ms >= time_limit_ms) break;

      const bool advanced = beam.step_with_key_and_observe(
          [&](int parent_rank, const Move& move, const State& state,
              long long, int next_generation) {
            if (!finished(state) || next_generation >= best.turns()) return;
            beam.restore_candidate(parent_rank, move, candidate_path);
            best.moves = candidate_path;
          });
      if (!advanced) break;
    }

    State check = initial;
    for (Move& move : best.moves) apply_move(check, move);
    if (!finished(check)) throw runtime_error("AHC038: restored path is invalid");
    validate_or_throw(best);
    return best;
  }

  // 探索用Stateとは別に、公式仕様どおり盤面を再生して検査する。
  // apply/revertの両方に同じバグがあっても、ここで不正解を検出できる。
  void validate_or_throw(const Answer& answer) const {
    if (answer.lengths != lengths_) {
      throw runtime_error("AHC038: arm shape changed in restored answer");
    }
    vector<string> board = initial_board_;
    vector<unsigned char> direction(leaf_count(), 0);
    vector<unsigned char> holding(leaf_count(), 0);
    int root_row = answer.initial_root_row;
    int root_column = answer.initial_root_column;
    int turns = 0;
    const int vertices = leaf_count() + 1;

    for (const Move& move : answer.moves) {
      if (static_cast<int>(move.commands.size()) != move.advance) {
        throw runtime_error("AHC038: advance and command count differ");
      }
      for (const string& command : move.commands) {
        ++turns;
        if (turns > 100000 ||
            static_cast<int>(command.size()) != 2 * vertices) {
          throw runtime_error("AHC038: invalid command count or length");
        }

        if (command[0] == 'U') --root_row;
        if (command[0] == 'D') ++root_row;
        if (command[0] == 'L') --root_column;
        if (command[0] == 'R') ++root_column;
        if (string(".UDLR").find(command[0]) == string::npos ||
            !inside(root_row, root_column)) {
          throw runtime_error("AHC038: illegal root movement");
        }

        for (int leaf = 0; leaf < leaf_count(); ++leaf) {
          const char rotation = command[leaf + 1];
          if (rotation == 'L') direction[leaf] =
              static_cast<unsigned char>((direction[leaf] + 3) % 4);
          if (rotation == 'R') direction[leaf] =
              static_cast<unsigned char>((direction[leaf] + 1) % 4);
          if (string(".LR").find(rotation) == string::npos) {
            throw runtime_error("AHC038: illegal rotation character");
          }
        }
        if (command[vertices] != '.') {
          throw runtime_error("AHC038: root cannot pick or release");
        }

        for (int leaf = 0; leaf < leaf_count(); ++leaf) {
          const char action = command[vertices + leaf + 1];
          if (string(".P").find(action) == string::npos) {
            throw runtime_error("AHC038: illegal action character");
          }
          if (action != 'P') continue;
          const int row = root_row + DR[direction[leaf]] * lengths_[leaf];
          const int column =
              root_column + DC[direction[leaf]] * lengths_[leaf];
          if (!inside(row, column)) {
            throw runtime_error("AHC038: P outside the board");
          }
          if (holding[leaf]) {
            if (board[row][column] != '0') {
              throw runtime_error("AHC038: released onto an occupied cell");
            }
            board[row][column] = '1';
            holding[leaf] = 0;
          } else {
            if (board[row][column] != '1') {
              throw runtime_error("AHC038: picked from an empty cell");
            }
            board[row][column] = '0';
            holding[leaf] = 1;
          }
        }
      }
    }

    if (board != target_board_ ||
        any_of(holding.begin(), holding.end(), [](unsigned char value) {
          return value != 0;
        })) {
      throw runtime_error("AHC038: answer did not reach the target board");
    }
  }

  void print_answer(const Answer& answer, ostream& output) const {
    output << answer.lengths.size() + 1 << '\n';
    for (int length : answer.lengths) output << 0 << ' ' << length << '\n';
    output << answer.initial_root_row << ' ' << answer.initial_root_column
           << '\n';
    for (const Move& move : answer.moves) {
      for (const string& command : move.commands) output << command << '\n';
    }
  }

 private:
  int n_ = 0;
  int takoyaki_count_ = 0;
  int vertex_limit_ = 0;
  vector<string> initial_board_;
  vector<string> target_board_;
  vector<int> lengths_;
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  Ahc038BeamProblem problem;
  problem.read(cin);

  // TODO(AHC038): 最初に触る探索パラメータ。幅を増やすほど重くなる。
#ifndef AHC038_BEAM_WIDTH
#define AHC038_BEAM_WIDTH 7
#endif
#ifndef AHC038_SEARCH_TIME_MS
#define AHC038_SEARCH_TIME_MS 2600.0
#endif
  constexpr int BEAM_WIDTH = AHC038_BEAM_WIDTH;
  constexpr double SEARCH_TIME_MS = AHC038_SEARCH_TIME_MS;
  const Answer answer = problem.solve(BEAM_WIDTH, SEARCH_TIME_MS);
  problem.print_answer(answer, cout);
}
