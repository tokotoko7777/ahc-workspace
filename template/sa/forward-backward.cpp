// AHC_SOURCE: template/sa/forward-backward.cpp
// Source repository: https://github.com/tokotoko7777/ahc-workspace
// 有限状態の確率遷移: 固定長の操作列を1か所ずつ変えて期待値を改善する。
// TODOの型・関数宣言を実装する。列の挿入・削除はcache.buildからやり直す。
#include <iostream>
#include <optional>
#include <random>
#include <vector>
#include "library/best-keeper.hpp"
#include "library/forward-backward-dp.hpp"
#include "library/time-based-simulated-annealing.hpp"
#include "library/timer.hpp"

constexpr double TIME_LIMIT_MS = 1900.0;  // TODO

struct Problem {
    using Action = int;  // TODO: 1操作を表す型。
    struct Edge {
        int to;              // 次状態番号。-1は終了。
        double probability;  // 各from・turn・actionで合計1。
        double reward;       // この遷移が起きたときの報酬（確率を掛ける前）。
    };
    // TODO: 固定入力と以下の関数。state_count>=1、分布と報酬の長さはstate_count。
    void read_input();
    int state_count() const;
    std::vector<double> initial_distribution() const;
    std::vector<double> terminal_rewards() const;
    std::vector<Action> initial_actions() const;
    void print_answer(const std::vector<Action>& answer) const;

    // TODO: 同じ引数からは常に同じ遷移。列の他位置に依存させない。
    std::vector<Edge> transitions(std::size_t turn, int from, Action action) const;
    // TODO: 変更位置（0<=position<actions.size()）と合法な新操作。空列ならnullopt。
    std::optional<std::pair<std::size_t, Action>> propose(
        const std::vector<Action>& actions, std::mt19937_64& rng) const;
};

int main() {
    Timer timer;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    Problem problem;
    problem.read_input();
    const auto transitions = [&](std::size_t turn, int from, Problem::Action action,
                                 const auto& emit) {
        for (const auto& edge : problem.transitions(turn, from, action))
            emit(edge.to, edge.probability, edge.reward);
    };
    ForwardBackwardDP<Problem::Action> cache(problem.state_count());
    cache.build(problem.initial_distribution(), problem.initial_actions(),
                transitions, problem.terminal_rewards());
    BestKeeper<double, std::vector<Problem::Action>> best(cache.score(), cache.actions());
    std::mt19937_64 rng(0);
    const double remaining = timer.remaining_ms(TIME_LIMIT_MS);
    if (remaining > 0.0) {
        TimeBasedSimulatedAnnealing sa(remaining, 100.0, 1.0, 0);  // TODO: 温度。
        while (!sa.is_over()) {
            auto move = problem.propose(cache.actions(), rng);
            if (!move) continue;
            const auto [position, action] = *move;
            const double score = cache.score_if_changed(position, action, transitions);
            if (!sa.accept(score - cache.score())) continue;
            cache.commit_change(position, action, transitions);
            best.update(cache.score(), cache.actions());
        }
    }
    problem.print_answer(best.best_state);
}

// 仮評価は軽くなるが、採用時は前向き表・後ろ向き表の一部を再構築する。
// 状態数×列長のメモリと、1回のcommit時間を見て採用する。
