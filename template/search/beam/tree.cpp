// 1個のStateをapply/revertで使い回すビーム。TODOの型・宣言を実装する。
#include <iostream>
#include <vector>
#include "library/best-keeper.hpp"
#include "library/timer.hpp"
#include "library/tree-beam-search.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO
constexpr int BEAM_WIDTH = 100;           // TODO

struct Problem {
    using Score = long long;  // 順位・最終得点とも最大化。
    struct State {
        // TODO: 盤面、得点、hash、cache、出力に必要な情報（操作履歴等）。
    };
    struct Move {
        // TODO: 行動 + undo情報。applyで変更前の値を書き込める。
    };
    // TODO: 固定入力、入出力、完成解の判定。
    void read_input();
    State make_initial_state() const;
    State make_fallback() const;  // 合法な完成解。
    bool is_complete(const State& state) const;
    Score final_score(const State& complete) const;
    void print_answer(const State& complete) const;

    // TODO: 合法手を列挙。完成状態からは空を返す。
    std::vector<Move> generate_moves(const State& state) const;
    void apply_move(State& state, Move& move) const;
    // TODO: 盤面・得点・hash・cache・履歴をapply前と完全に同じ値へ戻す。
    void revert_move(State& state, const Move& move) const;
    Score evaluate(const State& state) const;  // 子の順位値そのもの。
    // 重複除去を使うなら make_key(const State&) も定義し、
    // 下をstep_with_key_and_observeに変更。keyが同じ状態はまとめられる。
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
    TreeBeamRunner<Problem> beam(problem, initial, problem.evaluate(initial), BEAM_WIDTH);
    while (!timer.is_over(TIME_LIMIT_MS)) {
        if (!beam.step_and_observe(
                [&](int, const Problem::Move&, const Problem::State& child,
                    const Problem::Score&) { save_complete(child); })) break;
    }
    problem.print_answer(best.best_state);
}

// 操作列だけを保存したい場合はobserver内でbeam.restore_candidate(parent_rank, move)。
// 具体的な復元例: examples/search/ahc011_tree_beam.cpp
