#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/prime-table.hpp

// [0, limit] の素数判定と最小素因数を O(limit) で前計算する線形篩。
struct PrimeTable {
  int limit;
  std::vector<int> smallest_factor;
  std::vector<int> primes;

  explicit PrimeTable(int limit)
      : limit(limit), smallest_factor(limit + 1, 0) {
    assert(limit >= 0);
    for (int value = 2; value <= limit; ++value) {
      if (smallest_factor[value] == 0) {
        smallest_factor[value] = value;
        primes.push_back(value);
      }
      for (int prime : primes) {
        if (prime > smallest_factor[value] ||
            static_cast<long long>(value) * prime > limit) {
          break;
        }
        smallest_factor[value * prime] = prime;
      }
    }
  }

  bool is_prime(int value) const {
    assert(0 <= value && value <= limit);
    return value >= 2 && smallest_factor[value] == value;
  }

  std::vector<std::pair<int, int>> factorize(int value) const {
    assert(1 <= value && value <= limit);
    std::vector<std::pair<int, int>> factors;
    while (value > 1) {
      const int prime = smallest_factor[value];
      int exponent = 0;
      do {
        value /= prime;
        ++exponent;
      } while (value > 1 && smallest_factor[value] == prime);
      factors.push_back({prime, exponent});
    }
    return factors;
  }

  std::vector<int> divisors(int value) const {
    std::vector<int> result{1};
    for (const auto& [prime, exponent] : factorize(value)) {
      const int old_size = static_cast<int>(result.size());
      int power = 1;
      for (int count = 1; count <= exponent; ++count) {
        power *= prime;
        for (int i = 0; i < old_size; ++i) {
          result.push_back(result[i] * power);
        }
      }
    }
    std::sort(result.begin(), result.end());
    return result;
  }
};
