#include <cassert>
#include <cstdint>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/mod-combination.hpp

// 素数MODで nCk、nPk を O(1) で取得する。事前計算は O(max_n)。
// 0 <= n < MOD で使う。
// 使い方:
// ModCombination<998244353> combination(max_n);
// int answer = combination.choose(n, k);
template <int Mod>
struct ModCombination {
  static_assert(Mod >= 2, "Mod must be at least 2");

  std::vector<int> factorials{1};
  std::vector<int> inverse_factorials{1};

  explicit ModCombination(int max_n = 0) { ensure(max_n); }

  void ensure(int max_n) {
    assert(0 <= max_n && max_n < Mod);
    const int old_size = static_cast<int>(factorials.size());
    if (max_n < old_size) return;
    factorials.resize(max_n + 1);
    inverse_factorials.resize(max_n + 1);
    for (int i = old_size; i <= max_n; ++i) {
      factorials[i] =
          static_cast<int>(static_cast<std::int64_t>(factorials[i - 1]) * i %
                           Mod);
    }
    inverse_factorials[max_n] = power(factorials[max_n], Mod - 2);
    for (int i = max_n; i > old_size; --i) {
      inverse_factorials[i - 1] = static_cast<int>(
          static_cast<std::int64_t>(inverse_factorials[i]) * i % Mod);
    }
  }

  int factorial(int n) {
    ensure(n);
    return factorials[n];
  }

  int inverse_factorial(int n) {
    ensure(n);
    return inverse_factorials[n];
  }

  int choose(int n, int k) {
    if (k < 0 || n < k) return 0;
    ensure(n);
    return multiply(factorials[n],
                    multiply(inverse_factorials[k],
                             inverse_factorials[n - k]));
  }

  int permutation(int n, int k) {
    if (k < 0 || n < k) return 0;
    ensure(n);
    return multiply(factorials[n], inverse_factorials[n - k]);
  }

  // 重複を許してn種類からk個選ぶ。n=0,k=0は1。
  int multichoose(int n, int k) {
    if (n < 0 || k < 0) return 0;
    if (n == 0) return k == 0 ? 1 : 0;
    return choose(n + k - 1, k);
  }

 private:
  static int multiply(int a, int b) {
    return static_cast<int>(static_cast<std::int64_t>(a) * b % Mod);
  }

  static int power(int base, int exponent) {
    int result = 1;
    while (exponent > 0) {
      if (exponent & 1) result = multiply(result, base);
      base = multiply(base, base);
      exponent >>= 1;
    }
    return result;
  }
};
