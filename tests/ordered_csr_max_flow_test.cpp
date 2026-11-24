#include "ordered-csr-max-flow.hpp"
#include "max-flow.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using C = long long;
struct Edge { int a, b; C cap; };

template<class E, class Fn>
void reject(Fn fn) {
  try { fn(); assert(false); }
  catch (const E&) {}
}

C brute_cut(int n, const std::vector<Edge>& edges, int s, int t) {
  C least = std::numeric_limits<C>::max();
  for (unsigned set = 0; set < (1U << n); ++set) {
    if (((set >> s) & 1U) == 0 || ((set >> t) & 1U) != 0) continue;
    C cap = 0;
    for (auto [a,b,w] : edges)
      if (((set >> a) & 1U) != 0 && ((set >> b) & 1U) == 0) cap += w;
    least = std::min(least, cap);
  }
  return least;
}

void compare(int n, const std::vector<Edge>& edges, int s, int t) {
  MaxFlow<C> baseline(n);
  ahc::OrderedCsrMaxFlow<C> ordered(n);
  for (auto [a,b,w] : edges) {
    const int i = ordered.add_edge(a,b,w);
    assert(i == baseline.add_edge(a,b,w));
  }
  ordered.freeze();
  auto residual_exact = [&] {
    auto x = ordered.edges(); const auto y = baseline.edges();
    assert(x.size() == y.size());
    for (std::size_t k=0;k<x.size();++k) {
      assert(x[k].from == y[k].from && x[k].to == y[k].to &&
             x[k].capacity == y[k].capacity && x[k].flow == y[k].flow);
    }
    for (int u=0;u<n;++u) assert(ordered.min_cut(u) == baseline.min_cut(u));
  };
  residual_exact();
  C sent = 0;
  for (C lim : std::array<C,5>{0,1,2,1,std::numeric_limits<C>::max()}) {
    const auto value = ordered.flow(s,t,lim);
    assert(value == baseline.flow(s,t,lim));
    sent += value;
    ordered.freeze();
    residual_exact();
  }
  assert(sent == brute_cut(n,edges,s,t));
  std::vector<C> net(static_cast<std::size_t>(n));
  for (const auto& e : ordered.edges()) {
    assert(e.flow >= 0 && e.flow <= e.capacity);
    net[static_cast<std::size_t>(e.from)] -= e.flow;
    net[static_cast<std::size_t>(e.to)] += e.flow;
  }
  for (int u=0;u<n;++u)
    assert(net[static_cast<std::size_t>(u)] == (u == s ? -sent : u == t ? sent : 0));
  // Swap terminals to exercise cancellation, including anti-parallel edges.
  assert(ordered.flow(t,s,3) == baseline.flow(t,s,3));
  residual_exact();
}

int main() {
  using G = ahc::OrderedCsrMaxFlow<C>;
  reject<std::invalid_argument>([]{ G x(-1); });
  G empty(0); empty.freeze(); assert(empty.frozen() && empty.edges().empty());
  reject<std::out_of_range>([&]{ (void)empty.min_cut(0); });
  G x(2);
  reject<std::logic_error>([&]{ (void)x.flow(0,1,0); });
  reject<std::logic_error>([&]{ (void)x.min_cut(0); });
  reject<std::logic_error>([&]{ (void)x.edges(); });
  reject<std::logic_error>([&]{ (void)x.get_edge(0); });
  reject<std::out_of_range>([&]{ (void)x.add_edge(-1,1,3); });
  reject<std::invalid_argument>([&]{ (void)x.add_edge(0,1,-2); });
  assert(x.add_edge(0,1,std::numeric_limits<C>::max()) == 0);
  x.freeze();
  reject<std::logic_error>([&]{ (void)x.add_edge(0,1,2); });
  reject<std::out_of_range>([&]{ (void)x.get_edge(1); });
  reject<std::invalid_argument>([&]{ (void)x.flow(0,0); });
  reject<std::invalid_argument>([&]{ (void)x.flow(0,1,-2); });
  assert(x.flow(0,1) == std::numeric_limits<C>::max() && x.flow(0,1) == 0);
  assert(x.get_edge(0).capacity == std::numeric_limits<C>::max());
  assert(x.flow(1,0) == std::numeric_limits<C>::max() && x.get_edge(0).flow == 0);
  for (bool reverse : {false, true}) {
    ahc::OrderedCsrMaxFlow<int> diamond(4);
    diamond.add_edge(0, reverse ? 2 : 1, 1);
    diamond.add_edge(0, reverse ? 1 : 2, 1);
    diamond.add_edge(1, 3, 1);
    diamond.add_edge(2, 3, 1);
    diamond.freeze();
    assert(diamond.flow(0, 3, 1) == 1);
    std::vector<int> witness;
    for (const auto& edge : diamond.edges()) witness.push_back(edge.flow);
    assert(witness == (reverse ? std::vector<int>{1, 0, 0, 1}
                               : std::vector<int>{1, 0, 1, 0}));
    assert((diamond.min_cut(0) == std::vector<char>{1, 1, 1, 1}));
    assert(diamond.flow(0, 3, 1) == 1 && diamond.flow(0, 3, 1) == 0);
    assert((diamond.min_cut(0) == std::vector<char>{1, 0, 0, 0}));
  }
  {
    ahc::OrderedCsrMaxFlow<signed char> small(2);
    const auto cap = std::numeric_limits<signed char>::max();
    small.add_edge(0, 1, cap);
    small.add_edge(0, 1, cap);
    small.freeze();
    assert(small.flow(0, 1) == cap && small.min_cut(0)[1]);
    assert(small.flow(0, 1) == cap && !small.min_cut(0)[1]);
  }
  compare(4,{},0,3);
  compare(4,{{0,0,9},{0,1,6},{1,0,2},{0,1,3},{1,3,8},{3,2,4},{0,2,1},{2,3,2}},0,3);
  compare(4,{{0,1,2},{0,2,2},{1,2,2},{1,3,2},{2,3,2}},0,3);
  std::mt19937 rng(702421);
  for (int trial=0;trial<2000;++trial) {
    const int n = 2 + static_cast<int>(rng()%6);
    const auto nv = static_cast<unsigned>(n);
    const auto pick = [&] { return static_cast<int>(rng()%nv); };
    std::vector<Edge> edges;
    const auto m = rng()%35;
    for (unsigned j=0;j<m;++j) edges.push_back({pick(),pick(),static_cast<C>(rng()%12)});
    const auto s=pick();
    compare(n,edges,s,(s+1+static_cast<int>(rng()%(nv-1)))%n);
  }
  std::cout << "OK: 2003 bounded graphs, residual order, exhaustive cuts, diamond and capacity limit\n";
}
