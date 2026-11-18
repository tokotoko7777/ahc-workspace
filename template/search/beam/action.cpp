// 軽いActionで候補を選び、採用分だけStateをコピーするビーム。TODOを実装する。
#include <iostream>
#include <vector>
#include "library/action-beam-search.hpp"
#include "library/best-keeper.hpp"
#include "library/timer.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO
constexpr int BEAM_WIDTH = 100;           // TODO

struct Problem {
    using Score = long long;  // 順位・最終得点とも大きいほど良い。
    struct State {
        // TODO: 盤面、手数、cache、出力に必要な情報（操作履歴等）。
    };
    struct Action {
        // TODO: 変更位置・操作種類など。次状態の全コピーは持たない。
    };
    // TODO: 固定入力と以下の入出力・完成判定。
    void read_input();
    State make_initial_state() const;
    State make_fallback() const;  // 合法な完成解。
    bool is_complete(const State& state) const;
    bool completes(const State& parent, const Action& action) const;
    Score rank_score(const State& state) const;
    Score final_score(const State& complete) const;
    void print_answer(const State& complete) const;

    // TODO: 合法Actionを列挙。完成状態は空を返して展開を止める。
    std::vector<Action> generate_actions(const State& state) const;
    // TODO: 子の順位値そのもの。stateは変更しない（差分だけを返さない）。
    Score evaluate_action(const State& state, const Action& action) const;
    // TODO: コピー済みの子Stateに採用Actionを適用。cache・履歴も更新。
    void apply_action(State& state, Action& action) const;
};

int main() {
    Timer timer;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    Problem problem;
    problem.read_input();
    auto fallback = problem.make_fallback();
    BestKeeper<Problem::Score, Problem::State> best(problem.final_score(fallback), fallback);
    auto initial = problem.make_initial_state();
    if (problem.is_complete(initial)) best.update(problem.final_score(initial), initial);
    ActionBeamRunner<Problem> beam(problem, initial, problem.rank_score(initial), BEAM_WIDTH);

    while (!timer.is_over(TIME_LIMIT_MS)) {
        if (!beam.step_and_observe(
                [&](int, const Problem::State& parent, const Problem::Action& action,
                    const Problem::Score&) {
                    if (!problem.completes(parent, action)) return;
                    // 完成候補は幅から落ちても保存するため、ここでも実体化する。
                    auto child = parent;
                    auto move = action;
                    problem.apply_action(child, move);
                    best.update(problem.final_score(child), child);
                })) break;
    }
    problem.print_answer(best.best_state);
}
