// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;

// 提出時は、この2行を各hppの全文へ置き換える。
#include "../../library/action-beam-search.hpp"
#include "../../library/simulated-annealing.hpp"

// Pre-contest public solver source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc071_action_beam.cpp
// Official problem: https://atcoder.jp/contests/ahc071/tasks/ahc071_a

// AHC071「Wall Making」用の実例。
// 上の段に置いた各レンガは、中心1マスだけが下の段に支えられていればよい。
// したがって上から下へ作る時、次の段へ渡す情報は「直上段の中心bitset」だけ。
//
// 問題ごとに考える部分は TODO(AHC071) と書いた。ビームの上位N件選択、
// 同一状態の重複除去、採用候補だけのStateコピーはActionBeamRunnerが担当する。
using Mask = uint64_t;
constexpr int MAX_W = 60;
constexpr int MAX_ROW_CANDIDATES = 32;
constexpr float INF = 1e30f;

#ifndef AHC071_TIME_LIMIT
#define AHC071_TIME_LIMIT 1.80
#endif
#ifndef AHC071_BEAM_WIDTH
#define AHC071_BEAM_WIDTH 64
#endif
#ifndef AHC071_ROW_CANDIDATES
#define AHC071_ROW_CANDIDATES 8
#endif

struct Brick {
  int x;
  int y;
  int width;
};

struct Row {
  // starts[t]のx bitが1なら、(x, y)から幅2*t+1のレンガを置く。
  array<Mask, 5> starts{};
  Mask centers = 0;
  int cost = 0;

  Mask covered() const {
    Mask result = 0;
    for (int t = 0; t < 5; ++t) {
      for (int dx = 0; dx <= 2 * t; ++dx) result |= starts[t] << dx;
    }
    return result;
  }
};

struct Solver {
  // TODO(AHC071): 入力と、全Stateで共通の事前計算をここへ置く。
  int W = 0;
  int H = 0;
  int K = 0;
  array<int, 5> costs{};
  vector<Mask> holes;
  vector<array<float, MAX_W>> potential;
  vector<unordered_map<Mask, vector<array<float, MAX_W>>>> local_cache;
  array<float, MAX_W> zero_weight{};
  Mask full = 0;

  mt19937 random_engine{712367821};
  chrono::steady_clock::time_point started;
  double time_limit = AHC071_TIME_LIMIT;
  int beam_width = AHC071_BEAM_WIDTH;
  int row_candidate_count = AHC071_ROW_CANDIDATES;
  double initial_ratio = 0.25;
  int min_rebuild_height = 3;
  int max_rebuild_height = 12;
  bool use_lns = true;

  double elapsed() const {
    return chrono::duration<double>(
               chrono::steady_clock::now() - started)
        .count();
  }

  void read_input() {
    cin >> W >> H >> K;
    for (int& cost : costs) cin >> cost;
    full = (Mask(1) << W) - 1;
    holes.assign(H, 0);
    for (int i = 0; i < K; ++i) {
      int x, y;
      cin >> x >> y;
      holes[y] |= Mask(1) << x;
    }
  }

  // TODO(AHC071): 1行の最適化。
  // requiredを全て覆い、各レンガの中心がallowedに含まれる配置の最小値を返す。
  // weight[center]は「その中心を次の段でも支える将来費用」の近似値。
  float row_dp(Mask required,
               const array<float, MAX_W>& weight,
               Mask allowed,
               Row* answer = nullptr) const {
    float dp[MAX_W + 1];
    int next_required[MAX_W + 1];
    signed char take_type[MAX_W];
    dp[W] = 0;
    next_required[W] = W;
    for (int p = W - 1; p >= 0; --p) {
      const int first = next_required[p] =
          ((required >> p) & 1) ? p : next_required[p + 1];
      dp[p] = first == p ? INF : dp[p + 1];
      take_type[p] = -1;
      if (first == W) {
        dp[p] = 0;
        continue;
      }
      for (int type = 0; type < 5; ++type) {
        const int length = 2 * type + 1;
        if (p + length > W || p + length <= first) continue;
        if (!((allowed >> (p + type)) & 1)) continue;
        const float value =
            static_cast<float>(costs[type]) +
            weight[p + type] + dp[p + length];
        if (value < dp[p]) {
          dp[p] = value;
          take_type[p] = static_cast<signed char>(type);
        }
      }
    }

    if (answer != nullptr && dp[0] < INF / 2) {
      *answer = Row{};
      for (int p = 0; next_required[p] < W;) {
        const int type = take_type[p];
        if (type < 0) {
          ++p;
          continue;
        }
        answer->starts[type] |= Mask(1) << p;
        answer->centers |= Mask(1) << (p + type);
        answer->cost += costs[type];
        p += 2 * type + 1;
      }
    }
    return dp[0];
  }

  // TODO(AHC071): ある中心を追加した時、下の行以降で増える費用を前計算する。
  void build_potential() {
    potential.resize(H);
    for (int y = 0; y < H; ++y) {
      array<float, MAX_W> weight{};
      if (y > 0) {
        for (int x = 0; x < W; ++x) {
          weight[x] = 0.90f * potential[y - 1][x];
        }
      }
      const float base = row_dp(holes[y], weight, full);
      for (int x = 0; x < W; ++x) {
        const float added =
            row_dp(holes[y] | (Mask(1) << x), weight, full);
        potential[y][x] = max(0.0f, added - base);
      }
    }
  }

  struct RowDpEntry {
    float value = 0;
    Mask centers = 0;
    int cost = 0;
    unsigned char start = 0;
    unsigned char type = 0;
    unsigned char tail_rank = 0;
  };

  // TODO(AHC071): 1行について最良だけでなく上位limit通りを列挙する。
  // ビーム幅を広げても各親から同じ1通りしか出さないと多様性が増えない。
  vector<Row> row_candidates(
      Mask required,
      const array<float, MAX_W>& weight,
      int limit) const {
    assert(1 <= limit && limit <= MAX_ROW_CANDIDATES);
    if (required == 0) return {Row{}};

    RowDpEntry dp[MAX_W + 1][MAX_ROW_CANDIDATES];
    int count[MAX_W + 1]{};
    int next_required[MAX_W + 1];
    count[W] = 1;
    next_required[W] = W;
    const auto worse = [](const RowDpEntry& left, const RowDpEntry& right) {
      if (left.value != right.value) return left.value > right.value;
      if (left.cost != right.cost) return left.cost > right.cost;
      return left.centers > right.centers;
    };

    for (int p = W - 1; p >= 0; --p) {
      const int first = next_required[p] =
          ((required >> p) & 1) ? p : next_required[p + 1];
      if (first == W) {
        count[p] = 1;
        continue;
      }

      array<RowDpEntry, 6> heap;
      int heap_size = 0;
      const auto make_entry = [&](int type, int rank) {
        const bool skip = type == 5;
        const int end = p + (skip ? 1 : 2 * type + 1);
        const RowDpEntry& tail = dp[end][rank];
        return RowDpEntry{
            (skip ? 0.0f
                  : static_cast<float>(costs[type]) + weight[p + type]) +
                tail.value,
            tail.centers |
                (skip ? Mask(0) : Mask(1) << (p + type)),
            (skip ? 0 : costs[type]) + tail.cost,
            static_cast<unsigned char>(p),
            static_cast<unsigned char>(type),
            static_cast<unsigned char>(rank)};
      };

      if (first != p && count[p + 1]) {
        heap[heap_size++] = make_entry(5, 0);
      }
      for (int type = 0; type < 5; ++type) {
        const int end = p + 2 * type + 1;
        if (first < end && end <= W && count[end] &&
            weight[p + type] < INF / 4) {
          heap[heap_size++] = make_entry(type, 0);
        }
      }
      make_heap(heap.begin(), heap.begin() + heap_size, worse);
      while (heap_size && count[p] < limit) {
        pop_heap(heap.begin(), heap.begin() + heap_size, worse);
        const RowDpEntry entry = heap[--heap_size];
        bool duplicate = false;
        for (int i = 0; i < count[p]; ++i) {
          duplicate |= dp[p][i].centers == entry.centers;
        }
        if (!duplicate) dp[p][count[p]++] = entry;

        const int end =
            p + (entry.type == 5 ? 1 : 2 * entry.type + 1);
        if (entry.tail_rank + 1 < count[end]) {
          heap[heap_size++] = make_entry(entry.type, entry.tail_rank + 1);
          push_heap(heap.begin(), heap.begin() + heap_size, worse);
        }
      }
    }

    vector<Row> result;
    result.reserve(count[0]);
    for (int rank = 0; rank < count[0]; ++rank) {
      Row row;
      row.cost = dp[0][rank].cost;
      row.centers = dp[0][rank].centers;
      int p = 0;
      int current_rank = rank;
      while (next_required[p] < W) {
        const RowDpEntry& entry = dp[p][current_rank];
        if (entry.type == 5) {
          ++p;
        } else {
          row.starts[entry.type] |= Mask(1) << entry.start;
          p = entry.start + 2 * entry.type + 1;
        }
        current_rank = entry.tail_rank;
      }
      result.push_back(row);
    }
    return result;
  }

  vector<Row> greedy_solution() const {
    vector<Row> rows(H);
    Mask support_from_above = 0;
    for (int y = H - 1; y >= 0; --y) {
      const auto& weight = y ? potential[y - 1] : zero_weight;
      row_dp(holes[y] | support_from_above, weight, full, &rows[y]);
      support_from_above = rows[y].centers;
    }
    return rows;
  }

  // TODO(AHC071): 他の行を固定し、1行だけ厳密に安くする局所改善。
  void polish(vector<Row>& rows) const {
    for (int pass = 0; pass < 6; ++pass) {
      bool changed = false;
      for (int i = 0; i < H; ++i) {
        const int y = pass % 2 ? i : H - 1 - i;
        const Mask required =
            holes[y] | (y + 1 < H ? rows[y + 1].centers : 0);
        const Mask allowed = y ? rows[y - 1].covered() : full;
        Row replacement;
        const float value =
            row_dp(required, zero_weight, allowed, &replacement);
        if (value < INF / 2 && replacement.cost < rows[y].cost) {
          rows[y] = replacement;
          changed = true;
        }
      }
      if (!changed) break;
    }
  }

  static int total_cost(const vector<Row>& rows) {
    int result = 0;
    for (const Row& row : rows) result += row.cost;
    return result;
  }

  // Scoreは小さいほど良い。estimatedだけが同点なら実費、中心bitsetで比較する。
  struct BeamRank {
    float estimated = 0;
    int cost = 0;
    Mask centers = 0;

    friend bool operator<(const BeamRank& left, const BeamRank& right) {
      if (left.estimated != right.estimated) {
        return left.estimated < right.estimated;
      }
      if (left.cost != right.cost) return left.cost < right.cost;
      return left.centers < right.centers;
    }
  };

  // ここが「人が問題に合わせて書く部分」。ライブラリ本体は編集しない。
  struct RowBeamProblem {
    struct State {
      // TODO(AHC071): 次の行を作るための最小状態。
      Mask support_from_above = 0;
      int cost = 0;
      vector<Row> rows_top_down;
    };

    struct Action {
      // TODO(AHC071): 1手=今作る1行。next_costは差分評価結果のcache。
      Row row;
      int next_cost = 0;
    };

    using Score = BeamRank;

    Solver& solver;
    int lo;
    int hi;
    int row_limit;
    int completion_cost_bound;
    float scale;
    Mask lower_support;
    const vector<array<float, MAX_W>>& guide;
    const vector<Row>* incumbent_rows;
    double deadline;
    vector<array<float, MAX_W>> row_weight;
    vector<array<float, MAX_W>> below_weight;

    RowBeamProblem(Solver& solver_value,
                   int lo_value,
                   int hi_value,
                   int row_limit_value,
                   int completion_cost_bound_value,
                   float scale_value,
                   float noise,
                   Mask lower_support_value,
                   const vector<array<float, MAX_W>>& guide_value,
                   const vector<Row>* incumbent_rows_value,
                   double deadline_value)
        : solver(solver_value),
          lo(lo_value),
          hi(hi_value),
          row_limit(row_limit_value),
          completion_cost_bound(completion_cost_bound_value),
          scale(scale_value),
          lower_support(lower_support_value),
          guide(guide_value),
          incumbent_rows(incumbent_rows_value),
          deadline(deadline_value),
          row_weight(solver.H),
          below_weight(solver.H) {
      // 同じ世代の全親へ同じ摂動を使う。同一局面の評価も同じになる。
      for (int y = lo; y <= hi; ++y) {
        for (int x = 0; x < solver.W; ++x) {
          if (y > lo) {
            row_weight[y][x] = scale * guide[y - 1][x];
            if (noise != 0) {
              row_weight[y][x] +=
                  noise * (float(solver.random_engine() % 10001) / 10000 -
                           0.5f);
            }
          }
          if (y > lo + 1) {
            below_weight[y][x] = 0.90f * guide[y - 2][x];
          }
        }
      }
    }

    vector<Action> generate_actions(const State& state) const {
      // TODO(AHC071): 現在のStateから合法な「次の1行」を列挙する。
      if (solver.elapsed() >= deadline) return {};
      const int y = hi - static_cast<int>(state.rows_top_down.size());
      const Mask required = solver.holes[y] | state.support_from_above;
      vector<Row> options;
      if (y == lo) {
        Row row;
        if (solver.row_dp(required,
                          solver.zero_weight,
                          lower_support,
                          &row) < INF / 4) {
          options.push_back(row);
        }
      } else {
        options = solver.row_candidates(required, row_weight[y], row_limit);
        // 区間再構築では元の行も候補へ入れ、合法なら戻れる道を増やす。
        if (incumbent_rows != nullptr) {
          const Row& old = (*incumbent_rows)[y];
          if ((old.covered() & required) == required) options.push_back(old);
        }
      }

      vector<Action> actions;
      actions.reserve(options.size());
      for (Row& row : options) {
        const int next_cost = state.cost + row.cost;
        // TODO(AHC071): 完成済み解を閾値にしたbranch-and-bound。
        // 未構築行の費用は非負なので、この時点で上限を超えた枝は改善不能。
        if (next_cost > completion_cost_bound) continue;
        actions.push_back(Action{std::move(row), next_cost});
      }
      return actions;
    }

    Score evaluate_action(const State& state, const Action& action) const {
      // TODO(AHC071): Action適用後の「順位値そのもの」を返す。
      const int y = hi - static_cast<int>(state.rows_top_down.size());
      float future = 0;
      if (y > lo) {
        const Mask allowed = y == lo + 1 ? lower_support : solver.full;
        future = solver.row_dp(solver.holes[y - 1] | action.row.centers,
                               below_weight[y],
                               allowed);
      }
      return Score{static_cast<float>(action.next_cost) + scale * future,
                   action.next_cost,
                   action.row.centers};
    }

    optional<Score> evaluate_action_with_threshold(
        const State& state,
        const Action& action,
        const Score* threshold) const {
      // TODO(AHC071): key重複除去が不要な問題ではrun_with_thresholdを使える。
      // 将来費用は非負なので、現在費用だけで境界を超えたらrow_dpを省略可能。
      if (threshold != nullptr &&
          static_cast<float>(action.next_cost) > threshold->estimated) {
        return nullopt;
      }
      return evaluate_action(state, action);
    }

    Mask make_key(const State& state, const Action& action) const {
      // TODO(AHC071): 次の段が見る情報が同じ候補は、最安の1件だけ残す。
      const int y = hi - static_cast<int>(state.rows_top_down.size());
      return y > lo ? action.row.centers | solver.holes[y - 1] : 0;
    }

    void apply_action(State& state, Action& action) const {
      // TODO(AHC071): 選ばれた上位N件だけ、Stateを本当に更新する。
      state.support_from_above = action.row.centers;
      state.cost = action.next_cost;
      state.rows_top_down.push_back(std::move(action.row));
    }
  };

  // [lo, hi]を上から下へActionBeamRunnerで構築する。
  optional<vector<Row>> search_rows(
      int lo,
      int hi,
      int width,
      int row_limit,
      int cost_bound,
      float scale,
      float noise,
      Mask upper_support,
      Mask lower_support,
      const vector<array<float, MAX_W>>& guide,
      const vector<Row>* incumbent_rows,
      double deadline) {
    RowBeamProblem problem(*this,
                           lo,
                           hi,
                           row_limit,
                           cost_bound,
                           scale,
                           noise,
                           lower_support,
                           guide,
                           incumbent_rows,
                           deadline);
    typename RowBeamProblem::State initial;
    initial.support_from_above = upper_support;
    ActionBeamRunner<RowBeamProblem> beam(
        problem, initial, BeamRank{}, width, false);  // false = 小さいほど良い
    beam.reserve_candidates(static_cast<size_t>(width) * row_limit);

    // AHC071では同じ「次段の必須bitset」をまとめる効果が大きいためkey版。
    // 閾値評価版を使う問題はrun_with_thresholdへ変更する。
    const int turns = hi - lo + 1;
    if (beam.run_with_key(turns) != turns) return nullopt;

    const auto& best = beam.best();
    vector<Row> result(turns);
    for (int i = 0; i < turns; ++i) {
      result[hi - lo - i] = best.rows_top_down[i];
    }
    return result;
  }

  // 区間下端の支持条件を含む将来費用をcacheする。
  const vector<array<float, MAX_W>>* prepare_local_potential(
      int lo,
      int hi,
      Mask lower,
      double deadline) {
    if (static_cast<int>(local_cache.size()) != H) local_cache.resize(H);
    auto& cache = local_cache[lo];
    if (cache.size() >= 64 && !cache.count(lower)) cache.clear();
    auto [iterator, inserted] = cache.try_emplace(lower);
    auto& local = iterator->second;
    if (inserted) local.resize(lo);
    while (static_cast<int>(local.size()) < hi) {
      if (elapsed() >= deadline) return nullptr;
      const int y = static_cast<int>(local.size());
      array<float, MAX_W> weight{};
      if (y > lo) {
        for (int x = 0; x < W; ++x) weight[x] = 0.90f * local[y - 1][x];
      }
      const Mask allowed = y == lo ? lower : full;
      const float base = row_dp(holes[y], weight, allowed);
      array<float, MAX_W> level{};
      for (int x = 0; x < W; ++x) {
        const float value =
            row_dp(holes[y] | (Mask(1) << x), weight, allowed);
        level[x] = value >= INF / 4 ? INF : max(0.0f, value - base);
      }
      local.push_back(level);
    }
    return &local;
  }

  bool rebuild(vector<Row>& rows,
               int lo,
               int hi,
               int width,
               float scale,
               float noise,
               double deadline) {
    const Mask lower = lo ? rows[lo - 1].covered() : full;
    const auto* guide = prepare_local_potential(lo, hi, lower, deadline);
    if (guide == nullptr) return false;

    int old_cost = 0;
    for (int y = lo; y <= hi; ++y) old_cost += rows[y].cost;
    auto replacement = search_rows(lo,
                                   hi,
                                   width,
                                   row_candidate_count,
                                   old_cost,
                                   scale,
                                   noise,
                                   hi + 1 < H ? rows[hi + 1].centers : 0,
                                   lower,
                                   *guide,
                                   &rows,
                                   deadline);
    if (!replacement.has_value()) return false;
    const int new_cost = total_cost(*replacement);
    if (new_cost > old_cost) return false;
    for (int y = lo; y <= hi; ++y) rows[y] = (*replacement)[y - lo];
    return true;
  }

  vector<Row> solve() {
    started = chrono::steady_clock::now();
    build_potential();
    vector<Row> best = greedy_solution();
    polish(best);
    int best_cost = total_cost(best);

    const double deadline = max(0.0, time_limit - 0.025);
    const double initial_deadline =
        use_lns ? deadline * initial_ratio : deadline;
    const float scales[] = {1.0f, 1.5f, 0.7f, 2.0f, 1.2f, 0.85f};
    int completed_beams = 0;
    for (int run = 0; elapsed() < initial_deadline; ++run) {
      auto candidate = search_rows(0,
                                   H - 1,
                                   beam_width,
                                   row_candidate_count,
                                   best_cost,
                                   scales[run % 6],
                                   run < 3 ? 0.0f : 1.0f,
                                   0,
                                   full,
                                   potential,
                                   nullptr,
                                   initial_deadline);
      if (!candidate.has_value()) break;
      ++completed_beams;
      polish(*candidate);
      const int cost = total_cost(*candidate);
      if (cost < best_cost) {
        best = std::move(*candidate);
        best_cost = cost;
      }
    }

    const int initial_cost = best_cost;
    int rebuild_count = 0;
    if (use_lns) {
      vector<Row> current = best;
      int current_cost = best_cost;
      SimulatedAnnealing annealing(2.0, 0.1, 314159265);
      while (elapsed() < deadline) {
        vector<Row> trial = current;
        const int max_height = min(H, max_rebuild_height);
        const int min_height = min(max_height, min_rebuild_height);
        const int height = min_height + static_cast<int>(
            random_engine() %
            static_cast<unsigned int>(max_height - min_height + 1));
        const int lo = static_cast<int>(
            random_engine() % static_cast<unsigned int>(H - height + 1));
        const float scale =
            0.7f + static_cast<float>(random_engine() % 1001) / 1000;

        if (random_engine() % 4 == 0) {
          // 1行だけ別配置にして、区間ビームとは違う谷へ移る。
          const int y = static_cast<int>(
              random_engine() % static_cast<unsigned int>(H));
          array<float, MAX_W> random_weight{};
          for (int x = 0; x < W; ++x) {
            random_weight[x] =
                8.0f * (float(random_engine() % 10001) / 10000 - 0.5f);
          }
          const Mask required =
              holes[y] | (y + 1 < H ? trial[y + 1].centers : 0);
          const Mask allowed = y ? trial[y - 1].covered() : full;
          row_dp(required, random_weight, allowed, &trial[y]);
        } else {
          if (!rebuild(trial,
                       lo,
                       lo + height - 1,
                       24,
                       scale,
                       2.0f,
                       deadline)) {
            continue;
          }
          ++rebuild_count;
          polish(trial);
        }

        const int trial_cost = total_cost(trial);
        const double progress = clamp(
            (elapsed() - initial_deadline) /
                max(0.001, deadline - initial_deadline),
            0.0,
            1.0);
        annealing.set_progress(progress);
        const int improvement = current_cost - trial_cost;  // 費用最小化
        if (annealing.accept(improvement)) {
          current = std::move(trial);
          current_cost = trial_cost;
          if (current_cost < best_cost) {
            best = current;
            best_cost = current_cost;
          }
        }
      }
      polish(best);
      best_cost = total_cost(best);
    }

    cerr << "cost: " << best_cost
         << " beams: " << completed_beams
         << " initial: " << initial_cost
         << " rebuilds: " << rebuild_count
         << " elapsed: " << elapsed() * 1000 << "ms\n";
    return best;
  }

  void print_answer(const vector<Row>& rows) const {
    vector<Brick> answer;
    for (int y = 0; y < H; ++y) {
      for (int type = 0; type < 5; ++type) {
        for (Mask starts = rows[y].starts[type]; starts;
             starts &= starts - 1) {
          answer.push_back(
              Brick{__builtin_ctzll(starts), y, 2 * type + 1});
        }
      }
    }
    cout << answer.size() << '\n';
    for (const Brick& brick : answer) {
      cout << brick.x << ' ' << brick.y << ' ' << brick.width << '\n';
    }
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  Solver solver;
  solver.read_input();
  if (solver.W <= 0 || solver.W > MAX_W || solver.H <= 0 ||
      solver.row_candidate_count < 1 ||
      solver.row_candidate_count > MAX_ROW_CANDIDATES) {
    return 1;
  }
  const vector<Row> answer = solver.solve();
  solver.print_answer(answer);
}
