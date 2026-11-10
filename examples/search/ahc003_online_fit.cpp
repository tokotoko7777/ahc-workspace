// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
using namespace std;
#include "../../library/time-based-simulated-annealing.hpp"
// Public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc003_online_fit.cpp
// Official problem: https://atcoder.jp/contests/ahc003/tasks/ahc003_a

constexpr int GRID_SIZE = 30;
constexpr int VERTEX_COUNT = GRID_SIZE * GRID_SIZE;
constexpr int HORIZONTAL_EDGE_COUNT = GRID_SIZE * (GRID_SIZE - 1);
constexpr int EDGE_COUNT = 2 * HORIZONTAL_EDGE_COUNT;

// 29本の辺を前半・後半の2区間に分ける。
// 真の入力も各行・各列で高々2区間なので、1辺ずつ完全に独立に学ぶより安定する。
constexpr int BUCKET_COUNT = 2;
constexpr double EXPLORATION_RATE = 1.65;
constexpr double EDGE_LEARNING_WEIGHT = 2.0;
constexpr double START_LEARNING_RATE = 0.70;
constexpr double END_LEARNING_RATE = 0.30;
constexpr int FEATURE_COUNT = 2 * GRID_SIZE * BUCKET_COUNT;
constexpr int LINE_COUNT = 2 * GRID_SIZE;

int horizontal_edge_id(int row, int left_column) {
  return row * (GRID_SIZE - 1) + left_column;
}

int vertical_edge_id(int upper_row, int column) {
  return HORIZONTAL_EDGE_COUNT + upper_row * GRID_SIZE + column;
}

int edge_feature(int edge) {
  if (edge < HORIZONTAL_EDGE_COUNT) {
    const int row = edge / (GRID_SIZE - 1);
    const int column = edge % (GRID_SIZE - 1);
    const int bucket = min(
        BUCKET_COUNT - 1,
        column * BUCKET_COUNT / (GRID_SIZE - 1));
    return row * BUCKET_COUNT + bucket;
  }

  const int local_edge = edge - HORIZONTAL_EDGE_COUNT;
  const int row = local_edge / GRID_SIZE;
  const int column = local_edge % GRID_SIZE;
  const int bucket = min(
      BUCKET_COUNT - 1,
      row * BUCKET_COUNT / (GRID_SIZE - 1));
  return GRID_SIZE * BUCKET_COUNT + column * BUCKET_COUNT + bucket;
}

int edge_line(int edge) {
  if (edge < HORIZONTAL_EDGE_COUNT) {
    return edge / (GRID_SIZE - 1);
  }
  return GRID_SIZE + (edge - HORIZONTAL_EDGE_COUNT) % GRID_SIZE;
}

// TODO(AHC003): 観測された経路総和から未知辺コストを推定する。
// 焼きなまし/山登りの基本フォーマットを「解の推定モデルの更新」に使う。
// ここでは凸2次目的なので、温度を使わない山登りで十分。
struct RegressionProblem {
  static constexpr int PARAMETER_COUNT = LINE_COUNT + FEATURE_COUNT + EDGE_COUNT;
  struct State {
    vector<double> weight = vector<double>(PARAMETER_COUNT, 0.0);
    vector<double> prediction;
  };
  struct Move {
    int feature;
    double shift, improvement;
  };
  using Score = double;
  struct Observation {
    vector<pair<int, int>> features;
    double base, observed, precision;
  };
  vector<Observation> history;
  array<vector<pair<int, int>>, PARAMETER_COUNT> appearances;
  array<vector<int>, 3> recent;
  array<double, PARAMETER_COUNT> prior{}, diagonal{};
  array<double, LINE_COUNT> line_information{};
  array<double, FEATURE_COUNT> feature_information{};
  array<int, EDGE_COUNT> edge_use_count{};

  RegressionProblem() {
    for (int i = 0; i < PARAMETER_COUNT; ++i) {
      // TODO: パラメータの事前分散。行/列の共通成分、区間補正、辺固有補正。
      prior[i] = 1.0 / (i < LINE_COUNT ? 4000000.0 :
                       i < LINE_COUNT + FEATURE_COUNT ? 1000000.0 : 250000.0);
      diagonal[i] = prior[i];
    }
  }

  // TODO: 採用済み経路と返却値だけを追加する。未知の真の辺長にはアクセスしない。
  double observe(State& state, const vector<int>& edges, int observed) {
    array<int, PARAMETER_COUNT> count{};
    for (int edge : edges) {
      ++count[edge_line(edge)];
      ++count[LINE_COUNT + edge_feature(edge)];
      ++count[LINE_COUNT + FEATURE_COUNT + edge];
      ++edge_use_count[edge];
    }
    for (auto& group : recent) group.clear();
    Observation observation;
    observation.base = 5000.0 * edges.size();
    observation.observed = observed;
    // 一様な±10%ノイズの分散は概ね真値^2 / 300。
    observation.precision = 300.0 / (1.0 * observed * observed + 1.0);
    double prediction = observation.base;
    for (int f = 0; f < PARAMETER_COUNT; ++f) if (count[f]) {
      observation.features.push_back({f, count[f]});
      appearances[f].push_back({static_cast<int>(history.size()), count[f]});
      diagonal[f] += count[f] * count[f] * observation.precision;
      prediction += count[f] * state.weight[f];
      const int group = f < LINE_COUNT ? 0 : f < LINE_COUNT + FEATURE_COUNT ? 1 : 2;
      recent[group].push_back(f);
      if (group == 0) line_information[f] += count[f] * count[f];
      if (group == 1) feature_information[f - LINE_COUNT] += count[f] * count[f];
    }
    state.prediction.push_back(prediction);
    const double error = prediction - observed;
    const double loss = error * error * observation.precision;
    history.push_back(std::move(observation));
    return -loss;
  }

  optional<Move> propose_move(const State& state, mt19937_64& engine, double) const {
    int feature;
    if (engine() % 4 != 0) {
      const auto& group = recent[engine() % 3];
      if (group.empty()) return nullopt;
      feature = group[engine() % group.size()];
    } else {
      feature = static_cast<int>(engine() % PARAMETER_COUNT);
      if (appearances[feature].empty()) return nullopt;
    }
    // TODO: 1変数だけの厳密な最小値へ移す近傍。触れた観測だけから計算する。
    double gradient = prior[feature] * state.weight[feature];
    for (auto [index, count] : appearances[feature]) {
      gradient += count * history[index].precision *
                  (state.prediction[index] - history[index].observed);
    }
    const double shift = -gradient / diagonal[feature];
    return Move{feature, shift, gradient * gradient / diagonal[feature]};
  }

  optional<Score> evaluate_move(const State&, const Move& move, double threshold) const {
    // TODO: 最大化する目的は「負の正則化付き二乗誤差」。返すのは改善量。
    if (move.improvement <= threshold) return nullopt;
    return move.improvement;
  }

  void apply_move(State& state, Move& move) const {
    state.weight[move.feature] += move.shift;
    for (auto [index, count] : appearances[move.feature]) {
      state.prediction[index] += count * move.shift;
    }
  }

  double full_score(const State& state) const {
    double score = 0;
    for (int f = 0; f < PARAMETER_COUNT; ++f) score -= prior[f] * state.weight[f] * state.weight[f];
    for (size_t i = 0; i < history.size(); ++i) {
      const double error = state.prediction[i] - history[i].observed;
      score -= history[i].precision * error * error;
    }
    return score;
  }

  double estimated_cost(const State& state, int edge) const {
    return clamp(5000.0 + state.weight[edge_line(edge)] +
                 state.weight[LINE_COUNT + edge_feature(edge)] +
                 state.weight[LINE_COUNT + FEATURE_COUNT + edge], 1000.0, 9000.0);
  }

  array<int, EDGE_COUNT> planning_costs(const State& state, int turn) const {
    array<int, EDGE_COUNT> result{};
    for (int edge = 0; edge < EDGE_COUNT; ++edge) {
      const double uncertainty =
          650.0 / sqrt(1.0 + line_information[edge_line(edge)] / 16.0) +
          450.0 / sqrt(1.0 + feature_information[edge_feature(edge)] / 8.0) +
          250.0 / sqrt(1.0 + edge_use_count[edge]);
      const double bonus = (1.0 - turn / 1000.0) * EXPLORATION_RATE * uncertainty;
      result[edge] = static_cast<int>(lround(max(1000.0, estimated_cost(state, edge) - bonus)));
    }
    return result;
  }
};

struct OnlineEdgeEstimator {
  RegressionProblem problem;
  RegressionProblem::State state;
  double score = 0;

  array<int, EDGE_COUNT> planning_costs(int turn) const {
    return problem.planning_costs(state, turn);
  }
  void update(const vector<int>& edges, int observed, int turn) {
    score += problem.observe(state, edges, observed);
    // TODO: 局所探索の基本フォーマットと同じRunner。温度は山登りなので使わない。
    // 固定256試行で再現性を優先。1クエリの安全上限も設定する。
    TimeBasedAnnealingRunner<RegressionProblem> search(
        problem, std::move(state), score, 10.0, 1.0, 1.0, 1234567 + turn, 16);
    for (int iteration = 0; iteration < 256; ++iteration) {
      if (!search.step_hill_climbing()) break;
    }
    state = search.best_state();
    score = search.best_score();
  }
};

struct Path {
  string moves;
  vector<int> edges;
};

Path shortest_path(
    int start_row,
    int start_column,
    int target_row,
    int target_column,
    const array<int, EDGE_COUNT>& edge_cost) {
  const int start = start_row * GRID_SIZE + start_column;
  const int target = target_row * GRID_SIZE + target_column;
  constexpr int INF = numeric_limits<int>::max() / 4;

  array<int, VERTEX_COUNT> distance;
  array<int, VERTEX_COUNT> parent;
  array<int, VERTEX_COUNT> parent_edge;
  array<char, VERTEX_COUNT> parent_move;
  distance.fill(INF);
  parent.fill(-1);

  using QueueEntry = pair<int, int>;
  priority_queue<QueueEntry, vector<QueueEntry>, greater<QueueEntry>> queue;
  distance[start] = 0;
  parent[start] = start;
  queue.push({0, start});

  auto relax = [&](int from, int to, int edge, char move) {
    const int next_distance = distance[from] + edge_cost[edge];
    if (next_distance >= distance[to]) return;
    distance[to] = next_distance;
    parent[to] = from;
    parent_edge[to] = edge;
    parent_move[to] = move;
    queue.push({next_distance, to});
  };

  while (!queue.empty()) {
    const auto [current_distance, vertex] = queue.top();
    queue.pop();
    if (current_distance != distance[vertex]) continue;
    if (vertex == target) break;

    const int row = vertex / GRID_SIZE;
    const int column = vertex % GRID_SIZE;
    if (row > 0) {
      relax(vertex, vertex - GRID_SIZE,
            vertical_edge_id(row - 1, column), 'U');
    }
    if (row + 1 < GRID_SIZE) {
      relax(vertex, vertex + GRID_SIZE,
            vertical_edge_id(row, column), 'D');
    }
    if (column > 0) {
      relax(vertex, vertex - 1,
            horizontal_edge_id(row, column - 1), 'L');
    }
    if (column + 1 < GRID_SIZE) {
      relax(vertex, vertex + 1,
            horizontal_edge_id(row, column), 'R');
    }
  }

  string reversed_moves;
  vector<int> reversed_edges;
  for (int vertex = target; vertex != start; vertex = parent[vertex]) {
    assert(parent[vertex] != -1);
    reversed_moves += parent_move[vertex];
    reversed_edges.push_back(parent_edge[vertex]);
  }
  reverse(reversed_moves.begin(), reversed_moves.end());
  reverse(reversed_edges.begin(), reversed_edges.end());
  return {move(reversed_moves), move(reversed_edges)};
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  OnlineEdgeEstimator estimator;

  for (int turn = 0; turn < 1000; ++turn) {
    int start_row, start_column, target_row, target_column;
    if (!(cin >> start_row >> start_column >> target_row >> target_column)) return 0;

    const auto edge_cost = estimator.planning_costs(turn);
    const Path path = shortest_path(
        start_row,
        start_column,
        target_row,
        target_column,
        edge_cost);

    cout << path.moves << endl;  // endlで対話出力をflushする

    int observed_length;
    if (!(cin >> observed_length)) return 0;
    estimator.update(path.edges, observed_length, turn);
  }
  return 0;
}
