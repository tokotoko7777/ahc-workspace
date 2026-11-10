// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;
#include "../../library/batched-timer.hpp"
#include "../../library/random.hpp"
#include "../../library/axis-aligned-rectangle.hpp"
#include "../../library/largest-empty-rectangle.hpp"
#include "../../library/time-based-simulated-annealing.hpp"
#include "../../library/scope-profiler.hpp"
// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc001_region_sa.cpp
// Official problem: https://atcoder.jp/contests/ahc001/tasks/ahc001_a
// Ideas: https://hakomof.hatenablog.com/entry/2021/03/14/202411
//        https://blog.terry-u16.net/entry/ahc001

#ifndef AHC001_TIME_LIMIT_MS
#define AHC001_TIME_LIMIT_MS 4750
#endif
#ifndef AHC001_ITERATIONS
#define AHC001_ITERATIONS 0
#endif
#ifndef AHC001_THRESHOLD_TABLE_SIZE
#define AHC001_THRESHOLD_TABLE_SIZE 0
#endif
#ifndef AHC001_PRECOMPUTE_THRESHOLD
#define AHC001_PRECOMPUTE_THRESHOLD (AHC001_THRESHOLD_TABLE_SIZE > 0)
#endif
#ifndef AHC001_SCORE_EARLY_STOP
#define AHC001_SCORE_EARLY_STOP 1
#endif
using Rect = AxisAlignedRectangle<int>;

// TODO(AHC001): 固定入力、初期解の再帰分割、領域内の最終サイズ選択。
struct Request {
  int x;
  int y;
  long long desired_area;
};

struct SplitCandidate {
  double error;
  int axis;
  int split_count;
  int cut;
};

double satisfaction(long long desired, long long actual) {
  const double ratio =
      static_cast<double>(min(desired, actual)) / static_cast<double>(max(desired, actual));
  return 1.0 - (1.0 - ratio) * (1.0 - ratio);
}

// A quick approximation used while comparing many recursive partitions.
long long quick_best_area(int width, int height, long long desired) {
  if (width > height) swap(width, height);
  long long best = 1;

  auto try_width = [&](int w) {
    if (w < 1 || w > width) return;
    const long long floor_height = min<long long>(height, desired / w);
    if (floor_height >= 1) {
      const long long area = 1LL * w * floor_height;
      if (abs(area - desired) < abs(best - desired)) best = area;
    }
    const long long ceil_height = min<long long>(height, (desired + w - 1) / w);
    if (ceil_height >= 1) {
      const long long area = 1LL * w * ceil_height;
      if (abs(area - desired) < abs(best - desired)) best = area;
    }
  };

  try_width(1);
  try_width(width);
  try_width(static_cast<int>(sqrt(static_cast<double>(desired))));
  try_width(static_cast<int>(desired / max(1, height)));
  try_width(static_cast<int>((desired + height - 1) / max(1, height)));
  for (int sample = 1; sample <= 24; ++sample) {
    try_width(1 + (width - 1) * sample / 24);
  }
  return best;
}

long long exact_best_area(int width, int height, long long desired,
                          int& best_width, int& best_height) {
  bool swapped = false;
  if (width > height) {
    swap(width, height);
    swapped = true;
  }

  long long best_area = 1;
  int answer_width = 1;
  int answer_height = 1;
  for (int w = 1; w <= width; ++w) {
    const long long floor_height = min<long long>(height, desired / w);
    const long long ceil_height = min<long long>(height, (desired + w - 1) / w);
    for (long long h : {floor_height, ceil_height}) {
      if (h < 1) continue;
      const long long area = 1LL * w * h;
      // 面積差でなく満足度を最大化する。積は入力制約から1e16以下。
      if (min(area, desired) * max(best_area, desired) >
          min(best_area, desired) * max(area, desired)) {
        best_area = area;
        answer_width = w;
        answer_height = static_cast<int>(h);
      }
    }
  }

  if (swapped) swap(answer_width, answer_height);
  best_width = answer_width;
  best_height = answer_height;
  return best_area;
}


vector<Rect> make_initial_regions(const vector<Request>& requests,
                                  double limit_ms, uint64_t input_hash) {
  const int n = static_cast<int>(requests.size());
  Random random(input_hash);
  BatchedTimer timer(limit_ms, 1);
  vector<AxisAlignedRectangle<int>> best_regions(n);
  double best_proxy_score = -1.0;
  int builds = 0;

  while (!timer.is_over()) {
    vector<AxisAlignedRectangle<int>> regions(n);

    function<void(vector<int>&, AxisAlignedRectangle<int>)> divide =
        [&](vector<int>& indices, AxisAlignedRectangle<int> space) {
          if (indices.size() == 1) {
            regions[indices[0]] = space;
            return;
          }

          vector<int> by_x = indices;
          vector<int> by_y = indices;
          sort(by_x.begin(), by_x.end(), [&](int a, int b) {
            return requests[a].x < requests[b].x;
          });
          sort(by_y.begin(), by_y.end(), [&](int a, int b) {
            return requests[a].y < requests[b].y;
          });

          long long total_desired = 0;
          for (int index : indices) total_desired += requests[index].desired_area;

          vector<SplitCandidate> candidates;
          candidates.reserve(indices.size() * 2);
          for (int axis = 0; axis < 2; ++axis) {
            const vector<int>& order = axis == 0 ? by_x : by_y;
            const int low = axis == 0 ? space.left : space.bottom;
            const int high = axis == 0 ? space.right : space.top;
            const int other_length = axis == 0 ? space.height() : space.width();
            long long left_desired = 0;

            for (int count = 1; count < static_cast<int>(order.size()); ++count) {
              left_desired += requests[order[count - 1]].desired_area;
              const int previous_coordinate =
                  axis == 0 ? requests[order[count - 1]].x
                            : requests[order[count - 1]].y;
              const int next_coordinate =
                  axis == 0 ? requests[order[count]].x
                            : requests[order[count]].y;
              if (previous_coordinate == next_coordinate) continue;

              const int minimum_cut = previous_coordinate + 1;
              const int maximum_cut = next_coordinate;
              const long double fraction =
                  static_cast<long double>(left_desired) / total_desired;
              int cut = static_cast<int>(llround(low + (high - low) * fraction));
              cut = clamp(cut, minimum_cut, maximum_cut);

              if (builds > 0 && minimum_cut < maximum_cut &&
                  random.next_int(0, 4) == 0) {
                const int pull = random.next_int(0, 2) == 0
                                     ? minimum_cut
                                     : maximum_cut;
                cut = (3 * cut + pull) / 4;
                cut = clamp(cut, minimum_cut, maximum_cut);
              }

              const long long parent_area = space.area();
              const long long left_area = 1LL * (cut - low) * other_length;
              const long double target_area = parent_area * fraction;
              const double allocation_error =
                  static_cast<double>(abs(static_cast<long double>(left_area) - target_area) /
                  max<long double>(1.0L, parent_area));
              const double depth_error =
                  0.002 * abs(2 * count - static_cast<int>(order.size())) /
                  static_cast<double>(order.size());
              candidates.push_back(
                  {allocation_error + depth_error, axis, count, cut});
            }
          }

          assert(!candidates.empty());
          const int keep = min(10, static_cast<int>(candidates.size()));
          nth_element(candidates.begin(), candidates.begin() + keep - 1,
                      candidates.end(), [](const auto& a, const auto& b) {
                        return a.error < b.error;
                      });
          sort(candidates.begin(), candidates.begin() + keep,
               [](const auto& a, const auto& b) { return a.error < b.error; });

          int chosen_rank = 0;
          if (builds > 0) {
            const int roll = random.next_int(0, 100);
            if (roll >= 58) chosen_rank = min(1, keep - 1);
            if (roll >= 80) chosen_rank = min(2, keep - 1);
            if (roll >= 91) chosen_rank = min(4, keep - 1);
            if (roll >= 97) chosen_rank = random.next_int(0, keep);
          }
          const SplitCandidate chosen = candidates[chosen_rank];
          const vector<int>& order = chosen.axis == 0 ? by_x : by_y;
          vector<int> first(order.begin(), order.begin() + chosen.split_count);
          vector<int> second(order.begin() + chosen.split_count, order.end());

          auto first_space = space;
          auto second_space = space;
          if (chosen.axis == 0) {
            first_space.right = chosen.cut;
            second_space.left = chosen.cut;
          } else {
            first_space.top = chosen.cut;
            second_space.bottom = chosen.cut;
          }
          divide(first, first_space);
          divide(second, second_space);
        };

    vector<int> all(n);
    iota(all.begin(), all.end(), 0);
    divide(all, {0, 0, 10000, 10000});
    ++builds;

    double proxy_score = 0.0;
    for (int i = 0; i < n; ++i) {
      const long long area = quick_best_area(
          max(1, regions[i].width()), max(1, regions[i].height()),
          requests[i].desired_area);
      proxy_score += satisfaction(requests[i].desired_area, area);
    }
    if (proxy_score > best_proxy_score) {
      best_proxy_score = proxy_score;
      best_regions = regions;
    }
  }


  return best_regions;
}

// ============================================================================
// TODO(AHC001): ここから問題固有のState / Moveと近傍。Runner本体は編集しない。
// 「担当領域」は要求面積より大きくてもよい。出力時に内部で最適サイズへ縮める。
// ============================================================================
struct RegionProblem {
  using Score = double; // 大きいほど良い、各領域の満足度の合計。
  struct State {
    vector<Rect> regions; // TODO: 点を含む非重複の担当領域。
    vector<double> quality; // TODO: 各領域の代理評価値をcache。
  };
  struct Move {
    int kind = 0; // 0:境界を押し動かす、1:1領域再配置、2:2領域再構築。
    int index = 0, side = 0, coordinate = 0, other = 0;
    uint64_t salt = 0;
  };
  struct Change { int index; Rect region; double quality; };
  const vector<Request>& requests;
  vector<vector<int>> neighbors;
  vector<Change> pending; // 仮変更。Stateではないので、不採用で現在解は壊れない。
  vector<Rect> obstacles; // 再配置用scratchを再利用する。
  LargestEmptyRectangleWorkspace<Rect> rectangle_workspace; // 探索全体で容量を再利用する。
  // TODO: 【診断時だけ】-DAHC_ENABLE_PROFILINGで処理別時間を測る。
  ScopeProfiler evaluation_profile{"evaluation (including rebuild)"};
  ScopeProfiler rebuild_profile{"rebuild"};

  explicit RegionProblem(const vector<Request>& input) : requests(input) {
    const int n = static_cast<int>(requests.size());
    pending.reserve(input.size());
    obstacles.reserve(input.size());
    neighbors.resize(input.size());
    for (int i = 0; i < n; ++i) {
      vector<pair<int,int>> distances;
      for (int j = 0; j < n; ++j) if (i != j) {
        distances.emplace_back(abs(input[i].x - input[j].x) +
                               abs(input[i].y - input[j].y), j);
      }
      sort(distances.begin(), distances.end());
      for (int k = 0; k < min(10, n - 1); ++k) neighbors[i].push_back(distances[k].second);
    }
  }
  // TODO: 最終出力用の広告ではなく、担当領域の容量を評価する。
  // 容量が要求以上なら1。整数の縦横サイズによる丸めは最後に厳密化する。
  double quality(int id, const Rect& rectangle) const {
    const double ratio = min(1.0, static_cast<double>(rectangle.area()) /
                                  static_cast<double>(requests[id].desired_area));
    return 1.0 - (1.0 - ratio) * (1.0 - ratio);
  }
  State make_state(vector<Rect> regions) const {
    State state{std::move(regions), {}};
    state.quality.resize(requests.size());
    for (int i = 0; i < static_cast<int>(requests.size()); ++i) {
      state.quality[i] = quality(i, state.regions[i]);
    }
    return state;
  }
  double score(const State& state) const {
    return accumulate(state.quality.begin(), state.quality.end(), 0.0);
  }
  // TODO: 近傍の材料だけを返す。現在Stateは変更しない。
  optional<Move> propose_move(const State& state, mt19937_64& random, double progress) const {
    const int n = static_cast<int>(requests.size());
    const auto pick = [&](int count) { return static_cast<int>(random() % static_cast<uint64_t>(count)); };
    int id = pick(n);
    if (pick(2) == 0) { const int j = pick(n); if (state.quality[j] < state.quality[id]) id = j; }
    Move move;
    move.index = id;
    const int kind = pick(100);
    move.kind = kind < 90 ? 0 : kind < 93 ? 1 : 2;
    move.side = pick(4);
    move.salt = random();
    if (move.kind == 2) {
      if (neighbors[id].empty()) return nullopt;
      move.other = neighbors[id][pick(static_cast<int>(neighbors[id].size()))];
    } else if (move.kind == 0) {
      // TODO: 初期は大きく、終盤は小さく境界を動かす。
      const int range = 2 + static_cast<int>(512 * (1 - progress) * (1 - progress));
      int amount = 1 + pick(range);
      if (pick(4) == 0) amount = -amount;
      const auto& r = state.regions[id];
      const auto& p = requests[id];
      // TODO: 1/4の手では要求面積にちょうど届く辺長を狙う。
      // 余分な領域は評価を下げずに縮め、不足領域は不足量だけ広げる。
      if (pick(4) == 0) {
        const int span = move.side < 2 ? r.height() : r.width();
        const int length = move.side < 2 ? r.width() : r.height();
        const int target = static_cast<int>((p.desired_area + span - 1) / span);
        if (target != length) amount = target - length;
      }
      if (move.side == 0) move.coordinate = clamp(r.left - amount, 0, p.x);
      if (move.side == 1) move.coordinate = clamp(r.right + amount, p.x + 1, 10000);
      if (move.side == 2) move.coordinate = clamp(r.bottom - amount, 0, p.y);
      if (move.side == 3) move.coordinate = clamp(r.top + amount, p.y + 1, 10000);
    }
    return move;
  }
  bool legal(int id, const Rect& r) const {
    return r.is_valid() && 0 <= r.left && r.right <= 10000 &&
           0 <= r.bottom && r.top <= 10000 && r.contains(requests[id].x, requests[id].y);
  }
  // TODO: thresholdより良くならないと分かったらnullopt。それ以外は正確な「差分」。
  // 仮変更をpendingへ保存し、採用時だけapply_moveで確定する。
  // 打ち切りOFFではthreshold=-inf。別の全評価関数は不要。不合法手は常にnullopt。
  optional<Score> evaluate_move(const State& state, const Move& move, double threshold) {
    auto evaluation_guard = evaluation_profile.measure();
    pending.clear();
    const int id = move.index, n = static_cast<int>(requests.size());
    if (move.kind == 0) {
      auto next = state.regions[id];
      if (move.side == 0) next.left = move.coordinate;
      if (move.side == 1) next.right = move.coordinate;
      if (move.side == 2) next.bottom = move.coordinate;
      if (move.side == 3) next.top = move.coordinate;
      double delta = quality(id, next) - state.quality[id];
      // 他領域は縮むだけなので、ここから差分は増えない。これは厳密な枝刈り。
      if (delta <= threshold) return nullopt;
      pending.push_back({id, next, quality(id, next)});
      for (int j = 0; j < n; ++j) if (j != id && next.overlaps(state.regions[j])) {
        auto other = state.regions[j];
        // TODO: 相手の点を残せる切り方を4方向から選ぶ。縮小だけなので差分の上界は不変。
        array<Rect, 4> trimmed{{other, other, other, other}};
        trimmed[0].right = min(other.right, next.left);
        trimmed[1].left = max(other.left, next.right);
        trimmed[2].top = min(other.top, next.bottom);
        trimmed[3].bottom = max(other.bottom, next.top);
        long long best_area = -1;
        for (const auto& candidate : trimmed) {
          if (legal(j, candidate) && candidate.area() > best_area) {
            best_area = candidate.area();
            other = candidate;
          }
        }
        if (best_area < 0) return nullopt;
        const double q = quality(j, other);
        delta += q - state.quality[j];
        if (delta <= threshold) return nullopt;
        pending.push_back({j, other, q});
      }
      return delta;
    }
    auto rebuild_guard = rebuild_profile.measure();
    obstacles.clear();
    for (int j = 0; j < n; ++j) {
      if (j != id && (move.kind == 1 || j != move.other)) obstacles.push_back(state.regions[j]);
    }
    if (move.kind == 1) {
      const Rect next = largest_empty_rectangle(Rect{0,0,10000,10000},
                                                requests[id].x, requests[id].y, obstacles, rectangle_workspace);
      const double q = quality(id, next);
      pending.push_back({id, next, q});
      return q - state.quality[id];
    }
    // TODO: 近い2領域を外し、点を分離する境界で空間を分け、各側で再配置する。
    // 3本の境界を試す近似探索。全ての2領域配置を厳密に最適化するわけではない。
    int first = id, second = move.other;
    int axis = move.side % 2;
    if ((axis == 0 ? requests[first].x == requests[second].x :
                     requests[first].y == requests[second].y)) axis ^= 1;
    const auto coordinate = [&](int i) { return axis == 0 ? requests[i].x : requests[i].y; };
    if (coordinate(first) > coordinate(second)) swap(first, second);
    const int low = coordinate(first) + 1, high = coordinate(second);
    const int random_cut = low + static_cast<int>(move.salt % static_cast<uint64_t>(high - low + 1));
    const array<int,3> cuts{{low, high, random_cut}};
    double best_delta = -1e100;
    Rect best_a{}, best_b{};
    for (int cut : cuts) {
      Rect left{0,0,10000,10000}, right = left;
      if (axis == 0) { left.right = cut; right.left = cut; }
      else { left.top = cut; right.bottom = cut; }
      const auto a = largest_empty_rectangle(left, requests[first].x, requests[first].y, obstacles, rectangle_workspace);
      const auto b = largest_empty_rectangle(right, requests[second].x, requests[second].y, obstacles, rectangle_workspace);
      const double delta = quality(first, a) + quality(second, b) -
                           state.quality[first] - state.quality[second];
      if (delta > best_delta) { best_delta = delta; best_a = a; best_b = b; }
    }
    pending.push_back({first, best_a, quality(first, best_a)});
    pending.push_back({second, best_b, quality(second, best_b)});
    return best_delta;
  }
  // TODO: 採用された仮変更だけをStateへ反映する。不採用時は呼ばれない。
  void apply_move(State& state, Move&) {
    for (const auto& c : pending) { state.regions[c.index] = c.region; state.quality[c.index] = c.quality; }
  }
  void validate(const State& state) const {
    for (int i = 0; i < static_cast<int>(requests.size()); ++i) {
      if (!legal(i, state.regions[i]) || abs(state.quality[i] - quality(i,state.regions[i])) > 1e-12)
        throw runtime_error("invalid region/cache");
      for (int j = 0; j < i; ++j) if (state.regions[i].overlaps(state.regions[j]))
        throw runtime_error("overlapping regions");
    }
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  const auto started = chrono::steady_clock::now();
  const auto elapsed = [&]() { return chrono::duration<double,milli>(chrono::steady_clock::now()-started).count(); };
  int n;
  if (!(cin >> n) || n < 1 || n > 200) throw runtime_error("invalid request count");
  vector<Request> requests(static_cast<size_t>(n));
  uint64_t seed = 1;
  for (auto& r : requests) {
    cin >> r.x >> r.y >> r.desired_area;
    seed = seed * 1000003 + static_cast<uint64_t>(r.x + 10000 * r.y) + static_cast<uint64_t>(r.desired_area);
  }
  if (!cin) throw runtime_error("incomplete input");
  RegionProblem problem(requests);
  // TODO: 初期解にも全体時間を使う。固定反復の検査では初期構築も短くする。
  auto state = problem.make_state(make_initial_regions(requests,
      AHC001_ITERATIONS > 0 ? 1.0 : min(180.0, AHC001_TIME_LIMIT_MS * 0.1), seed));
  uint64_t iterations = 0, pruned = 0;
  for (int phase = 0; phase < 2; ++phase) {
    const double remaining = AHC001_TIME_LIMIT_MS - elapsed();
    if (remaining < 10 && AHC001_ITERATIONS == 0) break;
    const double budget = AHC001_ITERATIONS > 0 ? 1e9 : remaining * (phase == 0 ? 0.84 : 0.98);
    // TODO: 代理評価の和に対する温度。後半はほぼ山登りで仕上げる。
    TimeBasedAnnealingRunner<RegionProblem> sa(problem, state, problem.score(state),
        budget, phase == 0 ? 0.015 : 1e-8, phase == 0 ? 1e-6 : 1e-8,
        seed + static_cast<uint64_t>(phase), 4);
    // TODO: 前計算とスコア途中打ち切りは独立。どちらもON/OFFできる。
    sa.annealing().set_threshold_precomputation(AHC001_PRECOMPUTE_THRESHOLD != 0,
        AHC001_THRESHOLD_TABLE_SIZE > 0 ? AHC001_THRESHOLD_TABLE_SIZE : 4096);
    if (AHC001_ITERATIONS > 0) {
      for (int i = 0; i < AHC001_ITERATIONS; ++i)
        sa.step_with_threshold(AHC001_SCORE_EARLY_STOP != 0);
    } else sa.run_with_threshold(AHC001_SCORE_EARLY_STOP != 0);
    state = sa.best_state();
    iterations += sa.iterations();
    pruned += sa.threshold_pruned_moves();
  }
  problem.validate(state);
  // TODO: 担当領域の中で、公式得点が最大になる整数サイズへ縮めて出力する。
  for (int i = 0; i < n; ++i) {
    const auto& region = state.regions[i];
    int w = 1, h = 1;
    exact_best_area(region.width(), region.height(), requests[i].desired_area, w, h);
    const int x = clamp(requests[i].x - w / 2, region.left, region.right - w);
    const int y = clamp(requests[i].y - h / 2, region.bottom, region.top - h);
    cout << x << ' ' << y << ' ' << x+w << ' ' << y+h << '\n';
  }
  cerr << "iterations=" << iterations << " pruned=" << pruned << " elapsed_ms=" << elapsed() << '\n';
  problem.evaluation_profile.report(cerr);
  problem.rebuild_profile.report(cerr);
  return 0;
}
