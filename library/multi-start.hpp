#include <cassert>
#include <chrono>
#include <optional>
#include <type_traits>
#include <utility>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/multi-start.hpp

// 初期解を何個も作り、一番良いものを返す。
// generate() が State、evaluate(state) が Score を返すようにする。
//
// 使い方:
// State best = multi_start<State>(100, generate, evaluate);
// State best = time_based_multi_start<State>(500.0, generate, evaluate);
template <class State, class Generate, class Evaluate>
State multi_start(
    int trials,
    Generate generate,
    Evaluate evaluate,
    bool maximize = true) {
  assert(trials > 0);

  State best = generate();
  using Score = std::decay_t<decltype(evaluate(best))>;
  Score best_score = evaluate(best);

  for (int trial = 1; trial < trials; ++trial) {
    State candidate = generate();
    const Score score = evaluate(candidate);
    const bool better = maximize ? best_score < score : score < best_score;
    if (better) {
      best = std::move(candidate);
      best_score = score;
    }
  }
  return best;
}

// limit_ms ミリ秒まで初期解を作る。最低でも1個は作る。
template <class State, class Generate, class Evaluate>
State time_based_multi_start(
    double limit_ms,
    Generate generate,
    Evaluate evaluate,
    bool maximize = true) {
  assert(limit_ms > 0.0);
  const auto start = std::chrono::steady_clock::now();

  State best = generate();
  using Score = std::decay_t<decltype(evaluate(best))>;
  Score best_score = evaluate(best);

  while (true) {
    const auto now = std::chrono::steady_clock::now();
    const double elapsed_ms =
        std::chrono::duration<double, std::milli>(now - start).count();
    if (elapsed_ms >= limit_ms) break;

    State candidate = generate();
    const Score score = evaluate(candidate);
    const bool better = maximize ? best_score < score : score < best_score;
    if (better) {
      best = std::move(candidate);
      best_score = score;
    }
  }
  return best;
}

// 複数の探索で締切を共有し、完成した最良解を返す（既存APIとは別の任意API）。
// fallback: 時間がなくてもそのまま出力できる合法な完成解。先に用意する。
// generate(trial, should_stop) -> std::optional<State>:
//   TODO: trial（0始まり）ごとに初期解・乱数seed・評価の重みなどを変えて探索する。
//   TODO: 長い探索の内側でも should_stop() を確認する。
//   完成したらState、未完成・修復失敗ならstd::nulloptを返す。
// evaluate(state): TODO: 完成解の絶対得点を返す。探索中の近似評価・差分ではない。
// should_stop(): 全試行で同じ締切を参照し、時間切れならtrueを返す。
//
// 例: Timer timer; // library/timer.hppを必要なら一緒に貼る。
// auto stop = [&] { return timer.is_over(1800.0); };
// auto best = budgeted_multi_start(fallback, 3, generate, evaluate, stop);
//
// 0試行・最初から時間切れならfallback。失敗試行をevaluateへ渡さない。
// 同点は先着優先。Stateはmove-onlyでもよい（fallbackをstd::moveして渡す）。
// 時計確認・中断は協調式。generateの1処理・評価・復元・出力時間は強制中断しない。
// 締切を越えて戻った「完成解」も比較する。実行時間保証ではないため余裕を残す。
// trials回の完了で早く終わることもある。同じ決定的探索の繰り返しには効果がない。
template <class State, class Generate, class Evaluate, class ShouldStop>
State budgeted_multi_start(State fallback, int trials, Generate generate,
                           Evaluate evaluate, ShouldStop should_stop,
                           bool maximize = true) {
  assert(trials >= 0);
  using Score = std::decay_t<decltype(evaluate(fallback))>;
  Score best_score = evaluate(fallback);
  for (int trial = 0; trial < trials && !should_stop(); ++trial) {
    std::optional<State> candidate = generate(trial, should_stop);
    if (!candidate) continue;
    const Score score = evaluate(*candidate);
    if (maximize ? best_score < score : score < best_score) {
      fallback = std::move(*candidate);
      best_score = score;
    }
  }
  return fallback;
}
