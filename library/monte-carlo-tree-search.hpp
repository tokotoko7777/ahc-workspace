#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/monte-carlo-tree-search.hpp
// UCTの着想: https://doi.org/10.1007/11871842_29
// 独自実装。確率遷移の結果を別の子として保持するclosed-loop UCT。

struct MctsOptions {
  double time_limit_ms = 100;
  double exploration = 0.5; // rolloutの値域[0,1]に合わせる。
  bool maximize = true;
  int max_depth = 100;
  int max_nodes = 10000; // 上限後もrolloutと既存辺の統計更新は続ける。
  std::uint64_t iteration_limit = std::numeric_limits<std::uint64_t>::max();
};

// 必須: State, Action, generate_actions, is_terminal,
// sample_transition(State&, Action, rng)->uint64_t, rollout(State, rng)->double。
// transitionの戻り値は「同じ親・同じActionから出た確率的結果」の正確な識別子。
// 決定的遷移なら0でよい。違う結果を同じidにまとめると別状態の統計が混ざる！
// rolloutは終端/打切り評価を[0,1]へ正規化して返す。最小化にも同じ値域を使う。
// 外部の実状態は変更しない。木に盤面を保存せず、各試行でrootをコピーして進める。
template <class Problem> class MonteCarloTreeSearch {
 public:
  using State = typename Problem::State;
  using Action = typename Problem::Action;
  explicit MonteCarloTreeSearch(Problem& problem, std::uint64_t seed = 0)
      : problem_(problem), rng_(seed) {}
  std::optional<Action> choose_action(const State& root, MctsOptions options = {}) {
    if (!std::isfinite(options.time_limit_ms) || options.time_limit_ms <= 0 ||
        !std::isfinite(options.exploration) || options.exploration < 0 ||
        options.max_depth <= 0 || options.max_nodes <= 0)
      throw std::invalid_argument("invalid MCTS options");
    const auto start = Clock::now();
    nodes_.clear(); path_.clear(); iterations_ = 0;
    path_.reserve(options.max_depth);
    if (problem_.is_terminal(root)) return std::nullopt;
    add_node(root);
    if (nodes_[0].edges.empty()) return std::nullopt;
    while (iterations_ < options.iteration_limit &&
           std::chrono::duration<double, std::milli>(Clock::now() - start).count() < options.time_limit_ms) {
      State state = root;
      int node = 0;
      path_.clear();
      for (int depth = 0; depth < options.max_depth && !problem_.is_terminal(state); ++depth) {
        if (nodes_[node].edges.empty()) break;
        const int edge_id = select_edge(node, options);
        const Action action = nodes_[node].edges[edge_id].action;
        path_.emplace_back(node, edge_id);
        const std::uint64_t outcome = problem_.sample_transition(state, action, rng_);
        if (problem_.is_terminal(state)) break;
        if (depth + 1 == options.max_depth) break; // この深さより先の未使用ノードは作らない。
        int child = -1;
        for (const auto& item : nodes_[node].edges[edge_id].children)
          if (item.first == outcome) { child = item.second; break; }
        if (child < 0) {
          if (static_cast<int>(nodes_.size()) < options.max_nodes) {
            child = add_node(state); // vector再確保後も参照を持ち越さずindexでアクセス。
            nodes_[node].edges[edge_id].children.emplace_back(outcome, child);
          }
          break; // 1試行につき最大1決定ノードを追加。その先はrollout。
        }
        node = child;
      }
      const double reward = problem_.rollout(std::move(state), rng_);
      if (!std::isfinite(reward) || reward < 0 || reward > 1)
        throw std::invalid_argument("MCTS rollout must return a finite reward in [0,1]");
      for (const auto& item : path_) {
        auto& decision = nodes_[item.first];
        auto& edge = decision.edges[item.second];
        ++decision.visits; ++edge.visits; edge.sum += reward;
      }
      ++iterations_;
    }
    // 探索時はUCB、最終選択は訪問回数最大（同数なら平均が良い方）。
    int best = 0;
    for (int id = 1; id < static_cast<int>(nodes_[0].edges.size()); ++id) {
      const auto& a = nodes_[0].edges[id]; const auto& b = nodes_[0].edges[best];
      if (a.visits > b.visits || (a.visits == b.visits && a.visits &&
          (options.maximize ? a.sum > b.sum : a.sum < b.sum))) best = id;
    }
    return nodes_[0].edges[best].action; // 0反復でも最初の合法手を返す。
  }
  std::uint64_t iterations() const { return iterations_; }
  std::size_t nodes() const { return nodes_.size(); }
 private:
  using Clock = std::chrono::steady_clock;
  struct Edge {
    Action action;
    std::uint64_t visits = 0;
    double sum = 0;
    std::vector<std::pair<std::uint64_t, int>> children;
  };
  struct Node { std::vector<Edge> edges; std::uint64_t visits = 0; };
  int add_node(const State& state) {
    Node node;
    for (const auto& action : problem_.generate_actions(state)) node.edges.push_back(Edge{action, 0, 0, {}});
    nodes_.push_back(std::move(node));
    return static_cast<int>(nodes_.size()) - 1;
  }
  int select_edge(int id, const MctsOptions& options) const {
    const auto& node = nodes_[id];
    const double log_visits = std::log(static_cast<double>(node.visits) + 1);
    int best = 0; double best_value = -std::numeric_limits<double>::infinity();
    for (int k = 0; k < static_cast<int>(node.edges.size()); ++k) {
      const auto& edge = node.edges[k];
      if (!edge.visits) return k;
      const double mean = edge.sum / edge.visits;
      const double value = (options.maximize ? mean : -mean) +
          options.exploration * std::sqrt(log_visits / edge.visits);
      if (value > best_value) { best = k; best_value = value; }
    }
    return best;
  }
  Problem& problem_;
  std::mt19937_64 rng_;
  std::vector<Node> nodes_;
  std::vector<std::pair<int, int>> path_;
  std::uint64_t iterations_ = 0;
};
