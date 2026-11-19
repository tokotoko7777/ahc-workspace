// AHC_SOURCE: template/search/local-search/basic.cpp
// Source repository: https://github.com/tokotoko7777/ahc-workspace
// 差分評価型の焼きなまし / 山登り。TODOを実装して自分のmain.cppにする。
// 関数の宣言は意図的に未定義。手順・他の雛形: template/README.md
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include "library/time-based-simulated-annealing.hpp"
#include "library/timer.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO: 入力・初期化を含む予算。

struct Problem {
    using Score = long long;  // TODO: 大きいほど良い。コスト最小化なら負のコスト。
    struct State {
        // TODO: 現在解と差分更新用cache。固定入力はProblem側に持つ。
    };
    struct Move {
        // TODO: 変更位置、新しい値、採用時に必要な情報。State丸ごとは不要。
    };

    // TODO: 入出力、合法な完成初期解、全再計算による絶対スコア。
    void read_input();
    State make_initial_state() const;
    Score full_score(const State& state) const;
    void print_answer(const State& answer) const;

    // TODO: 近傍を1個作る。作れない試行はnullopt。stateを変更しない。
    std::optional<Move> propose_move(
        const State& state, std::mt19937_64& rng, double progress) const;

    // TODO: 変更後score - 変更前score。絶対スコアではない。
    // stateを変更しない。不合法ならnullopt。まずthresholdは無視してよい。
    // 最適化時だけ「最終改善量 <= threshold」が証明できたらnulloptで打ち切る。
    std::optional<Score> evaluate_move(
        const State& state, const Move& move, double threshold) const;

    // TODO: 採用された手だけ反映する。得点・hash・cacheも更新する。
    // evaluate_moveがstateを変えないため、不採用時の巻き戻しは不要。
    void apply_move(State& state, Move& move) const;
};

int main() {
    Timer total_timer;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    Problem problem;
    problem.read_input();
    auto initial = problem.make_initial_state();
    const auto initial_score = problem.full_score(initial);
    const double remaining = total_timer.remaining_ms(TIME_LIMIT_MS);
    if (remaining <= 0.0) {
        problem.print_answer(initial);
        return 0;
    }

    TimeBasedAnnealingRunner<Problem> runner(
        problem, initial, initial_score, remaining,
        100.0, 1.0,  // TODO: 開始温度・終了温度。改善量と同じ尺度。
        0, 1);      // seed、時計確認間隔。軽い近傍なら間隔を64等に増やせる。
    runner.run();
    // 山登り: 上の1行を runner.run_hill_climbing(); に変更（改善量>0だけ採用）。
    // 評価の途中打ち切り: runner.run_with_threshold(); に変更。
    problem.print_answer(runner.best_state());
}
