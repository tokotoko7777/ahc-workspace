#include <cassert>
#include <utility>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/floor-sum.hpp

// sum_{i=0}^{n-1} floor((a*i+b)/modulus) を O(log modulus) で求める。
// n>=0, modulus>0。a,bは負でもよい。答えと中間の乗算はlong longに収まること。
inline long long floor_sum(long long n, long long modulus, long long a,
                           long long b) {
  assert(n >= 0 && modulus > 0);
  const auto triangular = [](long long value) {
    return value % 2 == 0 ? (value / 2) * (value - 1)
                          : value * ((value - 1) / 2);
  };
  long long answer = 0;
  if (a < 0) {
    const long long normalized = (a % modulus + modulus) % modulus;
    answer -= triangular(n) * ((normalized - a) / modulus);
    a = normalized;
  }
  if (b < 0) {
    const long long normalized = (b % modulus + modulus) % modulus;
    answer -= n * ((normalized - b) / modulus);
    b = normalized;
  }

  while (true) {
    if (a >= modulus) {
      answer += triangular(n) * (a / modulus);
      a %= modulus;
    }
    if (b >= modulus) {
      answer += n * (b / modulus);
      b %= modulus;
    }
    const long long maximum = a * n + b;
    if (maximum < modulus) break;
    n = maximum / modulus;
    b = maximum % modulus;
    std::swap(modulus, a);
  }
  return answer;
}
