#include <cassert>
#include <type_traits>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/static-mod-int.hpp

// コンパイル時にmodを固定する剰余整数。
// 使い方:
// using Mint = StaticModInt<998244353>;
// Mint answer = Mint(2).pow(100) / 3;
template <int Modulo>
struct StaticModInt {
  static_assert(2 <= Modulo && Modulo <= 2000000000);

  int stored_value = 0;

  StaticModInt() = default;

  template <class Integer,
            std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
  StaticModInt(Integer value) {
    long long remainder = static_cast<long long>(value % Modulo);
    if (remainder < 0) remainder += Modulo;
    stored_value = static_cast<int>(remainder);
  }

  static constexpr int mod() { return Modulo; }
  int value() const { return stored_value; }

  // すでに 0 <= value < mod() と分かっている時だけ使う高速な生成方法。
  static StaticModInt raw(int value) {
    assert(0 <= value && value < Modulo);
    StaticModInt result;
    result.stored_value = value;
    return result;
  }

  StaticModInt& operator+=(const StaticModInt& other) {
    const long long sum =
        static_cast<long long>(stored_value) + other.stored_value;
    stored_value = static_cast<int>(sum >= Modulo ? sum - Modulo : sum);
    return *this;
  }

  StaticModInt& operator-=(const StaticModInt& other) {
    stored_value -= other.stored_value;
    if (stored_value < 0) stored_value += Modulo;
    return *this;
  }

  StaticModInt& operator*=(const StaticModInt& other) {
    stored_value = static_cast<int>(
        static_cast<long long>(stored_value) * other.stored_value % Modulo);
    return *this;
  }

  StaticModInt pow(long long exponent) const {
    assert(exponent >= 0);
    StaticModInt base = *this;
    StaticModInt result = 1;
    while (exponent > 0) {
      if (exponent & 1) result *= base;
      base *= base;
      exponent >>= 1;
    }
    return result;
  }

  StaticModInt inverse() const {
    long long a = stored_value;
    long long b = Modulo;
    long long x = 1;
    long long y = 0;
    while (b != 0) {
      const long long quotient = a / b;
      a -= quotient * b;
      x -= quotient * y;
      const long long old_a = a;
      a = b;
      b = old_a;
      const long long old_x = x;
      x = y;
      y = old_x;
    }
    assert(a == 1);
    return StaticModInt(x);
  }

  StaticModInt& operator/=(const StaticModInt& other) {
    return *this *= other.inverse();
  }

  StaticModInt operator-() const {
    return stored_value == 0 ? *this : raw(Modulo - stored_value);
  }

  friend StaticModInt operator+(StaticModInt a, const StaticModInt& b) {
    return a += b;
  }
  friend StaticModInt operator-(StaticModInt a, const StaticModInt& b) {
    return a -= b;
  }
  friend StaticModInt operator*(StaticModInt a, const StaticModInt& b) {
    return a *= b;
  }
  friend StaticModInt operator/(StaticModInt a, const StaticModInt& b) {
    return a /= b;
  }
  friend bool operator==(const StaticModInt& a, const StaticModInt& b) {
    return a.stored_value == b.stored_value;
  }
  friend bool operator!=(const StaticModInt& a, const StaticModInt& b) {
    return !(a == b);
  }
};

using ModInt998244353 = StaticModInt<998244353>;
using ModInt1000000007 = StaticModInt<1000000007>;
