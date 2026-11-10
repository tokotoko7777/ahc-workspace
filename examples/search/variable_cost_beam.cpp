// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include "../../library/cost-tree-beam-search.hpp"

using namespace std;

/*
締切付き宝集め

N 個の場所があり、場所 1 から時刻 0 に出発し、そこの宝を獲得する。
場所 i の宝は初めて訪れた時だけ value[i] 点になる。場所 i から j への移動には
time[i][j] (1, 2, 3 のいずれか) だけ時間がかかる。1 時間待つこともできる。
訪問済みの場所へ再び移動してもよいが、宝は増えない。
時刻 T を超えないように行動し、得点を最大化せよ。

制約:
  1 <= N <= 10, 0 <= T <= 25
  0 <= value[i] <= 10^9
  time[i][i] = 0, i != j なら 1 <= time[i][j] <= 3

入力:
  N T
  value[1] ... value[N]
  time[1][1] ... time[1][N]
  ...
  time[N][1] ... time[N][N]

出力:
  1 行目: 最大得点
  2 行目: 行動数 K
  続く K 行: WAIT、または MOVE j

MOVE j は現在地から場所 j へ移動し、WAIT は 1 時間待つ。出力する行動列は
ちょうど時刻 T まで埋める。

この問題では、同じ時刻の (visited_mask, position) ごとに最高得点だけ残せば
よい。取り得る key は高々 N * 2^N 個なので、beam_width をその値にすると
CostTreeBeamSearch は枝を落とさず、時刻 DP と同じ厳密解になる。
*/

struct Problem {
  int n;
  int deadline;
  vector<long long> value;
  vector<vector<int>> travel_time;
};

struct State {
  int position = 0;
  int elapsed = 0;
  uint32_t visited = 1;
  long long score = 0;
};

struct Move {
  int from;
  int to;  // -1 は WAIT
  int duration;
  long long gain;
  bool first_visit;
};

void apply_move(State& state, const Move& move) {
  state.elapsed += move.duration;
  if (move.to == -1) return;
  state.position = move.to;
  if (move.first_visit) {
    state.visited |= uint32_t{1} << move.to;
    state.score += move.gain;
  }
}

void revert_move(State& state, const Move& move) {
  state.elapsed -= move.duration;
  if (move.to == -1) return;
  if (move.first_visit) {
    state.score -= move.gain;
    state.visited ^= uint32_t{1} << move.to;
  }
  state.position = move.from;
}

struct Answer {
  long long score;
  vector<Move> moves;
};

// 問題固有の型・候補生成・差分更新を1か所へ集める。
struct SearchProblem {
  using State = ::State;
  using Move = ::Move;
  using Score = long long;

  const Problem& input;
  vector<Move> action_buffer;

  explicit SearchProblem(const Problem& problem) : input(problem) {
    action_buffer.reserve(input.n + 1);
  }

  vector<Move>& generate_moves(const State& state) {
    action_buffer.clear();
    if (state.elapsed == input.deadline) return action_buffer;

    action_buffer.push_back(Move{state.position, -1, 1, 0, false});
    for (int next = 0; next < input.n; ++next) {
      if (next == state.position) continue;
      const int duration = input.travel_time[state.position][next];
      if (state.elapsed + duration > input.deadline) continue;
      const bool first_visit = ((state.visited >> next) & 1U) == 0;
      action_buffer.push_back(
          Move{state.position, next, duration,
               first_visit ? input.value[next] : 0, first_visit});
    }
    return action_buffer;
  }

  void apply_move(State& state, Move& move) const {
    ::apply_move(state, move);
  }

  void revert_move(State& state, const Move& move) const {
    ::revert_move(state, move);
  }

  Score evaluate(const State& state) const { return state.score; }

  int get_advance(const Move& move) const { return move.duration; }

  uint64_t make_key(const State& state) const {
    // position は 0..9 なので下位 4 bit で足りる。
    return (static_cast<uint64_t>(state.visited) << 4) |
           static_cast<uint64_t>(state.position);
  }
};

Answer solve_problem(const Problem& problem) {
  State initial;
  initial.score = problem.value[0];

  // 同じ到着時刻に存在できる key の総数以上。N <= 10 なら最大 10240。
  const int beam_width = problem.n * (1 << problem.n);
  SearchProblem search_problem(problem);
  CostTreeBeamRunner<SearchProblem> beam(
      search_problem, initial, initial.score, beam_width, problem.deadline);
  beam.reserve_candidates(
      static_cast<size_t>(beam_width) * (problem.n + 1));
  beam.run_with_key();

  return Answer{beam.best_score(), beam.restore()};
}

bool is_legal_answer(const Problem& problem, const Answer& answer) {
  State state;
  state.score = problem.value[0];
  for (const Move& move : answer.moves) {
    if (move.from != state.position) return false;
    if (move.to == -1) {
      if (move.duration != 1 || move.gain != 0 || move.first_visit) {
        return false;
      }
    } else {
      if (move.to < 0 || problem.n <= move.to) return false;
      if (move.to == state.position) return false;
      if (move.duration !=
          problem.travel_time[state.position][move.to]) {
        return false;
      }
      const bool first_visit = ((state.visited >> move.to) & 1U) == 0;
      if (move.first_visit != first_visit) return false;
      const long long expected_gain =
          first_visit ? problem.value[move.to] : 0;
      if (move.gain != expected_gain) return false;
    }
    if (problem.deadline < state.elapsed + move.duration) return false;
    apply_move(state, move);
  }
  return state.elapsed == problem.deadline && state.score == answer.score;
}

#ifndef VARIABLE_COST_BEAM_SELF_TEST

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  Problem problem;
  cin >> problem.n >> problem.deadline;
  problem.value.resize(problem.n);
  for (long long& value : problem.value) cin >> value;
  problem.travel_time.assign(problem.n, vector<int>(problem.n));
  for (auto& row : problem.travel_time) {
    for (int& duration : row) cin >> duration;
  }

  const Answer answer = solve_problem(problem);
  cout << answer.score << '\n';
  cout << answer.moves.size() << '\n';
  for (const Move& move : answer.moves) {
    if (move.to == -1) {
      cout << "WAIT\n";
    } else {
      cout << "MOVE " << move.to + 1 << '\n';
    }
  }
}

#else

long long exact_score_by_dp(const Problem& problem) {
  const long long unreachable = numeric_limits<long long>::lowest() / 4;
  const int mask_count = 1 << problem.n;
  const int state_count = mask_count * problem.n;
  vector<long long> future(
      static_cast<size_t>(problem.deadline + 1) * state_count,
      unreachable);

  const auto index = [&](int mask, int position) {
    return mask * problem.n + position;
  };
  const auto at = [&](int elapsed, int mask, int position) -> long long& {
    return future[static_cast<size_t>(elapsed) * state_count +
                  index(mask, position)];
  };

  at(0, 1, 0) = problem.value[0];
  for (int elapsed = 0; elapsed <= problem.deadline; ++elapsed) {
    for (int mask = 0; mask < mask_count; ++mask) {
      for (int position = 0; position < problem.n; ++position) {
        const long long score = at(elapsed, mask, position);
        if (score == unreachable) continue;
        if (elapsed < problem.deadline) {
          at(elapsed + 1, mask, position) =
              max(at(elapsed + 1, mask, position), score);
        }
        for (int next = 0; next < problem.n; ++next) {
          if (next == position) continue;
          const int next_time =
              elapsed + problem.travel_time[position][next];
          if (next_time > problem.deadline) continue;
          const bool first_visit = ((mask >> next) & 1) == 0;
          const int next_mask = mask | (1 << next);
          const long long next_score =
              score + (first_visit ? problem.value[next] : 0);
          at(next_time, next_mask, next) =
              max(at(next_time, next_mask, next), next_score);
        }
      }
    }
  }

  long long best = unreachable;
  for (int mask = 0; mask < mask_count; ++mask) {
    for (int position = 0; position < problem.n; ++position) {
      best = max(best, at(problem.deadline, mask, position));
    }
  }
  return best;
}

void run_self_tests() {
  {
    Problem problem{
        4,
        5,
        {0, 9, 12, 20},
        {{0, 1, 2, 3}, {1, 0, 1, 3}, {2, 1, 0, 1}, {3, 3, 1, 0}}};
    const Answer answer = solve_problem(problem);
    assert(is_legal_answer(problem, answer));
    assert(answer.score == exact_score_by_dp(problem));
  }

  // 1 -> 2 -> 1 -> 3 と訪問済みの場所を経由するのが最適。
  // 非三角な移動時間でも再訪を落としていないことを確認する。
  {
    Problem problem{
        3,
        3,
        {0, 10, 20},
        {{0, 1, 1}, {1, 0, 3}, {1, 3, 0}}};
    const Answer answer = solve_problem(problem);
    assert(is_legal_answer(problem, answer));
    assert(answer.score == 30);
    assert(answer.score == exact_score_by_dp(problem));
  }

  // 制約上限でも全 key を残せる幅になっていることを確認する。
  {
    Problem problem;
    problem.n = 10;
    problem.deadline = 25;
    problem.value.resize(problem.n);
    for (int i = 0; i < problem.n; ++i) problem.value[i] = i + 1;
    problem.travel_time.assign(problem.n, vector<int>(problem.n, 1));
    for (int i = 0; i < problem.n; ++i) problem.travel_time[i][i] = 0;
    const Answer answer = solve_problem(problem);
    assert(is_legal_answer(problem, answer));
    assert(answer.score == 55);
    assert(answer.score == exact_score_by_dp(problem));
  }

  mt19937 random(20260831);
  for (int test = 0; test < 200; ++test) {
    Problem problem;
    problem.n = 1 + static_cast<int>(random() % 7);
    problem.deadline = static_cast<int>(random() % 13);
    problem.value.resize(problem.n);
    for (long long& value : problem.value) value = random() % 31;
    problem.travel_time.assign(problem.n, vector<int>(problem.n));
    for (int from = 0; from < problem.n; ++from) {
      for (int to = 0; to < problem.n; ++to) {
        problem.travel_time[from][to] =
            from == to ? 0 : 1 + static_cast<int>(random() % 3);
      }
    }

    const Answer answer = solve_problem(problem);
    assert(is_legal_answer(problem, answer));
    assert(answer.score == exact_score_by_dp(problem));
  }
}

int main() {
  run_self_tests();
  cout << "variable_cost_beam: all tests passed\n";
}

#endif
