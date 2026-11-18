// 部分破壊→修復（LNS）。TODOを実装して使う、未完成のmain.cpp雛形。
#include <iostream>
#include <optional>
#include <random>
#include "library/large-neighborhood-search.hpp"
#include "library/timer.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO: 出力時間を残す。

struct Problem {
    using Score = long long;
    struct State {
        // TODO: 完成解、修復中の未配置要素、評価用cacheなど。
    };
    const Timer& timer;  // 重いrepairのループ内でもtimer.is_over(TIME_LIMIT_MS)を見る。
    // TODO: 固定入力・前計算はここへ。
    void read_input();
    State make_initial_state() const;  // 常に合法な完成解。
    Score full_score(const State& state) const;
    void print_answer(const State& answer) const;

    // TODO: candidate = current; から一部を壊す。currentは変更しない。
    // candidateには前試行の状態が残るので、毎回必要な情報を上書きする。
    void destroy(const State& current, State& candidate,
                 std::mt19937_64& rng, double progress) const;

    // TODO: 完成・合法な状態へ修復して「絶対スコア」を返す。差分ではない。
    // 修復失敗・時間切れならnulloptで現在解を維持する。
    // thresholdは最初は無視してよい。最大化ではscore>=thresholdが採用条件。
    std::optional<Score> repair(State& candidate, std::mt19937_64& rng,
                                double progress, long double threshold) const;
};

int main() {
    Timer timer;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    Problem problem{timer};
    problem.read_input();
    auto initial = problem.make_initial_state();
    const auto initial_score = problem.full_score(initial);
    const double remaining = timer.remaining_ms(TIME_LIMIT_MS);
    if (remaining <= 0.0) {
        problem.print_answer(initial);
        return 0;
    }

    LnsOptions options;
    options.time_limit_ms = remaining;
    options.seed = 0;
    options.maximize = true;  // コストをそのまま返す最小化ならfalse。
    options.acceptance = LnsAcceptance::HillClimbing;
    // SAに変更: SimulatedAnnealing + start_temperature / end_temperature。
    // RRTに変更: RecordToRecord + start_margin / end_margin（最良値からの悪化幅）。
    LargeNeighborhoodSearch<Problem> search(problem, initial, initial_score, options);
    search.run();
    problem.print_answer(search.best_state());
}
