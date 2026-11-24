#include "mutable-grid-enter-mask.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using Grid = ahc::MutableGridEnterMask;
using Bytes = std::vector<unsigned char>;
using Words = std::vector<std::uint64_t>;

constexpr std::array<int, 4> dr{-1, 0, 1, 0};
constexpr std::array<int, 4> dc{0, 1, 0, -1};

int geometric(int h, int w, int p, unsigned d) {
  const int r = p / w + dr[d], c = p % w + dc[d];
  return r >= 0 && r < h && c >= 0 && c < w ? r * w + c : -1;
}
std::size_t ix(int i) { return static_cast<std::size_t>(i); }

Words pack(const Bytes& b) {
  Words out((b.size() + 63) / 64);
  for (std::size_t i = 0; i < b.size(); ++i) {
    if (b[i] != 0) out[i / 64] |= std::uint64_t{1} << (i % 64);
  }
  return out;
}

template <class Edge>
auto bfs(int n, int s, Edge edge) {
  std::vector<int> dist(ix(n), -1), first(ix(n), -1), q{s};
  dist[ix(s)] = 0;
  first[ix(s)] = s;
  for (std::size_t i = 0; i < q.size(); ++i) {
    const int u = q[i];
    for (unsigned d = 0; d < 4; ++d) {
      const int v = edge(u,d);
      if (v < 0 || dist[ix(v)] >= 0) continue;
      dist[ix(v)] = dist[ix(u)] + 1;
      first[ix(v)] = u == s ? v : first[ix(u)];
      q.push_back(v);
    }
  }
  return std::pair{dist, first};
}

std::size_t frames = 0, start_queries = 0;
void compare(const Grid& g, const Bytes& walls) {
  ++frames;
  for (int u = 0; u < g.size(); ++u) {
    assert(g.is_blocked(u) == (walls[ix(u)] != 0));
    unsigned mask = 0;
    for (unsigned d = 0; d < 4; ++d) {
      const int v = geometric(g.height(),g.width(),u,d);
      assert(g.neighbor(u,d) == v);
      if (v >= 0 && walls[ix(v)] == 0) mask |= 1U << d;
    }
    assert(g.out_mask(u) == mask);
    const auto oracle = bfs(g.size(),u,[&](int p,unsigned d){
      const int v = geometric(g.height(),g.width(),p,d);
      return v >= 0 && walls[ix(v)] == 0 ? v : -1;
    });
    const auto fast = bfs(g.size(),u,[&](int p,unsigned d){
      return (g.out_mask(p) & (1U << d)) != 0 ? g.neighbor(p,d) : -1;
    });
    assert(oracle == fast);
    ++start_queries;
  }
}

template<class Exception, class Fn>
void reject(Fn fn) {
  try { fn(); assert(false); }
  catch (const Exception&) {}
}

int main() {
  reject<std::invalid_argument>([]{ Grid g(0, 1); });
  reject<std::invalid_argument>([]{ Grid g(1, -1); });
  reject<std::invalid_argument>([]{ Grid g(std::numeric_limits<int>::max(), 2); });
  {
    Grid g(1,3);
    reject<std::out_of_range>([&]{ (void)g.neighbor(0,4); });
    reject<std::out_of_range>([&]{ (void)g.out_mask(-1); });
    reject<std::out_of_range>([&]{ (void)g.is_blocked(3); });
    Bytes walls{0,1,0};
    g.update(pack(walls));
    assert(g.out_mask(1) == ((1U << Grid::right) | (1U << Grid::left)));
    assert(g.out_mask(0) == 0 && g.out_mask(2) == 0);
    compare(g,walls);
    // Same source loses one edge and gains another in one snapshot.
    walls = {1,1,0};g.update(pack(walls));
    walls = {0,1,1};g.update(pack(walls));compare(g,walls);
    g.update(pack(walls));compare(g,walls);
  }
  {
    Grid g(1,65);
    Bytes walls(65,0);
    walls[63]=1;walls[64]=1;
    g.update(pack(walls));
    reject<std::invalid_argument>([&]{ g.update(Words{0}); });
    reject<std::invalid_argument>([&]{ g.update(Words{0,0,0}); });
    // A real change in the first word must not be applied if tail is invalid.
    reject<std::invalid_argument>([&]{ g.update(Words{0,3}); });
    compare(g,walls);
    walls[63]=0;walls[64]=0;walls[0]=1;
    g.update(pack(walls));compare(g,walls);
  }
  std::mt19937 rng(905731);
  constexpr std::array dimensions{std::pair{1,1},std::pair{1,63},
    std::pair{1,64},std::pair{1,65},std::pair{63,1},std::pair{64,1},
    std::pair{65,1},std::pair{2,63},std::pair{2,64},std::pair{2,65},
    std::pair{4,5},std::pair{7,9},std::pair{8,8},std::pair{5,13}};
  for (auto [h,w] : dimensions) {
    Grid g(h,w);
    Bytes walls(ix(g.size()),0);
    compare(g,walls);
    for (unsigned t=0;t<48;++t) {
      if (t==0 || t==1) {
        walls.assign(walls.size(),static_cast<unsigned char>(1U-t));
      } else {
        // Deliberate duplicate toggles before publishing each final snapshot.
        const auto pos=static_cast<std::size_t>(rng()) % walls.size();
        walls[pos]^=1;walls[pos]^=1;
        for (unsigned k=0;k<t%13;++k) {
          const auto q=static_cast<std::size_t>(rng()) % walls.size();
          walls[q]^=1;
        }
      }
      g.update(pack(walls));compare(g,walls);
    }
  }
  std::cout << "OK frames=" << frames << " all-source exact BFS comparisons="
            << start_queries << '\n';
}
