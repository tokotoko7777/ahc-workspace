// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
#include "../../library/deterministic-rollout.hpp"
// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc058_deterministic_rollout.cpp
// Official problem: https://atcoder.jp/contests/ahc058/tasks/ahc058_a
using namespace std;
using i128 = __int128_t;

#ifndef ROLLOUT_DEPTH
#define ROLLOUT_DEPTH 3
#endif
#ifndef ROLLOUT_CANDIDATES
#define ROLLOUT_CANDIDATES 12
#endif
static_assert(ROLLOUT_DEPTH == 2 || ROLLOUT_DEPTH == 3);
static_assert(ROLLOUT_CANDIDATES >= 0);

// ============================================================================
// TODO(AHC058): ここから問題依存部分。RunnerはこのProblemだけを呼ぶ。
// 編集する順番: State/Action → generate_actions → advance → evaluate_action。
// ============================================================================
struct Problem {
    static constexpr int IDS = 10, LEVELS = 4;
    static constexpr i128 VALUE_LIMIT = i128{1} << 120;
    using Counts = array<array<i128, IDS>, LEVELS>;
    using Powers = array<array<int, IDS>, LEVELS>;
    struct State {
        // TODO: 現在の機械数、強化回数、りんご数、残りターン数を置く。
        // 固定長配列なので先読みでコピーしてもヒープ確保しない。
        Counts count{};
        Powers power{};
        i128 apples = 1;
        int turns = 500;
    };
    using Action = int;  // TODO: -1=待機、それ以外=level*10+id。
    using Score = i128;  // TODO: 最終りんご数の見積り。浮動小数へ丸めず比較。

    array<long long, IDS> production{};
    array<array<long long, IDS>, LEVELS> cost{};

    static i128 add(i128 a, i128 b) {
        return a >= VALUE_LIMIT - b ? VALUE_LIMIT : a + b;
    }
    static i128 multiply(i128 a, i128 b) {
        if (a == 0 || b == 0) return 0;
        return a >= VALUE_LIMIT / b ? VALUE_LIMIT : a * b;
    }
    static long long combination(int n, int k) {
        if (k < 0 || k > n) return 0;
        k = min(k, n - k);
        long long result = 1;
        for (int i = 1; i <= k; ++i) result = result * (n - k + i) / i;
        return result;
    }

    // TODO: 以後強化しない場合、このIDが残りturnsで作るりんご数を返す。
    // 毎ターン再生せず、機械の線形な増加を二項係数でまとめる。
    i128 future_production(int id, const State& state) const {
        i128 sum = multiply(state.count[0][id], state.turns);
        i128 product = 1;
        for (int level = 1; level < LEVELS; ++level) {
            product = multiply(product, state.power[level][id]);
            i128 term = multiply(state.count[level][id], product);
            term = multiply(term, combination(state.turns, level + 1));
            sum = add(sum, term);
        }
        return multiply(multiply(sum, state.power[0][id]), production[id]);
    }

    long long upgrade_cost(const State& state, Action action) const {
        return cost[action / IDS][action % IDS] *
               (state.power[action / IDS][action % IDS] + 1LL);
    }

    // TODO: 今実行可能な最初の1手を返す。待機も必ず入れる。
    // 同点では列挙順を優先するので、旧版と同じ「待機→番号順」。
    vector<Action> generate_actions(const State& state) const {
        vector<Action> actions;
        actions.reserve(1 + IDS * LEVELS);
        actions.push_back(-1);
        for (int code = 0; code < IDS * LEVELS; ++code) {
            if (state.apples >= upgrade_cost(state, code)) actions.push_back(code);
        }
        return actions;
    }

    // TODO: 合法なactionを反映し、公式順序で1ターン進める。
    // 本番の更新と仮実行で必ず同じ処理を使う。Score用の飽和とは分ける。
    void advance(State& state, Action action) const {
        assert(state.turns > 0);
        if (action != -1) {
            assert(action >= 0 && action < IDS * LEVELS);
            const long long payment = upgrade_cost(state, action);
            assert(state.apples >= payment);
            state.apples -= payment;
            ++state.power[action / IDS][action % IDS];
        }
        for (int id = 0; id < IDS; ++id) {
            state.apples += state.count[0][id] * state.power[0][id] * production[id];
        }
        for (int level = 1; level < LEVELS; ++level) {
            for (int id = 0; id < IDS; ++id) {
                state.count[level - 1][id] += state.count[level][id] * state.power[level][id];
            }
        }
        --state.turns;
    }

    // TODO: 今1回だけ強化するか待機し、以後待機した場合の最大最終値を返す。
    Score value_with_one_upgrade(State& state) const {
        i128 value = state.apples;
        for (int id = 0; id < IDS; ++id) value = add(value, future_production(id, state));
        if (state.turns == 0) return value;
        i128 best_gain = 0;
        for (int code = 0; code < IDS * LEVELS; ++code) {
            const long long payment = upgrade_cost(state, code);
            if (state.apples < payment) continue;
            const int id = code % IDS, level = code / IDS;
            const i128 before = future_production(id, state);
            ++state.power[level][id];
            const i128 after = future_production(id, state);
            --state.power[level][id];
            best_gain = max(best_gain, after - before - payment);
        }
        return add(value, best_gain);
    }

    // TODO: 2手目だけ絞り込む。上位12個に各Level代表と待機を足す。
    // 長期投資の種類を1つの評価の上位だけで全滅させないため。
    vector<Action> second_actions(State& state) const {
        struct Ranked { i128 gain; int code; };
        vector<Ranked> ranked;
        ranked.reserve(IDS * LEVELS);
        array<int, LEVELS> best_code;
        array<i128, LEVELS> best_gain;
        best_code.fill(-1);
        best_gain.fill(-VALUE_LIMIT);
        for (int code = 0; code < IDS * LEVELS; ++code) {
            const long long payment = upgrade_cost(state, code);
            if (state.apples < payment) continue;
            const int id = code % IDS, level = code / IDS;
            const i128 before = future_production(id, state);
            ++state.power[level][id];
            const i128 after = future_production(id, state);
            --state.power[level][id];
            const i128 gain = after - before - payment;
            ranked.push_back({gain, code});
            if (gain > best_gain[level]) {
                best_gain[level] = gain;
                best_code[level] = code;
            }
        }
        sort(ranked.begin(), ranked.end(), [](const Ranked& a, const Ranked& b) {
            return a.gain > b.gain;
        });
        vector<Action> actions{-1};
        const int count = min(ROLLOUT_CANDIDATES, static_cast<int>(ranked.size()));
        for (int i = 0; i < count; ++i) actions.push_back(ranked[i].code);
        for (int code : best_code) {
            if (code != -1 && find(actions.begin(), actions.end(), code) == actions.end()) {
                actions.push_back(code);
            }
        }
        return actions;
    }

    // TODO: actionを最初に選んだときの評価を返す。元のstateは変えない。
    // 既定3手: 最初の1手→絞った2手目→式で選ぶ3手目→以後待機。
    Score evaluate_action(const State& state, Action action) const {
        State next = state;
        advance(next, action);
        if (next.turns == 0) return next.apples;
#if ROLLOUT_DEPTH >= 3
        i128 best = -1;
        for (Action second : second_actions(next)) {
            State after = next;
            advance(after, second);
            best = max(best, value_with_one_upgrade(after));
        }
        return best;
#else
        return value_with_one_upgrade(next);
#endif
    }

    // TODO: 入力を読む。公式の固定サイズを確認してから固定長配列へ格納する。
    State read_input() {
        int n, levels, turns;
        long long apples;
        if (!(cin >> n >> levels >> turns >> apples)) throw runtime_error("missing input");
        if (n != IDS || levels != LEVELS || turns != 500 || apples != 1) {
            throw runtime_error("expected AHC058 input: 10 4 500 1");
        }
        for (auto& x : production) cin >> x;
        for (auto& row : cost) for (auto& x : row) cin >> x;
        if (!cin) throw runtime_error("incomplete input");
        State state;
        state.apples = apples;
        state.turns = turns;
        for (auto& row : state.count) row.fill(1);
        return state;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Problem problem;
    auto state = problem.read_input();
    DeterministicRolloutRunner<Problem> runner(problem);
    runner.reserve(1 + Problem::IDS * Problem::LEVELS);
    while (state.turns > 0) {
        const int action = runner.choose_action(state);
        // TODO: 選んだ最初の1手だけ出力・反映する。次ターンは改めて先読みする。
        if (action == -1) cout << -1 << '\n';
        else cout << action / Problem::IDS << ' ' << action % Problem::IDS << '\n';
        problem.advance(state, action);
    }
    return 0;
}
