#include <array>
#include <cassert>
#include <limits>
#include <type_traits>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/xor-basis.hpp

// XORの線形基底。追加した値の任意の部分集合XORを表す。
template <class Unsigned,
          int BitCount = std::numeric_limits<Unsigned>::digits>
struct XorBasis {
  static_assert(std::is_unsigned<Unsigned>::value,
                "XorBasis requires an unsigned integer type");
  static_assert(1 <= BitCount &&
                    BitCount <= std::numeric_limits<Unsigned>::digits,
                "BitCount is out of range");

  std::array<Unsigned, BitCount> basis{};
  int basis_size = 0;

  int rank() const { return basis_size; }

  // 独立な値として基底に追加されたらtrue。
  bool insert(Unsigned value) {
    for (int bit = BitCount - 1; bit >= 0; --bit) {
      if (((value >> bit) & Unsigned{1}) == 0) continue;
      if (basis[bit] != 0) {
        value ^= basis[bit];
      } else {
        basis[bit] = value;
        ++basis_size;
        return true;
      }
    }
    return false;
  }

  bool contains(Unsigned value) const {
    for (int bit = BitCount - 1; bit >= 0; --bit) {
      if (((value >> bit) & Unsigned{1}) != 0) value ^= basis[bit];
    }
    return value == 0;
  }

  // seed XOR (部分集合XOR) の最大値。
  Unsigned maximum_xor(Unsigned seed = 0) const {
    Unsigned result = seed;
    for (int bit = BitCount - 1; bit >= 0; --bit) {
      if ((result ^ basis[bit]) > result) result ^= basis[bit];
    }
    return result;
  }

  // seed XOR (部分集合XOR) の最小値。
  Unsigned minimum_xor(Unsigned seed) const {
    Unsigned result = seed;
    for (int bit = BitCount - 1; bit >= 0; --bit) {
      if ((result ^ basis[bit]) < result) result ^= basis[bit];
    }
    return result;
  }

  void merge(const XorBasis& other) {
    for (Unsigned value : other.basis) {
      if (value != 0) insert(value);
    }
  }
};
