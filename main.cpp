// AHC_SOURCE: main.cpp
// Source repository: https://github.com/tokotoko7777/ahc-workspace
// 最初にコピーする雛形: 完成解をコピーして変更する焼きなまし。
// TODO の型・関数を問題に合わせて実装する。宣言だけの関数は意図的に未定義。
// このまま実行するサンプルではない。完成例は examples/、別の型は template/。
//
// 作業: cp main.cpp problems/ahc004/main.cpp
// 実装後: g++ -std=c++17 -O2 -I . problems/ahc004/main.cpp -o /tmp/solver
// 提出: 必要なhppを貼るか、次で1ファイルに展開（詳細はUSAGE.md）。
// python3 scripts/bundle.py problems/ahc004/main.cpp -o /tmp/submit.cpp

#include <cstdint>
#include <iostream>
#include <optional>
#include <random>

#include "library/best-keeper.hpp"
#include "library/time-based-simulated-annealing.hpp"
#include "library/timer.hpp"

// TODO 1: 制限時間・温度・seed。時間は入力と初期解構築を含み、出力用の余裕を取る。
constexpr double TIME_LIMIT_MS = 1900.0;
constexpr double START_TEMPERATURE = 100.0;
constexpr double END_TEMPERATURE = 1.0;
constexpr std::uint64_t SEED = 0;

struct Problem {
    // TODO 2: 得点型と変化する情報。固定入力・距離表などはProblem側へ置く。
    // scoreは大きいほど良い値に統一する。距離最小化なら -distance など。
    using Score = long long;
    struct State {
        // 例: std::vector<int> order; 盤面、経路、配置、必要なcacheなど。
    };
    // ここに問題の入力・前計算を持つフィールドを追加する。

    // TODO 3: 入力・初期解。初期解は時間切れでもそのまま出力できる合法な完成解。
    void read_input();
    State make_initial_state() const;

    // TODO 4: 完成解の絶対スコアを全再計算する。差分更新の検算にも使える。
    Score evaluate(const State& state) const;

    // TODO 5: currentをコピーして1か所変更した「合法な完成解」を返す。
    // currentは変更しない。作れない試行はstd::nullopt。
    // 例: State next = current; ...変更・合法性検査...; return next;
    // 乱数: std::uniform_int_distribution<int>(0, n - 1)(rng) は両端を含む。
    // progressは0..1。序盤は大きい変更、終盤は小さい変更にする等に使える。
    std::optional<State> propose_candidate(
        const State& current, std::mt19937_64& rng, double progress) const;

    // TODO 6: 解だけを標準出力へ書く。対話問題では通信に合わせて構成を変える。
    void print_answer(const State& answer) const;
};

// ここからは接続済みの探索。上のTODOを実装すれば使える。
int main() {
    Timer total_timer;
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Problem problem;
    problem.read_input();
    auto current = problem.make_initial_state();
    auto current_score = problem.evaluate(current);
    BestKeeper<Problem::Score, Problem::State> best(current_score, current);
    std::mt19937_64 rng(SEED);

    const double remaining = total_timer.remaining_ms(TIME_LIMIT_MS);
    if (remaining > 0.0) {
        TimeBasedSimulatedAnnealing sa(
            remaining, START_TEMPERATURE, END_TEMPERATURE, SEED, 1);
        while (!sa.is_over()) {
            auto candidate = problem.propose_candidate(current, rng, sa.cached_progress());
            if (!candidate) continue;
            const auto candidate_score = problem.evaluate(*candidate);
            const auto improvement = candidate_score - current_score;
            if (!sa.accept(improvement)) continue;
            // 山登りにするなら直前の行を if (improvement <= 0) continue; へ。
            current = std::move(*candidate);
            current_score = candidate_score;
            best.update(current_score, current);
        }
    }
    problem.print_answer(best.best_state);
}

// 遅くなったら: template/search/local-search/basic.cpp で
// 「Moveの生成 → 差分評価 → 採用時だけ更新」に置き換える。
// Stateのコピーと全評価が十分軽い場合は、この形のまま近傍を増やせる。
