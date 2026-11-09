#include <array>
#include <cassert>
#include <cstddef>
#include <utility>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/fixed-vector.hpp

// 最大要素数がコンパイル時に決まる、allocationなしの簡単な配列。
// T はデフォルト構築できる型にする。
//
// 使い方:
// FixedVector<int, 100> values;
// values.push_back(42);
template <class T, std::size_t Capacity>
struct FixedVector {
  std::array<T, Capacity> data;
  std::size_t length = 0;

  bool empty() const { return length == 0; }
  std::size_t size() const { return length; }
  constexpr std::size_t capacity() const { return Capacity; }

  void clear() { length = 0; }

  void push_back(const T& value) {
    assert(length < Capacity);
    data[length++] = value;
  }

  void push_back(T&& value) {
    assert(length < Capacity);
    data[length++] = std::move(value);
  }

  template <class... Args>
  T& emplace_back(Args&&... args) {
    assert(length < Capacity);
    data[length] = T(std::forward<Args>(args)...);
    return data[length++];
  }

  void pop_back() {
    assert(!empty());
    --length;
  }

  T& operator[](std::size_t index) {
    assert(index < length);
    return data[index];
  }

  const T& operator[](std::size_t index) const {
    assert(index < length);
    return data[index];
  }

  T& back() {
    assert(!empty());
    return data[length - 1];
  }

  const T& back() const {
    assert(!empty());
    return data[length - 1];
  }

  T* begin() { return data.data(); }
  T* end() { return data.data() + length; }
  const T* begin() const { return data.data(); }
  const T* end() const { return data.data() + length; }
};
