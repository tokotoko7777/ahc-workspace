#include <chrono>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/timer.hpp

// 使い方:
// Timer timer;
// while (!timer.is_over(1900.0)) {
//   // 探索を1回進める
// }
struct Timer {
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();

  void reset() { start = std::chrono::steady_clock::now(); }

  double elapsed_ms() const {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(now - start).count();
  }

  bool is_over(double limit_ms) const { return elapsed_ms() >= limit_ms; }

  double remaining_ms(double limit_ms) const {
    const double value = limit_ms - elapsed_ms();
    return value > 0.0 ? value : 0.0;
  }

  double progress(double limit_ms) const {
    if (limit_ms <= 0.0) return 1.0;
    const double value = elapsed_ms() / limit_ms;
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
  }
};
