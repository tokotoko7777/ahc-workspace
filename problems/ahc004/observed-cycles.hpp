#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace ahc004 {
struct Cycle {
  std::string letters;
  std::vector<int> words;
  int evidence = 0;
};
struct CycleCatalogue {
  std::vector<Cycle> cycles;
  int visits = 0;
  bool capped = false;
};

// Each output has N consecutive observed (k+1)-mers, including the closure.
// The observation graph can contain spurious cycles: rank as hypotheses only.
inline CycleCatalogue observed_cycles(
    int n, const std::vector<std::string>& words,
    const std::vector<int>& frequencies, int budget = 16000,
    const std::function<bool()>& deadline = [] { return false; }) {
  CycleCatalogue out;
  std::map<std::string, int> unique;
  if (n != 20 || budget <= 0 || deadline()) return out;
  for (const int k : {5, 4}) {
    std::vector<uint8_t> graph(1 << (3*k));
    const int mask = (1 << (3*k)) - 1;
    for (const auto& word : words) {
      int state = 0;
      for (int i = 0; i < int(word.size()); ++i) {
        const int letter = word[i] - 'A';
        if (i >= k) graph[state] |= 1 << letter;
        state = ((state << 3) | letter) & mask;
      }
    }
    const int layer_budget = out.visits + budget / 2;
    bool stop = false;
    for (int start = 0; start <= mask && !stop; ++start) {
      if (!graph[start]) continue;
      std::string append(n, 'A');
      const auto dfs = [&](const auto& self, int state, int depth) -> void {
        if (stop) return;
        if (++out.visits >= layer_budget ||
            ((out.visits & 255) == 0 && deadline())) {
          out.capped = stop = true;
          return;
        }
        if (depth == n) {
          if (state != start) return;
          // start is a minimal k-mer on this walk; ties may correspond to
          // different rotations, so also canonicalize the complete sequence.
          std::string canonical = append;
          for (int p = 1; p < n; ++p)
            canonical = std::min(canonical, append.substr(p) + append.substr(0,p));
          unique[canonical] = std::max(unique[canonical], k);
          return;
        }
        if ((graph[state] == 0) || (depth && state < start)) return;
        for (int c = 0; c < 8; ++c) if (graph[state] >> c & 1) {
          append[depth] = char('A' + c);
          self(self, ((state << 3) | c) & mask, depth + 1);
        }
      };
      dfs(dfs, start, 0);
    }
    if (deadline()) {
      out.capped = true;
      break;
    }
  }
  for (const auto& [letters, k] : unique) {
    if (deadline()) { out.capped = true; break; }
    Cycle cycle{letters, {}, 0};
    const std::string wrapped = letters + letters.substr(0,n-1);
    for (int i = 0; i < int(words.size()); ++i)
      if (wrapped.find(words[i]) < size_t(n)) {
        cycle.words.push_back(i);
        const int length = words[i].size();
        cycle.evidence += (length-1)*(length-1)*std::min(3, frequencies[i]);
      }
    // Prefer six-seed-letter support in otherwise similar candidates.
    cycle.evidence += 5 * (k == 5);
    out.cycles.push_back(std::move(cycle));
  }
  std::sort(out.cycles.begin(), out.cycles.end(), [](const Cycle& a, const Cycle& b) {
    return a.evidence != b.evidence ? a.evidence > b.evidence : a.letters < b.letters;
  });
  return out;
}
}  // namespace ahc004
