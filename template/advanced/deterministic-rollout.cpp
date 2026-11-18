// 候補を仮実行して最初の1手だけを採用。TODOの型・関数宣言を実装する。
#include <iostream>
#include <vector>
#include "library/deterministic-rollout.hpp"
#include "library/timer.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO

struct Problem {
    using Score = long long;  // 仮実行結果の絶対評価値。大きいほど良い。
    struct State {
        // TODO: 現在の実状態、出力に必要な情報（採用した操作列等）。
    };
    struct Action {
        // TODO: 次の1手。
    };
    const Timer& timer;
    // TODO: 固定入力、入出力、終端判定。
    void read_input();
    State make_initial_state() const;
    State make_fallback() const;  // 途中で止まった時に出力できる合法な完成解。
    bool is_terminal(const State& state) const;
    void apply_action(State& state, const Action& action) const;
    void print_answer(const State& complete) const;

    // TODO: 比較する合法手。非終端では必ず1個以上返す（空だとRunnerは例外）。
    std::vector<Action> generate_actions(const State& state) const;
    // TODO: stateをコピー→actionを適用→残りを貪欲等で仮実行→評価値を返す。
    // 元のstateは変更しない。重いループではtimerを確認し、打切り評価を返す。
    // Runnerは時間管理しないため、1回のchoose_actionを有界時間に収める。
    Score evaluate_action(const State& state, const Action& action) const;
};

int main() {
    Timer timer;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    Problem problem{timer};
    problem.read_input();
    auto fallback = problem.make_fallback();
    auto state = problem.make_initial_state();
    DeterministicRolloutRunner<Problem> search(problem);
    while (!problem.is_terminal(state) && !timer.is_over(TIME_LIMIT_MS)) {
        const auto action = search.choose_action(state);
        problem.apply_action(state, action);  // 実状態に反映するのはこの1手だけ。
    }
    problem.print_answer(problem.is_terminal(state) ? state : fallback);
}
