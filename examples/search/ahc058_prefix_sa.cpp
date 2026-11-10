// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
#include "../../library/deterministic-rollout.hpp"
#include "../../library/prefix-replay.hpp"
#include "../../library/time-based-simulated-annealing.hpp"
// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc058_prefix_sa.cpp
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
struct ProductionProblem {
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


// ============================================================================
// TODO(AHC058-SA): 購入順序を改善する問題依存部分。
// Stateは短い行動列だけ。大きいprefix cacheはProblem側に1個だけ持つ。
// ============================================================================
#ifndef AHC058_TIME_LIMIT_MS
#define AHC058_TIME_LIMIT_MS 1850
#endif
#ifndef AHC058_SA_ITERATIONS
#define AHC058_SA_ITERATIONS 0
#endif
#ifndef AHC058_RANDOM_SEED
#define AHC058_RANDOM_SEED 58
#endif

struct PurchaseSequenceProblem {
    using SimulationState = ProductionProblem::State;
    using Score = long long; // TODO: 最終りんご数を公式のlogスコアへ変換する。
    struct State {
        vector<int> actions; // TODO: 待機を除いた「購入する順序」。先頭0は保護する。
        Score score = 0;
    };
    struct Move { vector<int> actions; }; // TODO: 変更後の購入順序。

    const ProductionProblem& game;
    SimulationState initial;
    PrefixReplay<SimulationState, int> replay;
    Score trial_score = 0;

    PurchaseSequenceProblem(const ProductionProblem& game_value,
                            const SimulationState& initial_value)
        : game(game_value), initial(initial_value), replay(initial_value, 8) {
        replay.reserve(500);
    }

    // TODO: 購入しない間の生産量をC(t,1)..C(t,4)の係数で表す。
    // 同じ係数で待ち時間の二分探索を行うので、各判定は4項だけになる。
    array<i128, 4> income_coefficients(const SimulationState& state) const {
        array<i128, 4> coefficients{};
        for (int id = 0; id < ProductionProblem::IDS; ++id) {
            i128 factor = game.production[id];
            for (int level = 0; level < ProductionProblem::LEVELS; ++level) {
                factor *= state.power[level][id];
                coefficients[level] += state.count[level][id] * factor;
            }
        }
        return coefficients;
    }

    static i128 income(const array<i128, 4>& coefficients, int turns) {
        i128 result = 0;
        for (int level = 0; level < 4; ++level) {
            result += coefficients[level] *
                      ProductionProblem::combination(turns, level + 1);
        }
        return result;
    }

    // TODO: 行動しない区間を一括更新する。更新順は下位levelから。
    // 上位機械の個数はまだ書き換えていないので、その区間の開始時点の値を使える。
    void wait_turns(SimulationState& state, int turns,
                    const array<i128, 4>& coefficients) const {
        assert(0 <= turns && turns <= state.turns);
        state.apples += income(coefficients, turns);
        for (int id = 0; id < ProductionProblem::IDS; ++id) {
            for (int lower = 0; lower < ProductionProblem::LEVELS - 1; ++lower) {
                i128 factor = 1;
                for (int upper = lower + 1; upper < ProductionProblem::LEVELS; ++upper) {
                    factor *= state.power[upper][id];
                    state.count[lower][id] += state.count[upper][id] * factor *
                        ProductionProblem::combination(turns, upper - lower);
                }
            }
        }
        state.turns -= turns;
    }

    // TODO: 次の購入を最速で実行する。資金不足なら必要なだけ待つ。
    // 間に合わなければ残りを待機し、後続の購入も実行しない。
    // 「買えない操作を出力する」ことはない。入力制約内では128bitで十分。
    void advance_purchase(SimulationState& state, int action) const {
        if (state.turns == 0) return;
        assert(0 <= action && action < 40);
        const long long payment = game.upgrade_cost(state, action);
        if (state.apples < payment) {
            const auto coefficients = income_coefficients(state);
            const int last_wait = state.turns - 1;
            if (state.apples + income(coefficients, last_wait) < payment) {
                wait_turns(state, state.turns, coefficients);
                return;
            }
            int low = 0, high = last_wait;
            while (high - low > 1) {
                const int middle = (low + high) / 2;
                if (state.apples + income(coefficients, middle) >= payment) high = middle;
                else low = middle;
            }
            wait_turns(state, high, coefficients);
        }
        game.advance(state, action);
    }

    i128 final_apples(const SimulationState& state) const {
        return state.apples + income(income_coefficients(state), state.turns);
    }

    Score score(const SimulationState& state) const {
        return llround(100000.0 * log2(static_cast<double>(final_apples(state))));
    }

    State make_state(vector<int> actions) {
        State state{std::move(actions), 0};
        const auto step = [this](SimulationState& simulation, int action) {
            advance_purchase(simulation, action);
        };
        state.score = score(replay.evaluate(state.actions, step));
        replay.commit();
        return state;
    }

    // TODO: 近傍を作る。先頭の安い生産機だけは残し、交換/移動/変更/挿入/削除する。
    // ときどき同じIDの4段階をまとめて挿入し、投資先変更も試す。
    optional<Move> propose_move(const State& state, mt19937_64& engine,
                                double /* progress */) const {
        Move move{state.actions};
        auto& actions = move.actions;
        const int size = static_cast<int>(actions.size());
        if (size == 0) return nullopt;
        const auto random_index = [&](int n) { return static_cast<int>(engine() % static_cast<uint64_t>(n)); };
        const auto random_action = [&]() {
            if ((engine() & 1U) != 0) return random_index(40);
            const int id = actions[random_index(size)] % 10;
            return random_index(4) * 10 + id;
        };
        const int kind = random_index(100);
        if (kind < 40 && size > 2) {
            const int from = 1 + random_index(size - 1);
            int to = 1 + random_index(size - 1);
            if ((engine() & 1U) != 0) {
                to = clamp(from + random_index(13) - 6, 1, size - 1);
            }
            if (kind < 20) swap(actions[from], actions[to]);
            else {
                const int action = actions[from];
                actions.erase(actions.begin() + from);
                actions.insert(actions.begin() + to, action);
            }
        } else if (kind < 65 && size > 1) {
            actions[1 + random_index(size - 1)] = random_action();
        } else if (kind < 80 && size < 500) {
            const int action = random_action();
            actions.insert(actions.begin() + 1 + random_index(size), action);
        } else if (kind < 95 && size > 1) {
            actions.erase(actions.begin() + 1 + random_index(size - 1));
        } else if (size <= 496) {
            const int id = random_index(10);
            const int at = 1 + random_index(min(size, 80));
            const array<int, 4> block{{id, 10 + id, 20 + id, 30 + id}};
            actions.insert(actions.begin() + at, block.begin(), block.end());
        }
        if (actions == state.actions) return nullopt;
        return move;
    }

    // TODO: 仮実行で得たスコアの「差分」を返す。採用前のcacheは壊さない。
    optional<Score> evaluate_move(const State& state, const Move& move,
                                  double /* threshold */) {
        // TODO: 残りの生産増加の安全な上限はここでは作らず、閾値を無視して完走する。
        const auto step = [this](SimulationState& simulation, int action) {
            advance_purchase(simulation, action);
        };
        // restart_from_best後にも使えるよう、cacheを現在の行動列へ同期する。
        if (replay.actions() != state.actions) {
            replay.evaluate(state.actions, step);
            replay.commit();
        }
        trial_score = score(replay.evaluate(move.actions, step));
        return trial_score - state.score;
    }

    // TODO: 採用されたときだけ候補とcacheを確定する。
    void apply_move(State& state, Move& move) {
        replay.commit();
        state.actions = std::move(move.actions);
        state.score = trial_score;
    }

    // TODO: 購入順序を、合法な500ターンの出力へ復号する。
    vector<int> decode(const vector<int>& purchases, i128& apples) const {
        auto state = initial;
        vector<int> actions;
        actions.reserve(500);
        for (int action : purchases) {
            if (state.turns == 0) break;
            const long long payment = game.upgrade_cost(state, action);
            while (state.turns > 0 && state.apples < payment) {
                actions.push_back(-1);
                game.advance(state, -1);
            }
            if (state.turns == 0) break;
            actions.push_back(action);
            game.advance(state, action);
        }
        while (state.turns > 0) {
            actions.push_back(-1);
            game.advance(state, -1);
        }
        apples = state.apples;
        return actions;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    const auto started = chrono::steady_clock::now();
    const auto elapsed_ms = [&]() {
        return chrono::duration<double, milli>(chrono::steady_clock::now() - started).count();
    };
    ProductionProblem game;
    const auto initial = game.read_input();
    auto simulation = initial;
    DeterministicRolloutRunner<ProductionProblem> rollout(game);
    rollout.reserve(41);
    vector<int> baseline, purchases;
    baseline.reserve(500);
    purchases.reserve(500);
    // 既存の3手先読みで初期解を構築。初期解作成も総時間に含める。
    while (simulation.turns > 0) {
        const int action = rollout.choose_action(simulation);
        baseline.push_back(action);
        if (action != -1) purchases.push_back(action);
        game.advance(simulation, action);
    }
    const i128 baseline_apples = simulation.apples;
    PurchaseSequenceProblem problem(game, initial);
    auto state = problem.make_state(std::move(purchases));
    const double remaining_ms = AHC058_TIME_LIMIT_MS - elapsed_ms();
    vector<int> best_purchases = state.actions;
    uint64_t iterations = 0;
    if (remaining_ms > 10 || AHC058_SA_ITERATIONS > 0) {
        // TODO: 公式スコアの差に対する温度。例: 2500点→5点。
        TimeBasedAnnealingRunner<PurchaseSequenceProblem> annealing(
            problem, state, state.score,
            AHC058_SA_ITERATIONS > 0 ? 1e9 : remaining_ms,
            2500, 5, AHC058_RANDOM_SEED, 8);
        if (AHC058_SA_ITERATIONS > 0) {
            // 固定反復は回帰テスト専用。通常提出は時間制限で止める。
            for (int i = 0; i < AHC058_SA_ITERATIONS; ++i) annealing.step();
        } else annealing.run();
        best_purchases = annealing.best_state().actions;
        iterations = annealing.iterations();
    }
    i128 candidate_apples = 0;
    auto answer = problem.decode(best_purchases, candidate_apples);
    // 元の先読み出力を保持し、厳密なりんご数でも悪化させない。
    if (candidate_apples < baseline_apples) answer = baseline;
    for (int action : answer) {
        if (action == -1) cout << -1 << '\n';
        else cout << action / 10 << ' ' << action % 10 << '\n';
    }
    cerr << "iterations=" << iterations << " elapsed_ms=" << elapsed_ms() << '\n';
    return 0;
}
