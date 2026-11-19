// AHC_SOURCE: template/search/beam/simple.cpp
// Source repository: https://github.com/tokotoko7777/ahc-workspace
// 子状態をコピーして作るビーム。TODOの型・宣言を実装して使う。
#include <iostream>
#include <vector>
#include "library/best-keeper.hpp"
#include "library/simple-beam-search.hpp"
#include "library/timer.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO
constexpr int BEAM_WIDTH = 100;           // TODO

struct Problem {
    using Score = long long;  // この雛形では順位・最終得点とも最大化。
    struct State {
        // TODO: 盤面、手数、途中得点、出力に必要な情報（操作履歴等）。
    };
    // TODO: 固定入力はここへ。以下の宣言に実装を書く。
    void read_input();
    State make_initial_state() const;  // 構築開始時の部分解。
    State make_fallback() const;       // 時間切れ時にも出力できる合法な完成解。
    bool is_complete(const State& state) const;
    Score final_score(const State& complete) const;
    void print_answer(const State& complete) const;

    // TODO: 合法な子Stateを全て返す。完成状態はこの雛形側で展開を止める。
    std::vector<State> generate_children(const State& state) const;
    // TODO: 子の順位値そのものを返す。親との差分ではない。
    Score rank_score(const State& state) const;
};

int main() {
    Timer timer;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    Problem problem;
    problem.read_input();
    auto fallback = problem.make_fallback();
    BestKeeper<Problem::Score, Problem::State> best(problem.final_score(fallback), fallback);
    const auto save_complete = [&](const Problem::State& state) {
        if (problem.is_complete(state)) best.update(problem.final_score(state), state);
    };
    auto initial = problem.make_initial_state();
    save_complete(initial);
    SimpleBeamSearch<Problem::State, Problem::Score> beam(initial, BEAM_WIDTH);

    while (!timer.is_over(TIME_LIMIT_MS)) {
        const bool advanced = beam.step_and_observe(
            [&](const Problem::State& state) {
                if (problem.is_complete(state)) return std::vector<Problem::State>{};
                return problem.generate_children(state);
            },
            [&](const Problem::State& state) { return problem.rank_score(state); },
            [&](const Problem::State& child, const Problem::Score&) { save_complete(child); });
        if (!advanced) break;
    }
    // 幅から落ちる完成解もobserverで保存。beam.best()は未完成のことがある。
    problem.print_answer(best.best_state);
}
