#include <array>
#include <cassert>
#include <queue>
#include <stdexcept>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/aho-corasick.hpp

// 複数パターンをまとめて探す。文字を0..Alphabet-1の整数へ変換して使う。
// 例: AhoCorasick<3> ac; ac.add({0,1}, 7); ac.build();
// int state=0; for (int c : {0,1,0}) {
//   state=ac.advance(state,c); for(int id:ac.matches(state)) { /* id=7が一致 */ }
// }
// addはbuild前だけ。空パターンは不可。同じパターン・IDの重複登録は許可し、
// matchesにも重複を残す。全登録完了後buildを1回呼ぶ（再呼び出しは無害）。
// 検索O(入力長+一致数)、build O(節点数*Alphabet+展開した出力ID数)。
// suffixの出力IDを各節点へ複製するため、多数の包含語ではメモリに注意。
template <int Alphabet = 26>
struct AhoCorasick {
  static_assert(Alphabet > 0, "positive alphabet required");
  struct Node {
    std::array<int, Alphabet> next;
    int failure = 0;
    std::vector<int> output;
    Node() { next.fill(-1); }
  };
  std::vector<Node> nodes{1};
  bool built = false;

  void add(const std::vector<int>& word, int id) {
    if (built || word.empty()) throw std::invalid_argument("add nonempty patterns before build");
    for (int c : word) {
      if (c < 0 || c >= Alphabet) throw std::out_of_range("symbol outside alphabet");
    }
    int state = 0;
    for (int c : word) {
      if (nodes[state].next[c] < 0) {
        const int child = static_cast<int>(nodes.size());
        nodes[state].next[c] = child;
        nodes.emplace_back();
      }
      state = nodes[state].next[c];
    }
    nodes[state].output.push_back(id);
  }

  void build() {
    if (built) return;
    std::queue<int> queue;
    for (int c = 0; c < Alphabet; ++c) {
      int& child = nodes[0].next[c];
      if (child < 0) child = 0;
      else queue.push(child);
    }
    while (!queue.empty()) {
      const int state = queue.front();
      queue.pop();
      const int failure = nodes[state].failure;
      const auto& inherited = nodes[failure].output;
      nodes[state].output.insert(nodes[state].output.end(), inherited.begin(), inherited.end());
      for (int c = 0; c < Alphabet; ++c) {
        int& child = nodes[state].next[c];
        if (child < 0) child = nodes[failure].next[c];
        else {
          nodes[child].failure = nodes[failure].next[c];
          queue.push(child);
        }
      }
    }
    built = true;
  }

  int advance(int state, int symbol) const {
    assert(built && state >= 0 && state < static_cast<int>(nodes.size()));
    assert(symbol >= 0 && symbol < Alphabet);
    return nodes[state].next[symbol];
  }
  const std::vector<int>& matches(int state) const {
    assert(built && state >= 0 && state < static_cast<int>(nodes.size()));
    return nodes[state].output;
  }
};
