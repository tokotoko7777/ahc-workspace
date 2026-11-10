// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;

// 提出時は次の2行を、それぞれのhpp全文へ置き換える。
#include "../../library/deterministic-rollout.hpp"
#include "../../library/fixed-vector.hpp"

// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc026_deterministic_rollout.cpp
// Official problem: https://atcoder.jp/contests/ahc026/tasks/ahc026_a

// AHC026: 箱を1,2,...の順で運び出す。
// 各段で「近い番号の箱を何番先まで特別扱いするか」を全候補について
// 最後まで仮実行し、消費energyが最小の幅を1段だけ採用する。

struct BoxRolloutProblem {
  struct Plan {
    vector<pair<int, int>> operations;
    int energy = 0;
  };

  // TODO(AHC026): 現在の山と、次に運び出す箱をStateへ置く。
  // location[box]を持つため、目的の箱がある山を毎回全探索しない。
  struct State {
    vector<vector<int>> stacks;
    vector<int> location;
    Plan answer;
    int next_target = 1;
  };

  // TODO(AHC026): 今選ぶ方策パラメータ。
  // target + special_range以下の箱を、大きな塊から1個ずつ分離する。
  using Action = int;

  // TODO(AHC026): 小さい消費energyほど良いのでintを使う。
  using Score = int;

  int n = 0;
  int m = 0;

  State read_initial_state() {
    cin >> n >> m;
    if (n > 200) {
      throw runtime_error("this example supports at most 200 boxes");
    }
    State state;
    state.stacks.assign(m, vector<int>(n / m));
    state.location.assign(n + 1, -1);
    for (int stack_id = 0; stack_id < m; ++stack_id) {
      for (int& box : state.stacks[stack_id]) {
        cin >> box;
        state.location[box] = stack_id;
      }
    }
    return state;
  }

  // TODO(AHC026): 現在段で比較する先読み幅を全て返す。
  // FixedVectorなので、毎段の候補生成ではheap allocationしない。
  FixedVector<Action, 201> generate_actions(const State& state) const {
    FixedVector<Action, 201> actions;
    for (int range = 0; range <= n - state.next_target; ++range) {
      actions.push_back(range);
    }
    return actions;
  }

  int minimum_box(const vector<int>& stack) const {
    if (stack.empty()) return n + 1;
    return *min_element(stack.begin(), stack.end());
  }

  int choose_bulk_destination(
      const vector<vector<int>>& stacks,
      int from) const {
    // TODO(AHC026): 大きな塊の置き先評価。
    // 山の最小番号が最大の場所なら、近い将来に必要な箱を覆いにくい。
    int best_stack = -1;
    int best_minimum = -1;
    for (int to = 0; to < m; ++to) {
      if (to == from) continue;
      const int value = minimum_box(stacks[to]);
      if (value > best_minimum) {
        best_minimum = value;
        best_stack = to;
      }
    }
    return best_stack;
  }

  int choose_small_box_destination(
      const vector<vector<int>>& stacks,
      int from,
      int box) const {
    // TODO(AHC026): 近いうちに使う箱1個の置き先評価。
    // destinationの全箱がbox以上なら、より先に必要な箱を覆わない。
    int best_stack = -1;
    int best_minimum = n + 2;
    for (int to = 0; to < m; ++to) {
      if (to == from) continue;
      const int value = minimum_box(stacks[to]);
      if (value >= box && value < best_minimum) {
        best_minimum = value;
        best_stack = to;
      }
    }
    if (best_stack != -1) return best_stack;
    return choose_bulk_destination(stacks, from);
  }

  void move_suffix(
      vector<vector<int>>& stacks,
      vector<int>& location,
      int from,
      int first_height,
      int to,
      Plan& plan) const {
    const int moved_count =
        static_cast<int>(stacks[from].size()) - first_height;
    plan.energy += moved_count + 1;
    plan.operations.push_back({stacks[from][first_height], to + 1});
    for (int height = first_height;
         height < static_cast<int>(stacks[from].size());
         ++height) {
      location[stacks[from][height]] = to;
    }
    stacks[to].insert(
        stacks[to].end(),
        stacks[from].begin() + first_height,
        stacks[from].end());
    stacks[from].erase(
        stacks[from].begin() + first_height,
        stacks[from].end());
  }

  void remove_one_target(
      vector<vector<int>>& stacks,
      vector<int>& location,
      int target,
      int special_range,
      Plan& plan) const {
    while (true) {
      const int from = location[target];
      const auto iterator = find(
          stacks[from].begin(), stacks[from].end(), target);
      const int target_height = static_cast<int>(
          iterator - stacks[from].begin());
      const int top_height = static_cast<int>(stacks[from].size()) - 1;
      if (target_height == top_height) {
        plan.operations.push_back({target, 0});
        stacks[from].pop_back();
        location[target] = -1;
        return;
      }

      // TODO(AHC026): 1個に分ける箱の条件。
      // 上から探すため、その箱より上を先にまとめて退避できる。
      int special_height = -1;
      for (int height = top_height; height > target_height; --height) {
        if (stacks[from][height] <= target + special_range) {
          special_height = height;
          break;
        }
      }

      if (special_height == -1) {
        move_suffix(
            stacks, location, from, target_height + 1,
            choose_bulk_destination(stacks, from), plan);
        continue;
      }

      if (special_height < top_height) {
        move_suffix(
            stacks, location, from, special_height + 1,
            choose_bulk_destination(stacks, from), plan);
      }

      const int box = stacks[from].back();
      move_suffix(
          stacks, location, from,
          static_cast<int>(stacks[from].size()) - 1,
          choose_small_box_destination(stacks, from, box), plan);
    }
  }

  Plan make_plan(const State& state, int special_range) const {
    vector<vector<int>> stacks = state.stacks;
    vector<int> location = state.location;
    Plan plan;
    for (int target = state.next_target; target <= n; ++target) {
      remove_one_target(
          stacks, location, target, special_range, plan);
    }
    return plan;
  }

  // TODO(AHC026): Actionごとの決定的rollout。
  // 現在Stateを壊さず、最後までの消費energyだけを返す。
  Score evaluate_action(const State& state, const Action& action) const {
    return make_plan(state, action).energy;
  }

  // Runnerが選んだ幅を、本番Stateへ1箱分だけ反映する。
  void apply_action(State& state, const Action& action) const {
    remove_one_target(
        state.stacks, state.location, state.next_target,
        action, state.answer);
    ++state.next_target;
  }
};

// Solverの状態更新とは別の処理で、出力を最初から再生する。
// debug時に「評価は良いが操作が不正」を見逃さないための検査。
void validate_or_throw(
    const vector<vector<int>>& initial_stacks,
    int n,
    int m,
    const BoxRolloutProblem::Plan& answer) {
  if (answer.operations.size() > 5000) {
    throw runtime_error("more than 5000 operations");
  }

  vector<vector<int>> stacks = initial_stacks;
  vector<int> location(n + 1, -1);
  for (int stack_id = 0; stack_id < m; ++stack_id) {
    for (int box : stacks[stack_id]) location[box] = stack_id;
  }

  int next_target = 1;
  int energy = 0;
  for (const auto& [box, destination] : answer.operations) {
    if (box < 1 || box > n || location[box] == -1) {
      throw runtime_error("operation uses a missing box");
    }
    const int from = location[box];
    if (destination == 0) {
      if (box != next_target || stacks[from].back() != box) {
        throw runtime_error("illegal removal");
      }
      stacks[from].pop_back();
      location[box] = -1;
      ++next_target;
      continue;
    }
    const int to = destination - 1;
    if (to < 0 || to >= m || to == from) {
      throw runtime_error("illegal destination");
    }
    const auto iterator = find(
        stacks[from].begin(), stacks[from].end(), box);
    const int first_height = static_cast<int>(
        iterator - stacks[from].begin());
    const int moved_count =
        static_cast<int>(stacks[from].size()) - first_height;
    energy += moved_count + 1;
    for (int height = first_height;
         height < static_cast<int>(stacks[from].size());
         ++height) {
      location[stacks[from][height]] = to;
    }
    stacks[to].insert(
        stacks[to].end(), iterator, stacks[from].end());
    stacks[from].erase(iterator, stacks[from].end());
  }

  if (next_target != n + 1 || energy != answer.energy) {
    throw runtime_error("answer does not match the final state");
  }
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  BoxRolloutProblem problem;
  BoxRolloutProblem::State state = problem.read_initial_state();
#ifdef LOCAL
  const vector<vector<int>> initial_stacks = state.stacks;
#endif

  // AHC026は消費energy最小化なのでmaximize=false。
  DeterministicRolloutRunner<BoxRolloutProblem> rollout(problem, false);
  rollout.reserve(problem.n + 1);
  while (state.next_target <= problem.n) {
    const BoxRolloutProblem::Action action = rollout.choose_action(state);
    problem.apply_action(state, action);
  }

#ifdef LOCAL
  validate_or_throw(
      initial_stacks, problem.n, problem.m, state.answer);
  cerr << "energy = " << state.answer.energy << '\n';
#endif

  for (const auto& [box, destination] : state.answer.operations) {
    cout << box << ' ' << destination << '\n';
  }
}
