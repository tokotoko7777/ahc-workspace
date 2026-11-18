// 1手で進む世代数が異なるビーム。tree.cppとの主な違いはget_advance。
// TODOの型・関数宣言を問題に合わせて実装する。
#include <iostream>
#include <vector>
#include "library/best-keeper.hpp"
#include "library/cost-tree-beam-search.hpp"
#include "library/timer.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO
constexpr int BEAM_WIDTH = 100;           // TODO

struct Problem {
    using Score = long long;
    struct State {
        // TODO: 盤面、消費手数、得点、cache、出力に必要な情報。
    };
    struct Move {
        // TODO: 行動・正の消費手数・undo情報。
    };
    // TODO: 固定入力と入出力。
    void read_input();
    int max_generation() const;  // 探索可能な最大の消費手数。
    State make_initial_state() const;
    State make_fallback() const;  // 合法な完成解。
    bool is_complete(const State& state) const;
    Score final_score(const State& complete) const;
    void print_answer(const State& complete) const;

    // TODO: 完成状態からは空を返す。期限を超える手は生成しない。
    std::vector<Move> generate_moves(const State& state) const;
    void apply_move(State& state, Move& move) const;
    void revert_move(State& state, const Move& move) const;  // cache・履歴も戻す。
    Score evaluate(const State& state) const;               // 子の絶対順位値。
    int get_advance(const Move& move) const;                // 必ず1以上。
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
    CostTreeBeamRunner<Problem> beam(
        problem, initial, problem.evaluate(initial), BEAM_WIDTH, problem.max_generation());
    while (!timer.is_over(TIME_LIMIT_MS)) {
        if (!beam.step_and_observe(
                [&](int, const Problem::Move&, const Problem::State& child,
                    const Problem::Score&, int /* next_generation */) {
                    save_complete(child);
                })) break;
    }
    // 早く完成した候補も保存済み。最後の層だけから選ばない。
    problem.print_answer(best.best_state);
}
