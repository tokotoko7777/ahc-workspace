// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;

// 提出時は次の1行を、このhppの全文へ置き換える。
#include "../../library/action-beam-search.hpp"

// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc032_action_beam.cpp
// Official problem: https://atcoder.jp/contests/ahc032/tasks/ahc032_a

// ============================================================================
// ここから問題ごとに書く部分。定数、State、Action、Problem、出力を含む。
// ============================================================================
constexpr int BOARD_SIZE = 9;
constexpr int STAMP_SIZE = 3;
constexpr int STAMP_COUNT = 20;
constexpr int OPERATION_LIMIT = 81;
constexpr int PLACEMENTS_PER_AXIS = BOARD_SIZE - STAMP_SIZE + 1;
constexpr int PLACEMENT_COUNT =
    PLACEMENTS_PER_AXIS * PLACEMENTS_PER_AXIS;
constexpr uint32_t MODULO = 998244353U;
constexpr long long RANK_SCALE = 4900;

#ifndef AHC032_BEAM_WIDTH
#define AHC032_BEAM_WIDTH 9000
#endif
#ifndef AHC032_END_COMBOS
#define AHC032_END_COMBOS 0
#endif
static_assert(AHC032_END_COMBOS >= 0);
// 5～7枚合成＋最後の7手予約は実験用。開発10ケースで既存配分より悪化した
// ため既定では無効。-DAHC032_END_COMBOS=4096で比較できる。

struct ModStampProblem {
  struct Placement {
    uint8_t row = 0;
    uint8_t column = 0;
    uint8_t max_actions = 0;
    uint8_t cumulative_limit = 0;
  };

  // TODO(AHC032): 探索途中の盤面と、確定済み得点をStateへ置く。
  struct State {
    array<uint32_t, BOARD_SIZE * BOARD_SIZE> board{};
    array<uint16_t, PLACEMENT_COUNT> choices{};
    int position = 0;
    int operations = 0;
    long long finalized_score = 0;
  };

  // TODO(AHC032): Actionは「同じ場所で押すスタンプ多重集合」の番号だけ。
  // 3x3盤面をActionへ持たせず、全候補ぶんの保存量を2 byteに抑える。
  using Action = uint16_t;
  using Score = long long;

  struct Combo {
    array<uint32_t, STAMP_SIZE * STAMP_SIZE> add{};
    array<uint8_t, 7> stamp_ids{};
    uint8_t count = 0;
  };

  array<uint32_t, BOARD_SIZE * BOARD_SIZE> initial_board{};
  array<array<uint32_t, STAMP_SIZE * STAMP_SIZE>, STAMP_COUNT> stamps{};
  array<Placement, PLACEMENT_COUNT> placements{};
  vector<Combo> combinations;
  array<vector<Action>, 8> allowed_actions;

  void read_input() {
    int n, m, k;
    cin >> n >> m >> k;
    if (n != BOARD_SIZE || m != STAMP_COUNT || k != OPERATION_LIMIT) {
      throw runtime_error("this example expects the official AHC032 sizes");
    }
    for (uint32_t& value : initial_board) cin >> value;
    for (auto& stamp : stamps) {
      for (uint32_t& value : stamp) cin >> value;
    }
    prepare();
  }

  // 入力配列を直接用意するテスト・ベンチマークも、同じ前計算を使う。
  void prepare() {
    combinations.clear();
    for (auto& actions : allowed_actions) actions.clear();
    build_placements();
    build_combinations();
  }

  State make_initial_state() const {
    State state;
    state.board = initial_board;
    return state;
  }

  Score initial_score() const {
    return 6LL * MODULO * PLACEMENT_COUNT * OPERATION_LIMIT;
  }

  // TODO(AHC032): 現在位置で合法なAction一覧を返す。
  // Action一覧はProblem側で共有し、親Stateごとのvector確保を避ける。
  const vector<Action>& generate_actions(const State& state) const {
    const Placement& placement = placements[state.position];
    const int remaining_budget =
        static_cast<int>(placement.cumulative_limit) - state.operations;
    const int maximum = max(
        0, min(static_cast<int>(placement.max_actions), remaining_budget));
    return allowed_actions[maximum];
  }

  // TODO(AHC032): Action適用後の順位値そのものを、盤面を作らず計算する。
  // 走査済みの上辺・左辺は以後の3x3スタンプで変わらないので、その値を
  // finalized_scoreへ移す。残り手数の項は、序盤で81手を使い切るのを防ぐ。
  Score evaluate_action(const State& state, const Action& action) const {
    const Placement& placement = placements[state.position];
    const int row = placement.row;
    const int column = placement.column;
    const int finalized_rows =
        row == PLACEMENTS_PER_AXIS - 1 ? STAMP_SIZE : 1;
    const int finalized_columns =
        column == PLACEMENTS_PER_AXIS - 1 ? STAMP_SIZE : 1;
    const Combo& combo = combinations[action];

    Score finalized = state.finalized_score;
    for (int di = 0; di < finalized_rows; ++di) {
      for (int dj = 0; dj < finalized_columns; ++dj) {
        const int board_index = (row + di) * BOARD_SIZE + column + dj;
        const int stamp_index = di * STAMP_SIZE + dj;
        finalized += add_mod(
            state.board[board_index], combo.add[stamp_index]);
      }
    }
    const int next_position = state.position + 1;
    const int next_operations = state.operations + combo.count;
    return finalized * RANK_SCALE +
           6LL * MODULO * (PLACEMENT_COUNT - next_position) *
               (OPERATION_LIMIT - next_operations);
  }

  // TODO(AHC032): 上位N件に残ったActionだけを子Stateへ反映する。
  void apply_action(State& state, Action& action) const {
    const Placement& placement = placements[state.position];
    const int row = placement.row;
    const int column = placement.column;
    const Combo& combo = combinations[action];
    for (int di = 0; di < STAMP_SIZE; ++di) {
      for (int dj = 0; dj < STAMP_SIZE; ++dj) {
        const int board_index = (row + di) * BOARD_SIZE + column + dj;
        const int stamp_index = di * STAMP_SIZE + dj;
        state.board[board_index] = add_mod(
            state.board[board_index], combo.add[stamp_index]);
      }
    }

    const int finalized_rows =
        row == PLACEMENTS_PER_AXIS - 1 ? STAMP_SIZE : 1;
    const int finalized_columns =
        column == PLACEMENTS_PER_AXIS - 1 ? STAMP_SIZE : 1;
    for (int di = 0; di < finalized_rows; ++di) {
      for (int dj = 0; dj < finalized_columns; ++dj) {
        state.finalized_score +=
            state.board[(row + di) * BOARD_SIZE + column + dj];
      }
    }

    state.choices[state.position] = action;
    state.operations += combo.count;
    ++state.position;
  }

  void validate(const State& answer) const {
    if (answer.position != PLACEMENT_COUNT ||
        answer.operations > OPERATION_LIMIT) {
      throw runtime_error("beam did not produce a legal complete answer");
    }
    auto board = initial_board;
    int operations = 0;
    for (int position = 0; position < PLACEMENT_COUNT; ++position) {
      const Placement& placement = placements[position];
      const Combo& combo = combinations[answer.choices[position]];
      operations += combo.count;
      if (combo.count > placement.max_actions ||
          operations > placement.cumulative_limit) {
        throw runtime_error("operation schedule limit exceeded");
      }
      for (int di = 0; di < STAMP_SIZE; ++di) {
        for (int dj = 0; dj < STAMP_SIZE; ++dj) {
          const int board_index =
              (placement.row + di) * BOARD_SIZE + placement.column + dj;
          board[board_index] = add_mod(
              board[board_index], combo.add[di * STAMP_SIZE + dj]);
        }
      }
    }
    long long score = 0;
    for (uint32_t value : board) score += value;
    if (operations != answer.operations || board != answer.board ||
        score != answer.finalized_score) {
      throw runtime_error("incremental state differs from full replay");
    }
  }

 private:
  static uint32_t add_mod(uint32_t left, uint32_t right) {
    uint32_t result = left + right;
    if (result >= MODULO) result -= MODULO;
    return result;
  }

  void add_combination(const array<int, 7>& ids, int count) {
    Combo combo;
    combo.count = static_cast<uint8_t>(count);
    for (int i = 0; i < count; ++i) {
      combo.stamp_ids[i] = static_cast<uint8_t>(ids[i]);
      for (int cell = 0; cell < STAMP_SIZE * STAMP_SIZE; ++cell) {
        combo.add[cell] = add_mod(combo.add[cell], stamps[ids[i]][cell]);
      }
    }
    combinations.push_back(combo);
  }

  void enumerate_combinations(
      int count, int depth, int minimum_stamp, array<int, 7>& ids) {
    if (depth == count) {
      add_combination(ids, count);
      return;
    }
    for (int stamp = minimum_stamp; stamp < STAMP_COUNT; ++stamp) {
      ids[depth] = stamp;
      enumerate_combinations(count, depth + 1, stamp, ids);
    }
  }

  void build_combinations() {
    array<int, 7> ids{};
    for (int count = 0; count <= 4; ++count) {
      enumerate_combinations(count, 0, 0, ids);
    }
    // TODO(AHC032): 最後の3x3だけ、5/6/7押しの候補も試す。
    // (8,8)は最後の(6,6)への配置以外では変わらない。そこで、その1マスが
    // 高くなる合成を各枚数で上位K個残す。これは速度のための近似選抜。
    // 0～4押しも全て残す。最後の7押し用に、build_placementsで手数を予約する。
    // 参考: https://img.atcoder.jp/ahc032/editorial.pdf (スタンプの合成)
    if (AHC032_END_COMBOS > 0) {
      for (int count = 5; count <= 7; ++count) {
        const size_t begin = combinations.size();
        enumerate_combinations(count, 0, 0, ids);
        const auto better = [&](const Combo& a, const Combo& b) {
          const auto av = add_mod(initial_board.back(), a.add.back());
          const auto bv = add_mod(initial_board.back(), b.add.back());
          if (av != bv) return av > bv;
          return a.stamp_ids < b.stamp_ids;
        };
        const size_t keep = min(static_cast<size_t>(AHC032_END_COMBOS),
                                combinations.size() - begin);
        auto first = combinations.begin() + static_cast<ptrdiff_t>(begin);
        auto last = first + static_cast<ptrdiff_t>(keep);
        if (last != combinations.end()) nth_element(first, last, combinations.end(), better);
        sort(first, last, better);
        combinations.resize(begin + keep);
      }
    }
    if (combinations.size() > numeric_limits<Action>::max()) {
      throw runtime_error("too many combinations for uint16_t Action");
    }
    for (int id = 0; id < static_cast<int>(combinations.size()); ++id) {
      for (int limit = combinations[id].count; limit <= 7; ++limit) {
        allowed_actions[limit].push_back(static_cast<Action>(id));
      }
    }
  }

  void build_placements() {
    int position = 0;
    int cumulative_twice = 0;
    const auto add = [&](int row, int column) {
      const bool bottom = row == PLACEMENTS_PER_AXIS - 1;
      const bool right = column == PLACEMENTS_PER_AXIS - 1;
      int maximum = 2;
      int budget_increase_twice = 3;
      if (bottom && right) {
        maximum = AHC032_END_COMBOS > 0 ? 7 : 4;
        budget_increase_twice = 6;
      } else if (bottom || right) {
        maximum = 3;
        budget_increase_twice = 4;
      }
      cumulative_twice += budget_increase_twice;
      // TODO(AHC032): 最後に7手残す。候補だけ増やしても、以前の配分では
      // 残り3～4手の親ばかりになり、5～7押し候補を実際には試せない。
      const int limit = AHC032_END_COMBOS > 0 && !(bottom && right)
          ? min((cumulative_twice + 1) / 2, OPERATION_LIMIT - 7)
          : (cumulative_twice + 1) / 2;
      placements[position++] = {
          static_cast<uint8_t>(row),
          static_cast<uint8_t>(column),
          static_cast<uint8_t>(maximum),
          static_cast<uint8_t>(limit)};
    };

    // 未処理領域の上辺、左辺の順で1層ずつ確定する。
    for (int layer = 0; layer < PLACEMENTS_PER_AXIS; ++layer) {
      for (int column = layer; column < PLACEMENTS_PER_AXIS; ++column) {
        add(layer, column);
      }
      for (int row = layer + 1; row < PLACEMENTS_PER_AXIS; ++row) {
        add(row, layer);
      }
    }
    if (position != PLACEMENT_COUNT ||
        placements.back().cumulative_limit != OPERATION_LIMIT) {
      throw runtime_error("invalid placement schedule");
    }
  }
};

void print_answer(
    const ModStampProblem& problem, const ModStampProblem::State& answer) {
  cout << answer.operations << '\n';
  for (int position = 0; position < PLACEMENT_COUNT; ++position) {
    const ModStampProblem::Placement& placement =
        problem.placements[position];
    const ModStampProblem::Combo& combo =
        problem.combinations[answer.choices[position]];
    for (int i = 0; i < combo.count; ++i) {
      cout << static_cast<int>(combo.stamp_ids[i]) << ' '
           << static_cast<int>(placement.row) << ' '
           << static_cast<int>(placement.column) << '\n';
    }
  }
}

// ============================================================================
// ここから下は探索の呼び出し。候補buffer・上位N件選抜・StateコピーはRunner側。
// ============================================================================
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  ModStampProblem problem;
  problem.read_input();
  ModStampProblem::State initial = problem.make_initial_state();
  ActionBeamRunner<ModStampProblem> beam(
      problem, initial, problem.initial_score(),
      AHC032_BEAM_WIDTH);  // TODO(AHC032): ビーム幅。
  const int advanced = beam.run(PLACEMENT_COUNT);
  if (advanced != PLACEMENT_COUNT) {
    throw runtime_error("beam stopped before the final placement");
  }
  problem.validate(beam.best());
  print_answer(problem, beam.best());
  return 0;
}
