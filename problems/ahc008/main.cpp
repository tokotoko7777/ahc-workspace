#pragma GCC optimize("O3")
#include <bits/stdc++.h>
#include "../../library/flat-grid-first-step-bfs.hpp"
#include "../../library/vertex-removal-components.hpp"
using namespace std;

#ifndef AHC008_ADAPTIVE
#define AHC008_ADAPTIVE 1
#endif

namespace {
constexpr int S = 30, CELLS = S * S;
constexpr int DR[] = {-1, 1, 0, 0}, DC[] = {0, 0, -1, 1};
constexpr string_view moves = "UDLR", builds = "udlr";
int adjacent(int p, int d) {
  int r = p / S + DR[d], c = p % S + DC[d];
  return 0 <= r && r < S && 0 <= c && c < S ? r * S + c : -1;
}
int direction(int a, int b) {
  for (int d = 0; d < 4; ++d) if (adjacent(a, d) == b) return d;
  assert(false);
  return 0;
}
struct Rect {
  array<int, 2> lo{0, 0}, hi{S-1, S-1};
  int area() const { return (hi[0] - lo[0] + 1) * (hi[1] - lo[1] + 1); }
  bool contains(int p) const {
    const int r = p / S, c = p % S;
    return lo[0] <= r && r <= hi[0] && lo[1] <= c && c <= hi[1];
  }
};
struct Cut {
  Rect keep;
  int axis, line, side, gate, gate_on_keep, started;
  vector<pair<int,int>> jobs; // (wall cell, builder stance)
};
Cut make_cut(const Rect& active, int axis, int line, int side, int turn) {
  Cut cut{active, axis, line, side, -1, -1, turn, {}};
  if (side > 0) cut.keep.lo[axis] = line + 1;
  else cut.keep.hi[axis] = line - 1;
  const int cross = 1 - axis, mid = (active.lo[cross] + active.hi[cross]) / 2;
  for (int j = active.lo[cross]; j <= active.hi[cross]; ++j) {
    int cell = axis == 0 ? line*S+j : j*S+line;
    int stance = cell + side * (axis == 0 ? S : 1);
    if (j == mid) cut.gate = cell, cut.gate_on_keep = stance;
    else cut.jobs.emplace_back(cell, stance);
  }
  return cut;
}

#if defined(AHC008_PARALLEL_PENS)
#include "parallel-pens.hpp"
#endif

class Game {
 public:
  void read() {
    int n, m;
    cin >> n;
    for (int i = 0; i < n; ++i) {
      int r, c, t;
      cin >> r >> c >> t;
      pets.push_back((r-1)*S+c-1);
    }
    cin >> m;
    for (int i = 0; i < m; ++i) {
      int r, c;
      cin >> r >> c;
      humans.push_back((r-1)*S+c-1);
    }
    cut = make_cut(active, 0, 9, 1, 0);
  }

  string plan(int turn) {
#ifdef AHC008_PARALLEL_PENS
    return pens.plan(humans, pets, walls);
#endif
#if defined(AHC008_EXACT_ENDGAME)
#if defined(AHC008_EXACT_PAIR)
    if (turn >= 270) {
      string instant = exact_endgame(false, true);
      if (instant.find_first_of(builds) != string::npos) endgame_only = true;
      if (endgame_only || instant.find_first_not_of('.') != string::npos) return instant;
    }
#elif defined(AHC008_EXACT_REPLACE)
    if (turn >= 270) return exact_endgame(true);
#else
    if (turn >= 195) {
      string instant = exact_endgame(endgame_only || (!cut && turn >= 270));
      if (instant.find_first_of(builds) != string::npos) endgame_only = true;
      if (endgame_only || instant.find_first_not_of('.') != string::npos) return instant;
    }
#endif
#endif
    string command(humans.size(), '.');
    if (!cut && AHC008_ADAPTIVE && turn < 270) cut = choose_cut(turn);
    if (!cut) return command;
    auto& c = *cut;
    bitset<CELLS> no_build, placed, moved, claimed;
    for (int p : pets) {
      no_build[p] = true;
      for (int d = 0; d < 4; ++d) {
        const int q = adjacent(p, d);
        if (q >= 0) no_build[q] = true;
      }
    }
    for (int h : humans) no_build[h] = true;
    bool unfinished = false;
    for (auto [wall, stance] : c.jobs) unfinished |= !walls[wall];
    if (!unfinished) {
      bool all_staged = ranges::all_of(humans, [&](int h) { return c.keep.contains(h); });
      int prior = pet_count(active), left = pet_count(c.keep);
      // Closing strictly improves all humans' immediate score, even counting
      // the divider as previously open; wait briefly for a larger catch.
      const int removed = prior - left;
      if (all_staged && !no_build[c.gate] && removed > 0 &&
          (int64_t(c.keep.area()) << removed) > active.area() &&
          (removed >= max(1, prior / 3) || turn - c.started >= 60 || turn >= 290)) {
        for (int i = 0; i < int(humans.size()); ++i) if (humans[i] == c.gate_on_keep) {
          command[i] = builds[direction(humans[i], c.gate)];
          active = c.keep;
          cut.reset();
          return command;
        }
      }
    }

    for (int i = 0; i < int(humans.size()); ++i) {
      int h = humans[i], target = -1, stance = c.gate_on_keep;
      // Already legal adjacent walls take priority; reserving movement and
      // walls makes this sequential construction a legal simultaneous turn.
      for (auto [wall, worker] : c.jobs) {
        if (worker == h && !walls[wall] && !placed[wall] && !no_build[wall] && !moved[wall]) {
          target = wall;
          break;
        }
      }
      if (target >= 0) {
        command[i] = builds[direction(h, target)];
        claimed[target] = placed[target] = true;
        continue;
      }
      bfs.run(h, [&](int p) { return !walls[p] && !placed[p]; });
      int priority = INT_MAX;
      for (auto [wall, worker] : c.jobs) {
        if (walls[wall] || claimed[wall]) continue;
        int d = bfs.distance(worker);
        if (d < 0) continue;
        int key = d + (no_build[wall] ? 4 : 0);
        if (key < priority) priority = key, target = wall, stance = worker;
      }
      if (target >= 0) claimed[target] = true;
      const int next = bfs.first_step(stance);
      if (next >= 0 && next != h) {
        command[i] = moves[direction(h, next)];
        moved[next] = true;
      }
    }
    return command;
  }

  void apply(const string& action) {
    for (int i = 0; i < int(humans.size()); ++i) {
      auto b = builds.find(action[i]), m = moves.find(action[i]);
      if (b != string_view::npos) walls[adjacent(humans[i], b)] = true;
      if (m != string_view::npos) humans[i] = adjacent(humans[i], m);
    }
  }

  bool read_pets() {
    for (int& p : pets) {
      string steps;
      if (!(cin >> steps)) return false;
      for (char ch : steps) {
        auto m = moves.find(ch);
        if (m != string_view::npos) {
          p = adjacent(p, m);
          assert(p >= 0 && !walls[p]);
        }
      }
    }
    return true;
  }

 private:
#if defined(AHC008_EXACT_ENDGAME)
  struct Counts {
    int h=0,p=0;
    Counts& operator+=(const Counts& b) { h+=b.h; p+=b.p; return *this; }
    Counts& operator-=(const Counts& b) { h-=b.h; p-=b.p; return *this; }
  };
  string exact_endgame(bool permit_approach, bool use_pairs = false) {
    string cmd(humans.size(), '.');
    array<Counts,CELLS> counts{};
    bitset<CELLS> forbidden;
    for (int p : pets) {
      ++counts[p].p;
      forbidden[p] = true;
      for (int q : close_neighbors[p]) forbidden[q] = true;
    }
    for (int h : humans) ++counts[h].h, forbidden[h] = true;
    end_cuts.build([&](int v) -> const auto& { return close_neighbors[v]; },
              [&](int v) { return !walls[v]; }, [&](int v) { return counts[v]; });
    // A blocked vertex cannot contain a pet or human; weights add exactly.
    int64_t best_gain = 0;
    int best = -1;
    for (int gate = 0; gate < CELLS; ++gate) {
      if (walls[gate] || forbidden[gate] || !end_cuts.is_cut(gate)) continue;
      Counts old;
      int area = 1;
      int64_t value = 0;
      end_cuts.after_removing(gate, [&](int, int area2, Counts c) {
        old += c;
        area += area2;
        value += (int64_t(area2) * c.h) << (20-c.p);
      });
      if (!old.h || !old.p) continue;
      const int64_t gain = value - ((int64_t(area) * old.h) << (20-old.p));
      if (gain <= best_gain) continue;
      if (!permit_approach && ranges::none_of(close_neighbors[gate],
              [&](int u){ return counts[u].h > 0; })) continue;
      best_gain = gain, best = gate;
    }
    if (use_pairs) {
      vector<pair<int,unsigned>> candidates;
      for (int i = 0; i < int(humans.size()); ++i) {
        for (int a : close_neighbors[humans[i]]) {
          if (walls[a] || forbidden[a] || end_cuts.is_cut(a)) continue;
          auto existing = ranges::find(candidates, a, &pair<int,unsigned>::first);
          if (existing == candidates.end()) candidates.emplace_back(a, 1u << i);
          else existing->second |= 1u << i;
        }
      }
      pair<int,int> joint{-1,-1}, who2;
      for (int i = 0; i + 1 < int(candidates.size()); ++i) {
        const auto [a, a_workers] = candidates[i];
        pair_cuts.build([&](int v) -> const auto& { return close_neighbors[v]; },
                  [&](int v) { return v != a && !walls[v]; }, [&](int v) { return counts[v]; });
        for (int j = i+1; j < int(candidates.size()); ++j) {
          const auto [b, b_workers] = candidates[j];
          if (popcount(a_workers | b_workers) < 2 || !pair_cuts.is_cut(b)
                   || end_cuts.side_of(a,b) == -1) continue;
          Counts old;
          int area = 2;
          int64_t value = 0;
          pair_cuts.after_removing(b, [&](int, int area2, Counts c) {
            old += c;
            area += area2;
            value += (int64_t(area2) * c.h) << (20-c.p);
          });
          if (!old.h || !old.p) continue;
          const int64_t gain = value - ((int64_t(area) * old.h) << (20-old.p));
          if (gain > best_gain) {
            const int aw = countr_zero((b_workers & (b_workers - 1)) ? a_workers : a_workers & ~b_workers);
            const int bw = countr_zero(b_workers & ~(1u << aw));
            best_gain = gain;
            joint = {a,b};
            who2 = {aw,bw};
          }
        }
      }
      if (joint.first >= 0) {
        cmd[who2.first] = builds[direction(humans[who2.first], joint.first)];
        cmd[who2.second] = builds[direction(humans[who2.second], joint.second)];
        return cmd;
      }
    }
    if (best < 0) return cmd;
    int who = -1, dmin = INT_MAX, step = -1;
    for (int i = 0; i < int(humans.size()); ++i) {
      int h = humans[i];
      for (int x : close_neighbors[best]) if (x == h) {
        cmd[i] = builds[direction(h,best)];
        return cmd;
      }
      if (!permit_approach) continue;
      bfs.run(h, [&](int p){ return p != best && !walls[p]; });
      for (int x : close_neighbors[best]) {
        int d = bfs.distance(x);
        if (d >= 0 && d < dmin) who = i, step = bfs.first_step(x), dmin = d;
      }
    }
    if (who >= 0) cmd[who] = moves[direction(humans[who],step)];
    return cmd;
  }
  static inline const array<vector<int>,CELLS> close_neighbors = [] {
    array<vector<int>,CELLS> ans;
    for (int v = 0; v < CELLS; ++v) for (int d = 0; d < 4; ++d)
      if (int q = adjacent(v,d); q >= 0) ans[v].push_back(q);
    return ans;
  }();
  VertexRemovalComponents<Counts> end_cuts{CELLS}, pair_cuts{CELLS};
  bool endgame_only = false;
#endif

  int pet_count(const Rect& region) const {
    return ranges::count_if(pets, [&](int p) { return region.contains(p); });
  }

  optional<Cut> choose_cut(int turn) const {
    const int current = pet_count(active);
    if (!current) return nullopt;
    double best = -1e100;
    optional<Cut> selected;
    for (int axis = 0; axis < 2; ++axis) for (int side : {-1, 1}) {
      const int span = active.hi[axis] - active.lo[axis] + 1;
      const int wall = active.hi[1-axis] - active.lo[1-axis] + 1;
      for (int trim = 3; trim <= min(11, (span-1)/2); ++trim) {
        const int line = side > 0 ? active.lo[axis] + trim : active.hi[axis] - trim;
        auto proposal = make_cut(active, axis, line, side, turn);
        const double keep = double(proposal.keep.area()) / active.area();
        const double catch_estimate = 0.55*(current - pet_count(proposal.keep))
                                    + 0.45*current*(1.0 - keep);
        const double value = log2(keep) + catch_estimate - 0.017*wall;
        if (value > best) best = value, selected = std::move(proposal);
      }
    }
    return selected;
  }

#ifdef AHC008_PARALLEL_PENS
  ParallelPens pens;
#endif
  Rect active;
  optional<Cut> cut;
  vector<int> humans, pets;
  bitset<CELLS> walls;
  FlatGridFirstStepBfs bfs{S,S};
};
} // namespace

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  Game game;
  game.read();
  for (int turn = 0; turn < 300; ++turn) {
    auto action = game.plan(turn);
    game.apply(action);
    cout << action << endl;
    if (!game.read_pets()) return 0;
  }
}
