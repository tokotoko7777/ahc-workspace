#pragma once
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ahc060 {
struct Relocation {
  std::vector<int> actions;
  int before, after;
};
inline Relocation relocate(const std::vector<std::vector<int>>& graph, int shops,
                           int limit, const std::vector<int>& actions,
                           std::size_t work = 2000000) {
  struct Visit { int move, word; std::size_t pos; };
  struct Delivery { int shop; std::string word; };
  const int n = int(graph.size());
  if (shops <= 0 || shops > n || limit < 0 || actions.size() > std::size_t(limit))
    throw std::invalid_argument("invalid instance or action budget");
  std::vector<int> moves, boundary(n, -1);
  std::vector<std::vector<Visit>> visits(n);
  std::vector<Delivery> words;
  std::string cone;
  int at = 0, prev = -1;
  for (int v : actions) {
    if (v == -1) {
      if (at < shops || visits[at].empty() || boundary[at] != -1)
        throw std::invalid_argument("illegal flip");
      boundary[at] = int(visits[at].size())-1;
      continue;
    }
    if (v < 0 || v >= n || v == prev ||
        std::find(graph[at].begin(),graph[at].end(),v)==graph[at].end())
      throw std::invalid_argument("illegal move");
    const int ix = int(moves.size());
    moves.push_back(v);
    prev = at; at = v;
    if (at < shops) {
      words.push_back({at,std::move(cone)});
      cone.clear();
    } else {
      visits[at].push_back({ix,int(words.size()),cone.size()});
      cone.push_back(boundary[at] < 0 ? 'W':'R');
    }
  }
  std::vector<std::unordered_map<std::string,int>> stock(shops);
  int score = 0;
  for (auto& d : words) if (++stock[d.shop][d.word] == 1) ++score;
  const int before = score;
  auto change = [&](const Visit& visit, char color) {
    if (visit.word == int(words.size())) return; // Undelivered suffix.
    auto& [shop,word] = words[visit.word];
    auto& counts = stock[shop];
    auto old = counts.find(word);
    if (--old->second == 0) { counts.erase(old); --score; }
    word[visit.pos] = color;
    if (++counts[word] == 1) ++score;
  };
  for (int tree = shops; tree < n; ++tree) {
    int& cur = boundary[tree];
    const auto& seq = visits[tree];
    if (cur < 0 || seq.size() <= 1) continue;
    // At most three crossings of each visit, including restoration.
    // Charge string length for exact map keys; never partially scan a tree.
    std::size_t fee = 0;
    bool fits = true;
    for (const auto& v : seq) {
      const auto cost = v.word == int(words.size()) ? 1 : words[v.word].word.size()+1;
      if (cost > (work-fee)/3) { fits = false; break; }
      fee += 3*cost;
    }
    if (!fits) continue;
    work -= fee;
    const int baseline = score;
    int best = cur, best_score = score;
    auto remember = [&] { if (score > best_score || (score == best_score && score > baseline && cur < best)) { best = cur; best_score = score; } };
    while (cur > 0) { change(seq[cur--], 'R'); remember(); }
    while (cur+1 < int(seq.size())) { change(seq[++cur], 'W'); remember(); }
    while (cur > best) change(seq[cur--], 'R');
  }
  std::vector<int> out;
  out.reserve(actions.size());
  for (int i = 0; i < int(moves.size()); ++i) {
    const int v = moves[i];
    out.push_back(v);
    if (boundary[v] >= 0 && visits[v][boundary[v]].move == i) out.push_back(-1);
  }
  if (out.size() != actions.size()) throw std::logic_error("changed action count");
  return {std::move(out),before,score};
}
} // namespace ahc060
