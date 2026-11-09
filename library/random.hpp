#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/random.hpp

// 使い方:
// Random random(123);
// int x = random.next_int(0, 10);        // 0以上10未満
// long long y = random.next_int(0LL, 1LL << 40);
// double p = random.next_real();         // 0以上1未満
struct Random {
  std::mt19937_64 engine;

  explicit Random(std::uint64_t seed = 0) : engine(seed) {}

  std::uint64_t next_u64() { return engine(); }

  // [left, right) の整数を返す。int、long long などを選べる。
  template <class Int>
  Int next_int(Int left, Int right) {
    assert(left < right);
    std::uniform_int_distribution<Int> distribution(left, right - 1);
    return distribution(engine);
  }

  // [left, right) の小数を返す。型を省略すると double。
  template <class Real = double>
  Real next_real(Real left = Real(0), Real right = Real(1)) {
    assert(left < right);
    std::uniform_real_distribution<Real> distribution(left, right);
    return distribution(engine);
  }

  double next_double() { return next_real<double>(); }

  template <class T>
  void shuffle(std::vector<T>& values) {
    std::shuffle(values.begin(), values.end(), engine);
  }

  template <class T>
  T& choice(std::vector<T>& values) {
    assert(!values.empty());
    return values[next_int<std::size_t>(0, values.size())];
  }

  template <class T>
  const T& choice(const std::vector<T>& values) {
    assert(!values.empty());
    return values[next_int<std::size_t>(0, values.size())];
  }

  // weights[i] に比例する確率で添字 i を返す。
  template <class Weight>
  int weighted_index(const std::vector<Weight>& weights) {
    assert(!weights.empty());

    long double total = 0.0L;
    for (const Weight& weight : weights) {
      assert(weight >= Weight(0));
      total += static_cast<long double>(weight);
    }
    assert(total > 0.0L);

    const long double target = next_real<long double>(0.0L, total);
    long double sum = 0.0L;
    for (int i = 0; i < static_cast<int>(weights.size()); ++i) {
      sum += static_cast<long double>(weights[i]);
      if (target < sum) return i;
    }
    return static_cast<int>(weights.size()) - 1;
  }
};
