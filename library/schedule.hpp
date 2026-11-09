#include <algorithm>
#include <cassert>
#include <cmath>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/schedule.hpp

// progress は 0.0 から 1.0。
// 使い方:
// double width = linear_schedule(100.0, 10.0, progress);
// int trials = power_schedule(1000, 100, progress, 2.0);
template <class T>
T linear_schedule(T start, T end, double progress) {
  progress = std::clamp(progress, 0.0, 1.0);
  return static_cast<T>(static_cast<long double>(start) * (1.0 - progress) +
                        static_cast<long double>(end) * progress);
}

template <class T>
T power_schedule(T start, T end, double progress, double power) {
  assert(power > 0.0);
  progress = std::clamp(progress, 0.0, 1.0);
  const double curved = std::pow(progress, power);
  return static_cast<T>(static_cast<long double>(start) * (1.0 - curved) +
                        static_cast<long double>(end) * curved);
}

inline double geometric_schedule(
    double start, double end, double progress) {
  assert(start > 0.0);
  assert(end > 0.0);
  progress = std::clamp(progress, 0.0, 1.0);
  return start * std::pow(end / start, progress);
}
