#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ahc {

// Integer Dinic for a single, explicitly frozen edge list. Residual capacities
// still change during flow(); only the locations and order of arcs are fixed.
template <std::signed_integral Capacity>
class OrderedCsrMaxFlow {
 public:
  struct EdgeInfo {
    int from, to;
    Capacity capacity, flow;
  };

  explicit OrderedCsrMaxFlow(int vertex_count) : n_(vertex_count) {
    if (n_ < 0) throw std::invalid_argument("negative vertex count");
  }

  [[nodiscard]] int vertex_count() const noexcept { return n_; }
  [[nodiscard]] bool frozen() const noexcept { return frozen_; }

  int add_edge(int from, int to, Capacity capacity) {
    if (frozen_) throw std::logic_error("cannot append after freeze");
    check_vertex(from);
    check_vertex(to);
    if (capacity < Capacity{}) throw std::invalid_argument("negative capacity");
    if (pending_.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max())) {
      throw std::length_error("edge IDs exceed int range");
    }
    const int id = static_cast<int>(pending_.size());
    pending_.push_back({from, to, capacity});
    return id;
  }

  // Call once after appending; a subsequent call is a no-op, not a flow reset.
  void freeze() {
    if (frozen_) return;
    const auto nv = static_cast<std::size_t>(n_);
    const auto ne = pending_.size();
    std::vector<Arc> arc;
    std::vector<std::size_t> offset;
    if (ne > arc.max_size() / 2 || nv >= offset.max_size()) {
      throw std::length_error("CSR dimensions too large");
    }
    offset.resize(nv + 1, 0);
    for (const auto& e : pending_) {
      ++offset[ix(e.from) + 1];
      ++offset[ix(e.to) + 1];
    }
    for (std::size_t i = 0; i < nv; ++i) offset[i + 1] += offset[i];
    arc.resize(ne * 2);
    std::vector<Location> location(ne);
    auto cursor = offset;
    for (std::size_t i = 0; i < ne; ++i) {
      const auto& e = pending_[i];
      const auto a = cursor[ix(e.from)]++;
      const auto b = cursor[ix(e.to)]++;
      arc[a] = {e.to, b, e.capacity};
      arc[b] = {e.from, a, Capacity{}};
      location[i] = {e.from, a};
    }
    std::vector<int> level(nv), queue(nv);
    offset_.swap(offset);
    arc_.swap(arc);
    location_.swap(location);
    cursor_.swap(cursor);
    level_.swap(level);
    queue_.swap(queue);
    std::vector<Pending>().swap(pending_);
    frozen_ = true;
  }

  [[nodiscard]] EdgeInfo get_edge(int id) const {
    check_frozen();
    if (id < 0 || ix(id) >= location_.size()) throw std::out_of_range("invalid edge ID");
    const auto place = location_[ix(id)];
    const auto& edge = arc_[place.slot];
    const Capacity sent = arc_[edge.reverse].remaining;
    return {place.from, edge.to, static_cast<Capacity>(edge.remaining + sent), sent};
  }

  [[nodiscard]] std::vector<EdgeInfo> edges() const {
    check_frozen();
    std::vector<EdgeInfo> result;
    result.reserve(location_.size());
    for (std::size_t i = 0; i < location_.size(); ++i)
      result.push_back(get_edge(static_cast<int>(i)));
    return result;
  }

  // Push up to limit additional units into the current residual graph.
  Capacity flow(int source, int sink,
                Capacity limit = std::numeric_limits<Capacity>::max()) {
    check_frozen();
    check_vertex(source);
    check_vertex(sink);
    if (source == sink || limit < Capacity{}) {
      throw std::invalid_argument("invalid flow terminals or limit");
    }
    Capacity total{};
    while (total < limit && build_levels(source, sink)) {
      std::copy(offset_.begin(), offset_.end(), cursor_.begin());
      while (total < limit) {
        const Capacity pushed = send(source, sink, static_cast<Capacity>(limit - total));
        if (pushed == Capacity{}) break;
        total = static_cast<Capacity>(total + pushed);
      }
    }
    return total;
  }

  // Residual reachability; represents an s-t minimum cut only once no
  // augmenting path from s to the intended t remains.
  [[nodiscard]] std::vector<char> min_cut(int source) const {
    check_frozen();
    check_vertex(source);
    std::vector<char> seen(ix(n_), 0);
    std::vector<int> todo;
    todo.reserve(ix(n_));
    todo.push_back(source);
    seen[ix(source)] = 1;
    for (std::size_t i = 0; i < todo.size(); ++i) {
      const auto u = ix(todo[i]);
      for (std::size_t j = offset_[u]; j < offset_[u + 1]; ++j) {
        const auto& e = arc_[j];
        if (e.remaining == Capacity{} || seen[ix(e.to)] != 0) continue;
        seen[ix(e.to)] = 1;
        todo.push_back(e.to);
      }
    }
    return seen;
  }

 private:
  struct Pending { int from, to; Capacity capacity; };
  struct Arc { int to; std::size_t reverse; Capacity remaining; };
  struct Location { int from; std::size_t slot; };

  static std::size_t ix(int v) noexcept { return static_cast<std::size_t>(v); }
  void check_vertex(int v) const {
    if (v < 0 || v >= n_) throw std::out_of_range("invalid vertex");
  }
  void check_frozen() const {
    if (!frozen_) throw std::logic_error("call freeze before solving or querying");
  }

  bool build_levels(int source, int sink) {
    std::fill(level_.begin(), level_.end(), -1);
    std::size_t end = 1;
    queue_[0] = source;
    level_[ix(source)] = 0;
    for (std::size_t i = 0; i < end; ++i) {
      const auto u = ix(queue_[i]);
      for (std::size_t j = offset_[u]; j < offset_[u + 1]; ++j) {
        const auto& e = arc_[j];
        const auto v = ix(e.to);
        if (e.remaining == Capacity{} || level_[v] >= 0) continue;
        level_[v] = level_[u] + 1;
        queue_[end++] = e.to;
      }
    }
    return level_[ix(sink)] >= 0;
  }

  Capacity send(int u, int sink, Capacity limit) {
    if (u == sink) return limit;
    const auto p = ix(u);
    for (auto& i = cursor_[p]; i < offset_[p + 1]; ++i) {
      auto& e = arc_[i];
      if (e.remaining == Capacity{} || level_[ix(e.to)] != level_[p] + 1) continue;
      const Capacity moved = send(e.to, sink, std::min(limit, e.remaining));
      if (moved == Capacity{}) continue;
      e.remaining = static_cast<Capacity>(e.remaining - moved);
      auto& reverse = arc_[e.reverse];
      reverse.remaining = static_cast<Capacity>(reverse.remaining + moved);
      return moved;
    }
    return Capacity{};
  }

  int n_;
  bool frozen_ = false;
  std::vector<Pending> pending_;
  std::vector<std::size_t> offset_, cursor_;
  std::vector<Arc> arc_;
  std::vector<Location> location_;
  std::vector<int> level_, queue_;
};

}  // namespace ahc
