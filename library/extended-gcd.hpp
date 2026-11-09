#include <cassert>
#include <optional>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/extended-gcd.hpp

template <class Integer>
struct ExtendedGcdResult {
  Integer gcd;
  Integer x;
  Integer y;
};

// a*x + b*y = gcd(a,b) を満すgcd>=0とx,yを取得する。
template <class Integer>
ExtendedGcdResult<Integer> extended_gcd(Integer a, Integer b) {
  Integer old_remainder = a;
  Integer remainder = b;
  Integer old_x = 1;
  Integer x = 0;
  Integer old_y = 0;
  Integer y = 1;
  while (remainder != 0) {
    const Integer quotient = old_remainder / remainder;
    const Integer next_remainder = old_remainder - quotient * remainder;
    old_remainder = remainder;
    remainder = next_remainder;
    const Integer next_x = old_x - quotient * x;
    old_x = x;
    x = next_x;
    const Integer next_y = old_y - quotient * y;
    old_y = y;
    y = next_y;
  }
  if (old_remainder < 0) {
    old_remainder = -old_remainder;
    old_x = -old_x;
    old_y = -old_y;
  }
  return {old_remainder, old_x, old_y};
}

// value * inverse ≡ 1 (mod modulus)。逆元がなけれぺnullopt。
template <class Integer>
std::optional<Integer> modular_inverse(Integer value, Integer modulus) {
  assert(modulus > 0);
  const auto result = extended_gcd(value, modulus);
  if (result.gcd != 1) return std::nullopt;
  Integer inverse = result.x % modulus;
  if (inverse < 0) inverse += modulus;
  return inverse;
}
