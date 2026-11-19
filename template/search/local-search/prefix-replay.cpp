// AHC_SOURCE: template/search/local-search/prefix-replay.cpp
// Source repository: https://github.com/tokotoko7777/ahc-workspace
// 行動列を変更し、共通する前半の再実行を省く。TODOの型・宣言を実装する。
#include <iostream>
#include <optional>
#include <random>
#include <vector>
#include "library/best-keeper.hpp"
#include "library/prefix-replay.hpp"
#include "library/time-based-simulated-annealing.hpp"
#include "library/timer.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO

struct Problem {
    using Score = long long;  // 大きいほど良い。
    struct State {
        // TODO: 途中時刻・資源など、続きの実行に必要な全情報。
    };
    struct Action {
        // TODO: 1行動と等値比較。同じ遷移を表すときだけtrue。
        bool operator==(const Action& other) const;
    };
    using Actions = std::vector<Action>;
    // TODO: 固定入力・入出力。初期行動列は必ず合法な完成解。
    void read_input();
    State initial_state() const;
    Actions initial_actions() const;
    void print_answer(const Actions& answer) const;

    // TODO: 決定的な1手。隠れた乱数状態や外部の可変情報に依存させない。
    void step(State& state, const Action& action) const;
    bool is_valid(const State& end) const;
    Score evaluate(const State& end) const;
    // TODO: currentをコピーし、入替・挿入・削除などを施す。作れなければnullopt。
    // stepは返された列を最後まで処理できるようにする（不正状態も表現する等）。
    std::optional<Actions> propose(const Actions& current, std::mt19937_64& rng) const;
};

int main() {
    Timer timer;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    Problem problem;
    problem.read_input();
    PrefixReplay<Problem::State, Problem::Action> replay(problem.initial_state(), 16);
    const auto step = [&](Problem::State& s, const Problem::Action& a) { problem.step(s, a); };
    replay.evaluate(problem.initial_actions(), step);
    replay.commit();
    auto current_score = problem.evaluate(replay.current_end());
    BestKeeper<Problem::Score, Problem::Actions> best(current_score, replay.actions());
    std::mt19937_64 rng(0);
    const double remaining = timer.remaining_ms(TIME_LIMIT_MS);
    if (remaining > 0.0) {
        TimeBasedSimulatedAnnealing sa(remaining, 100.0, 1.0, 0);  // TODO: 温度。
        while (!sa.is_over()) {
            auto candidate = problem.propose(replay.actions(), rng);
            if (!candidate) continue;
            const auto& end = replay.evaluate(*candidate, step);
            if (!problem.is_valid(end)) {
                replay.discard();
                continue;
            }
            const auto score = problem.evaluate(end);
            if (sa.accept(score - current_score)) {
                replay.commit();  // endの参照はここから先で使わない。
                current_score = score;
                best.update(score, replay.actions());
            } else {
                replay.discard();
            }
        }
    }
    problem.print_answer(best.best_state);
}
