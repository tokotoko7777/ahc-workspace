#include <cassert>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/dense-int-set.hpp

// 0以上universe_size未満の整数集合。追加・削除・検索・clearが平均O(1)。
// 使い方:
// DenseIntSet set(universe_size);
// set.insert(value); set.erase(value);
// for (int i = 0; i < set.size(); ++i) use(set[i]);
struct DenseIntSet {
  int universe_size;
  int count = 0;
  std::vector<int> dense;
  std::vector<int> position;

  explicit DenseIntSet(int universe_size)
      : universe_size(universe_size) {
    assert(universe_size >= 0);
    dense.resize(universe_size);
    position.assign(universe_size, 0);
  }

  int size() const { return count; }
  bool empty() const { return count == 0; }

  bool contains(int value) const {
    assert(0 <= value && value < universe_size);
    const int index = position[value];
    return index < count && dense[index] == value;
  }

  bool insert(int value) {
    assert(0 <= value && value < universe_size);
    if (contains(value)) return false;
    dense[count] = value;
    position[value] = count;
    ++count;
    return true;
  }

  bool erase(int value) {
    assert(0 <= value && value < universe_size);
    if (!contains(value)) return false;
    const int index = position[value];
    const int last = dense[count - 1];
    dense[index] = last;
    position[last] = index;
    --count;
    return true;
  }

  void clear() { count = 0; }

  int operator[](int index) const {
    assert(0 <= index && index < count);
    return dense[index];
  }

  // 乱数で [0,size()) のindexを作り、ここに渡すとランダムな要素を取れる。
  int at_random_index(int index) const { return (*this)[index]; }
};
