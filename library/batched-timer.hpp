#include <algorithm>
#include <cassert>
#include <chrono>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/batched-timer.hpp

// 時計を見る回数を減らすタイマー。1反復がとても軽い探索向け。
// 最初の呼び出しと、その後 check_interval 回ごとに時計を見る。
//
// 使い方:
// BatchedTimer timer(1900.0, 256);
// while (!timer.is_over()) {
//   // 軽い探索を1回進める
// }
struct BatchedTimer {
  double time_limit_ms;
  int check_interval;
  int calls_until_check = 0;
  bool over = false;
  double last_elapsed_ms = 0.0;
  std::chrono::steady_clock::time_point start;

  BatchedTimer(double time_limit_ms_value, int check_interval_value)
      : time_limit_ms(time_limit_ms_value),
        check_interval(check_interval_value),
        start(std::chrono::steady_clock::now()) {
    assert(time_limit_ms_value > 0.0);
    assert(check_interval_value > 0);
  }

  void reset() {
    calls_until_check = 0;
    over = false;
    last_elapsed_ms = 0.0;
    start = std::chrono::steady_clock::now();
  }

  double elapsed_ms() const {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(now - start).count();
  }

  bool is_over() {
    if (over) return true;
    if (calls_until_check > 0) {
      --calls_until_check;
      return false;
    }
    calls_until_check = check_interval - 1;
    last_elapsed_ms = elapsed_ms();
    over = last_elapsed_ms >= time_limit_ms;
    return over;
  }

  // 最後に時計を見た時点の進捗率。時計APIは呼ばない。
  double cached_progress() const {
    return std::clamp(last_elapsed_ms / time_limit_ms, 0.0, 1.0);
  }
};
