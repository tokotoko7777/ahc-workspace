#include <cassert>
#include <cmath>
#include <type_traits>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/integer-square-root.hpp

// valueの平方根を整数へ切り下げる。浮動小数で概算した後、整数演算で必ず補正する。
// 使い方: long long r = floor_integer_square_root(1000000000000LL);
template <class Integer>
Integer floor_integer_square_root(Integer value) {
  static_assert(std::is_integral_v<Integer>);
  assert(value >= 0);

  Integer root = static_cast<Integer>(
      std::sqrt(static_cast<long double>(value)));
  while (root > 0 && root > value / root) --root;
  while (root + 1 > root && root + 1 <= value / (root + 1)) ++root;
  return root;
}

// value以上になる最小の r*r のr、つまり平方根の切り上げを返す。
template <class Integer>
Integer ceil_integer_square_root(Integer value) {
  const Integer root = floor_integer_square_root(value);
  return root * root == value ? root : root + 1;
}
