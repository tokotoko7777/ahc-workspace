#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/binary-trie.hpp

// 非負整数の多重集合。追加・削除・最小/最大XOR要素を O(BitCount) で行う。
template <class Unsigned,
          int BitCount = std::numeric_limits<Unsigned>::digits>
struct BinaryTrie {
  static_assert(std::is_unsigned<Unsigned>::value,
                "BinaryTrie requires an unsigned integer type");
  static_assert(1 <= BitCount &&
                    BitCount <= std::numeric_limits<Unsigned>::digits,
                "BitCount is out of range");

  struct Node {
    int child[2]{-1, -1};
    int count = 0;
  };

  std::vector<Node> nodes{Node{}};

  int size() const { return nodes[0].count; }
  bool empty() const { return size() == 0; }

  void reserve(int expected_values) {
    assert(expected_values >= 0);
    nodes.reserve(1 + static_cast<std::size_t>(expected_values) * BitCount);
  }

  void insert(Unsigned value) {
    int node = 0;
    ++nodes[node].count;
    for (int bit = BitCount - 1; bit >= 0; --bit) {
      const int direction = static_cast<int>((value >> bit) & 1);
      if (nodes[node].child[direction] == -1) {
        nodes[node].child[direction] = static_cast<int>(nodes.size());
        nodes.push_back(Node{});
      }
      node = nodes[node].child[direction];
      ++nodes[node].count;
    }
  }

  int count(Unsigned value) const {
    int node = 0;
    for (int bit = BitCount - 1; bit >= 0; --bit) {
      const int direction = static_cast<int>((value >> bit) & 1);
      node = nodes[node].child[direction];
      if (node == -1) return 0;
    }
    return nodes[node].count;
  }

  bool erase(Unsigned value) {
    if (count(value) == 0) return false;
    int node = 0;
    --nodes[node].count;
    for (int bit = BitCount - 1; bit >= 0; --bit) {
      const int direction = static_cast<int>((value >> bit) & 1);
      node = nodes[node].child[direction];
      --nodes[node].count;
    }
    return true;
  }

  Unsigned minimum_xor_element(Unsigned value) const {
    return xor_element(value, false);
  }

  Unsigned maximum_xor_element(Unsigned value) const {
    return xor_element(value, true);
  }

  Unsigned minimum_xor_value(Unsigned value) const {
    return value ^ minimum_xor_element(value);
  }

  Unsigned maximum_xor_value(Unsigned value) const {
    return value ^ maximum_xor_element(value);
  }

 private:
  Unsigned xor_element(Unsigned value, bool maximize) const {
    assert(!empty());
    int node = 0;
    Unsigned result = 0;
    for (int bit = BitCount - 1; bit >= 0; --bit) {
      const int value_bit = static_cast<int>((value >> bit) & 1);
      const int preferred = value_bit ^ static_cast<int>(maximize);
      int direction = preferred;
      int next = nodes[node].child[direction];
      if (next == -1 || nodes[next].count == 0) {
        direction ^= 1;
        next = nodes[node].child[direction];
      }
      assert(next != -1 && nodes[next].count > 0);
      if (direction != 0) result |= (Unsigned{1} << bit);
      node = next;
    }
    return result;
  }
};
