// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

#include <bits/stdc++.h>
#include "../../library/time-based-simulated-annealing.hpp"
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc005_patrol_sa.cpp
// Problem / scoring: https://atcoder.jp/contests/ahc005/tasks/ahc005_a
using namespace std;

struct Point {
  int row;
  int column;
};

struct Solver {
  static constexpr int INF = 1'000'000'000;

  int n = 0;
  int start_row = 0;
  int start_column = 0;
  vector<string> grid;

  vector<Point> roads;
  vector<vector<int>> road_id;
  vector<vector<int>> next_road;
  vector<int> enter_cost;

  vector<int> horizontal_segment;
  vector<int> vertical_segment;
  vector<vector<int>> horizontal_members;
  vector<vector<int>> vertical_members;

  vector<vector<unsigned long long>> visible_cells;
  int bit_words = 0;

  // terminal 0 is always the starting square.
  vector<int> terminals;
  vector<vector<int>> distance_between_terminals;
  vector<vector<int>> shortest_path_parent;

  bool is_road(int row, int column) const {
    return 0 <= row && row < n && 0 <= column && column < n &&
           grid[row][column] != '#';
  }

  char move_letter(int from, int to) const {
    const int dr = roads[to].row - roads[from].row;
    const int dc = roads[to].column - roads[from].column;
    if (dr == -1) return 'U';
    if (dr == 1) return 'D';
    if (dc == -1) return 'L';
    return 'R';
  }

  void read_input() {
    cin >> n >> start_row >> start_column;
    grid.resize(n);
    for (string& row : grid) cin >> row;

    road_id.assign(n, vector<int>(n, -1));
    for (int row = 0; row < n; ++row) {
      for (int column = 0; column < n; ++column) {
        if (!is_road(row, column)) continue;
        road_id[row][column] = static_cast<int>(roads.size());
        roads.push_back({row, column});
        enter_cost.push_back(grid[row][column] - '0');
      }
    }

    const int road_count = static_cast<int>(roads.size());
    next_road.assign(road_count, {});
    const int dr[4] = {-1, 0, 1, 0};
    const int dc[4] = {0, 1, 0, -1};
    for (int id = 0; id < road_count; ++id) {
      for (int direction = 0; direction < 4; ++direction) {
        const int nr = roads[id].row + dr[direction];
        const int nc = roads[id].column + dc[direction];
        if (is_road(nr, nc)) next_road[id].push_back(road_id[nr][nc]);
      }
    }
  }

  // Give every maximal horizontal/vertical road its own segment number.
  void build_segments() {
    const int road_count = static_cast<int>(roads.size());
    horizontal_segment.assign(road_count, -1);
    vertical_segment.assign(road_count, -1);

    for (int row = 0; row < n; ++row) {
      int column = 0;
      while (column < n) {
        if (!is_road(row, column)) {
          ++column;
          continue;
        }
        horizontal_members.push_back({});
        const int segment = static_cast<int>(horizontal_members.size()) - 1;
        while (column < n && is_road(row, column)) {
          const int id = road_id[row][column];
          horizontal_segment[id] = segment;
          horizontal_members.back().push_back(id);
          ++column;
        }
      }
    }

    for (int column = 0; column < n; ++column) {
      int row = 0;
      while (row < n) {
        if (!is_road(row, column)) {
          ++row;
          continue;
        }
        vertical_members.push_back({});
        const int segment = static_cast<int>(vertical_members.size()) - 1;
        while (row < n && is_road(row, column)) {
          const int id = road_id[row][column];
          vertical_segment[id] = segment;
          vertical_members.back().push_back(id);
          ++row;
        }
      }
    }
  }

  void set_bit(vector<unsigned long long>& bits, int index) const {
    bits[index / 64] |= 1ULL << (index % 64);
  }

  bool has_bit(const vector<unsigned long long>& bits, int index) const {
    return (bits[index / 64] >> (index % 64)) & 1ULL;
  }

  void build_visibility_sets() {
    const int road_count = static_cast<int>(roads.size());
    bit_words = (road_count + 63) / 64;
    visible_cells.assign(
        road_count, vector<unsigned long long>(bit_words, 0ULL));

    for (int id = 0; id < road_count; ++id) {
      for (int other : horizontal_members[horizontal_segment[id]]) {
        set_bit(visible_cells[id], other);
      }
      for (int other : vertical_members[vertical_segment[id]]) {
        set_bit(visible_cells[id], other);
      }
    }
  }

  int uncovered_gain(const vector<unsigned long long>& covered,
                     int candidate) const {
    int gain = 0;
    for (int word = 0; word < bit_words; ++word) {
      gain += __builtin_popcountll(visible_cells[candidate][word] &
                                   ~covered[word]);
    }
    return gain;
  }

  bool all_bits_are_set(const vector<unsigned long long>& covered) const {
    const int road_count = static_cast<int>(roads.size());
    for (int id = 0; id < road_count; ++id) {
      if (!has_bit(covered, id)) return false;
    }
    return true;
  }

  // Greedy set cover: choose squares which see many still-unseen roads.
  vector<int> choose_observation_points() const {
    const int road_count = static_cast<int>(roads.size());
    const int start = road_id[start_row][start_column];
    vector<int> chosen = {start};
    vector<char> is_chosen(road_count, false);
    is_chosen[start] = true;
    vector<unsigned long long> covered = visible_cells[start];

    while (!all_bits_are_set(covered)) {
      int best = -1;
      int best_gain = -1;
      for (int candidate = 0; candidate < road_count; ++candidate) {
        if (is_chosen[candidate]) continue;
        const int gain = uncovered_gain(covered, candidate);
        if (gain > best_gain ||
            (gain == best_gain && best != -1 &&
             enter_cost[candidate] < enter_cost[best])) {
          best = candidate;
          best_gain = gain;
        }
      }

      // Every road sees itself, so this is only a defensive fallback.
      if (best == -1 || best_gain <= 0) break;
      chosen.push_back(best);
      is_chosen[best] = true;
      for (int word = 0; word < bit_words; ++word) {
        covered[word] |= visible_cells[best][word];
      }
    }

    // Greedy choices made early can become redundant after later choices.
    vector<int> cover_count(road_count, 0);
    for (int point : chosen) {
      for (int id = 0; id < road_count; ++id) {
        cover_count[id] += has_bit(visible_cells[point], id);
      }
    }
    for (int index = static_cast<int>(chosen.size()) - 1; index >= 1;
         --index) {
      const int point = chosen[index];
      bool removable = true;
      for (int id = 0; id < road_count; ++id) {
        if (has_bit(visible_cells[point], id) && cover_count[id] == 1) {
          removable = false;
          break;
        }
      }
      if (!removable) continue;
      for (int id = 0; id < road_count; ++id) {
        cover_count[id] -= has_bit(visible_cells[point], id);
      }
      chosen.erase(chosen.begin() + index);
    }
    return chosen;
  }

  bool build_shortest_paths() {
    const int terminal_count = static_cast<int>(terminals.size());
    const int road_count = static_cast<int>(roads.size());
    distance_between_terminals.assign(
        terminal_count, vector<int>(terminal_count, INF));
    shortest_path_parent.assign(
        terminal_count, vector<int>(road_count, -1));

    for (int source_index = 0; source_index < terminal_count;
         ++source_index) {
      vector<int> distance(road_count, INF);
      priority_queue<pair<int, int>, vector<pair<int, int>>,
                     greater<pair<int, int>>>
          queue;
      const int source = terminals[source_index];
      distance[source] = 0;
      queue.push({0, source});

      while (!queue.empty()) {
        const auto [current_distance, current] = queue.top();
        queue.pop();
        if (current_distance != distance[current]) continue;

        for (int next : next_road[current]) {
          const int new_distance = current_distance + enter_cost[next];
          if (new_distance >= distance[next]) continue;
          distance[next] = new_distance;
          shortest_path_parent[source_index][next] = current;
          queue.push({new_distance, next});
        }
      }

      for (int destination_index = 0; destination_index < terminal_count;
           ++destination_index) {
        distance_between_terminals[source_index][destination_index] =
            distance[terminals[destination_index]];
        if (distance[terminals[destination_index]] == INF) return false;
      }
    }
    return true;
  }

  vector<int> make_nearest_neighbor_order() const {
    const int terminal_count = static_cast<int>(terminals.size());
    vector<int> order = {0};
    vector<char> used(terminal_count, false);
    used[0] = true;
    while (static_cast<int>(order.size()) < terminal_count) {
      int best = -1;
      for (int candidate = 1; candidate < terminal_count; ++candidate) {
        if (used[candidate]) continue;
        if (best == -1 ||
            distance_between_terminals[order.back()][candidate] <
                distance_between_terminals[order.back()][best]) {
          best = candidate;
        }
      }
      order.push_back(best);
      used[best] = true;
    }
    return order;
  }

  // Moving into a cell costs that cell's digit. For two terminals a and b,
  // dist(a,b)-cost(b) equals dist(b,a)-cost(a), so this value is symmetric.
  // It lets ordinary 2-opt and relocate deltas work on this directed-looking
  // distance table.
  int symmetric_distance(int first, int second) const {
    return distance_between_terminals[first][second] -
           enter_cost[terminals[second]];
  }

  bool apply_best_two_opt(vector<int>& order) const {
    const int size = static_cast<int>(order.size());
    int best_delta = 0;
    int best_left = -1;
    int best_right = -1;

    for (int left = 1; left + 1 < size; ++left) {
      for (int right = left + 1; right < size; ++right) {
        const int a = order[left - 1];
        const int b = order[left];
        const int c = order[right];
        const int d = order[(right + 1) % size];
        const int delta = symmetric_distance(a, c) +
                          symmetric_distance(b, d) -
                          symmetric_distance(a, b) -
                          symmetric_distance(c, d);
        if (delta < best_delta) {
          best_delta = delta;
          best_left = left;
          best_right = right;
        }
      }
    }
    if (best_left == -1) return false;
    reverse(order.begin() + best_left, order.begin() + best_right + 1);
    return true;
  }

  bool apply_best_relocate(vector<int>& order) const {
    const int size = static_cast<int>(order.size());
    int best_delta = 0;
    int best_index = -1;
    int best_after = -1;

    for (int index = 1; index < size; ++index) {
      const int point = order[index];
      const int previous = order[index - 1];
      const int next = order[(index + 1) % size];
      const int remove_delta = symmetric_distance(previous, next) -
                               symmetric_distance(previous, point) -
                               symmetric_distance(point, next);

      for (int edge = 0; edge < size; ++edge) {
        if (edge == index || (edge + 1) % size == index) continue;
        const int before = order[edge];
        const int after = order[(edge + 1) % size];
        const int delta = remove_delta + symmetric_distance(before, point) +
                          symmetric_distance(point, after) -
                          symmetric_distance(before, after);
        if (delta < best_delta) {
          best_delta = delta;
          best_index = index;
          best_after = before;
        }
      }
    }

    if (best_index == -1) return false;
    const int point = order[best_index];
    order.erase(order.begin() + best_index);
    const int after_index = static_cast<int>(
        find(order.begin(), order.end(), best_after) - order.begin());
    order.insert(order.begin() + after_index + 1, point);
    return true;
  }

  void improve_visit_order(vector<int>& order) const {
    while (true) {
      bool changed = false;
      changed |= apply_best_two_opt(order);
      changed |= apply_best_relocate(order);
      if (!changed) break;
    }
  }

  bool mark_shortest_path_segments(int from_index, int to_index,
                                   vector<char>& seen_horizontal,
                                   vector<char>& seen_vertical) const {
    const int source = terminals[from_index];
    int current = terminals[to_index];
    while (true) {
      seen_horizontal[horizontal_segment[current]] = true;
      seen_vertical[vertical_segment[current]] = true;
      if (current == source) break;
      current = shortest_path_parent[from_index][current];
      if (current == -1) return false;
    }
    return true;
  }

  bool route_covers_every_road(const vector<int>& order) const {
    vector<char> seen_horizontal(horizontal_members.size(), false);
    vector<char> seen_vertical(vertical_members.size(), false);
    const int size = static_cast<int>(order.size());
    for (int index = 0; index < size; ++index) {
      if (!mark_shortest_path_segments(order[index],
                                       order[(index + 1) % size],
                                       seen_horizontal, seen_vertical)) {
        return false;
      }
    }
    for (int id = 0; id < static_cast<int>(roads.size()); ++id) {
      if (!seen_horizontal[horizontal_segment[id]] &&
          !seen_vertical[vertical_segment[id]]) {
        return false;
      }
    }
    return true;
  }

  // A shortest path often observes extra roads. Remove the waypoint whose
  // deletion saves most, but only after checking the complete visibility rule.
  void remove_unneeded_waypoints(vector<int>& order,
      chrono::steady_clock::time_point deadline = chrono::steady_clock::time_point::max()) const {
    while (order.size() > 1) {
      int best_position = -1;
      int best_saving = -1;
      for (int position = 1; position < static_cast<int>(order.size());
           ++position) {
        if (chrono::steady_clock::now() >= deadline) return;
        vector<int> trial = order;
        trial.erase(trial.begin() + position);
        if (!route_covers_every_road(trial)) continue;

        const int previous = order[position - 1];
        const int point = order[position];
        const int next = order[(position + 1) % order.size()];
        const int saving =
            distance_between_terminals[previous][point] +
            distance_between_terminals[point][next] -
            distance_between_terminals[previous][next];
        if (saving > best_saving) {
          best_saving = saving;
          best_position = position;
        }
      }
      if (best_position == -1) break;
      order.erase(order.begin() + best_position);
    }
  }

  bool append_shortest_path(int from_index, int to_index,
                            string& answer) const {
    const int source = terminals[from_index];
    int current = terminals[to_index];
    vector<int> reversed_path;
    while (current != source) {
      reversed_path.push_back(current);
      current = shortest_path_parent[from_index][current];
      if (current == -1) return false;
    }
    reversed_path.push_back(source);
    reverse(reversed_path.begin(), reversed_path.end());
    for (int index = 1; index < static_cast<int>(reversed_path.size());
         ++index) {
      answer.push_back(
          move_letter(reversed_path[index - 1], reversed_path[index]));
    }
    return true;
  }

  string make_answer(const vector<int>& order) const {
    string answer;
    for (int index = 0; index < static_cast<int>(order.size()); ++index) {
      if (!append_shortest_path(order[index],
                                order[(index + 1) % order.size()], answer)) {
        return {};
      }
    }
    return answer;
  }

  bool answer_is_valid(const string& answer) const {
    int row = start_row;
    int column = start_column;
    vector<unsigned long long> covered(bit_words, 0ULL);
    const auto observe = [&](int road, vector<unsigned long long>& bits) {
      for (int word = 0; word < bit_words; ++word) {
        bits[word] |= visible_cells[road][word];
      }
    };
    observe(road_id[row][column], covered);

    for (char move : answer) {
      if (move == 'U') --row;
      if (move == 'D') ++row;
      if (move == 'L') --column;
      if (move == 'R') ++column;
      if (!is_road(row, column)) return false;
      observe(road_id[row][column], covered);
    }
    return row == start_row && column == start_column &&
           all_bits_are_set(covered);
  }

  void dfs_fallback(int current, vector<char>& visited, string& answer) const {
    visited[current] = true;
    for (int next : next_road[current]) {
      if (visited[next]) continue;
      answer.push_back(move_letter(current, next));
      dfs_fallback(next, visited, answer);
      answer.push_back(move_letter(next, current));
    }
  }

  string make_safe_fallback() const {
    vector<char> visited(roads.size(), false);
    string answer;
    dfs_fallback(road_id[start_row][start_column], visited, answer);
    return answer;
  }

  string solve() {
    build_segments();
    build_visibility_sets();
    terminals = choose_observation_points();

    if (!build_shortest_paths()) return make_safe_fallback();
    vector<int> order = make_nearest_neighbor_order();
    improve_visit_order(order);

    // Do this after order optimization: the check also includes the exact
    // shortest paths used by the current order.
    remove_unneeded_waypoints(order);
    string answer = make_answer(order);
    if (!answer_is_valid(answer)) answer = make_safe_fallback();
    return answer;
  }
};

// ===== 問題ごとに書く部分。局所探索の基本フォーマットと同じ5項目 =====
struct PatrolProblem {
  using Score = int;  // TODO: 大きいほど良い値。今回は「移動時間の負数」。
  struct Visit { int group, point; };
  // TODO: 現在解と差分cacheを書く。各groupの代表点を1回ずつ並べる。
  struct State { vector<Visit> order; int cost = 0; };
  // TODO: 近傍1回分。0=2-opt、1=代表点変更、2=代表点変更＋relocate。
  struct Move { int kind, left, right, point = -1; };
  Solver& map;
  vector<vector<int>> groups;
  int pending_change = 0;  // 評価時のscratch。Stateには含めない。

  explicit PatrolProblem(Solver& map_) : map(map_) {
    // 長さ2以上の各直線道路を1回見る十分条件を使う（必要条件ではない）。
    // 候補点は直線の交点＋開始点。交点がない直線には1点を追加する。
    const int count = (int)map.roads.size();
    vector<int> index(count, -1);
    map.terminals.clear();
    auto add = [&](int cell) {
      if (index[cell] < 0) {
        index[cell] = (int)map.terminals.size();
        map.terminals.push_back(cell);
      }
    };
    add(map.road_id[map.start_row][map.start_column]);
    for (int cell = 0; cell < count; ++cell) {
      if (map.horizontal_members[map.horizontal_segment[cell]].size() > 1 &&
          map.vertical_members[map.vertical_segment[cell]].size() > 1) add(cell);
    }
    auto add_groups = [&](const vector<vector<int>>& segments) {
      for (const auto& segment : segments) {
        if (segment.size() <= 1) continue;
        vector<int> candidates;
        for (int cell : segment) if (index[cell] >= 0) candidates.push_back(index[cell]);
        if (candidates.empty()) {
          int cell = *min_element(segment.begin(), segment.end(), [&](int a, int b) {
            return map.enter_cost[a] < map.enter_cost[b];
          });
          add(cell);
          candidates.push_back(index[cell]);
        }
        groups.push_back(std::move(candidates));
      }
    };
    add_groups(map.horizontal_members);
    add_groups(map.vertical_members);
    if (!map.build_shortest_paths()) throw runtime_error("disconnected road graph");
  }

  int distance(int a, int b) const { return map.distance_between_terminals[a][b]; }
  int symmetric(int a, int b) const {
    return distance(a,b) - map.enter_cost[map.terminals[b]];
  }
  int full_cost(const State& state) const {
    int total = 0, size = (int)state.order.size();
    for (int i = 0; i < size; ++i)
      total += distance(state.order[i].point, state.order[(i+1)%size].point);
    return total;
  }
  State initial_state() const {
    State state;
    state.order.push_back({-1, 0});  // 開始点は固定し、近傍では動かさない。
    vector<char> used(groups.size(), false);
    for (int step = 0; step < (int)groups.size(); ++step) {
      int best_group = -1, best_point = -1, best_distance = Solver::INF;
      for (int group = 0; group < (int)groups.size(); ++group) {
        if (used[group]) continue;
        for (int point : groups[group]) {
          int d = distance(state.order.back().point, point);
          if (d < best_distance) { best_group=group; best_point=point; best_distance=d; }
        }
      }
      used[best_group] = true;
      state.order.push_back({best_group, best_point});
    }
    state.cost = full_cost(state);
    return state;
  }

  // TODO: 試したい変更を1個返す。作れない試行はnulloptでスキップする。
  // 同じgroup内の代表点変更なら、可視性を壊さずに地点選択も最適化できる。
  optional<Move> propose_move(const State& state, mt19937_64& rng, double) {
    const int size = (int)state.order.size();
    if (size <= 1) return nullopt;
    const int kind = rng()%3;
    const int left = 1+rng()%(size-1);
    if (kind == 0) {
      const int right = 1+rng()%(size-1);
      if (left == right) return nullopt;
      return Move{0, min(left,right), max(left,right)};
    }
    if (kind == 1) {
      const auto& candidates = groups[state.order[left].group];
      const int point = candidates[rng()%candidates.size()];
      if (point == state.order[left].point) return nullopt;
      return Move{1, left, 0, point};
    }
    const auto& choices = groups[state.order[left].group];
    const int node = choices[rng()%choices.size()];
    int best_edge = -1, best_increase = Solver::INF;
    // 地点を選んだら、挿入先は全ての辺を調べる。削除差は挿入先によらない。
    for (int edge = 0; edge < size; ++edge) {
      if (edge == left || edge+1 == left) continue;
      const int a = state.order[edge].point, b = state.order[(edge+1)%size].point;
      const int increase = distance(a,node) + distance(node,b) - distance(a,b);
      if (increase < best_increase) { best_increase = increase; best_edge = edge; }
    }
    if (best_edge < 0) return nullopt;
    return Move{2, left, best_edge, node};
  }

  // TODO: 「変更後score−変更前score」を返す。距離なら負号を付ける。
  // 2-optは境界4辺、代表点変更は前後2辺、relocateは削除・挿入の6辺だけ。
  // 閾値は安全な途中打ち切りができる時だけ使う。このO(1)評価では無視してよい。
  optional<Score> evaluate_move(const State& state, const Move& move, double threshold) {
    (void)threshold;
    const int size = (int)state.order.size(), i=move.left, j=move.right;
    auto point = [&](int k) { return state.order[k%size].point; };
    int change = 0;
    if (move.kind == 0) {
      // 到着マスの料金を引くと対称距離になる。区間内部の逆向き差は相殺。
      change = symmetric(point(i-1),point(j)) + symmetric(point(i),point(j+1))
             - symmetric(point(i-1),point(i)) - symmetric(point(j),point(j+1));
    } else if (move.kind == 1) {
      change = distance(point(i-1),move.point) + distance(move.point,point(i+1))
             - distance(point(i-1),point(i)) - distance(point(i),point(i+1));
    } else {
      change = distance(point(i-1),point(i+1))
             - distance(point(i-1),point(i)) - distance(point(i),point(i+1))
             + distance(point(j),move.point) + distance(move.point,point(j+1))
             - distance(point(j),point(j+1));
    }
    pending_change = change;
    return -change;
  }

  // TODO: 採用した手だけ反映する。棄却時は何もしないのでrollback不要。
  void apply_move(State& state, Move& move) {
    if (move.kind == 0) {
      reverse(state.order.begin()+move.left, state.order.begin()+move.right+1);
    } else if (move.kind == 1) {
      state.order[move.left].point = move.point;
    } else {
      Visit visit = state.order[move.left];
      visit.point = move.point;
      state.order.erase(state.order.begin()+move.left);
      const int after = move.right - (move.right > move.left);
      state.order.insert(state.order.begin()+after+1, visit);
    }
    state.cost += pending_change;
  }
};
// ===== ここまでが問題依存。時計・温度・採否・最良解の管理はRunner =====

#ifndef AHC005_TEST
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  const auto started = chrono::steady_clock::now();
  auto elapsed = [&] { return chrono::duration<double,milli>(chrono::steady_clock::now()-started).count(); };
  Solver solver;
  solver.read_input();
  string answer = solver.solve();  // 既存の合法な短時間解も保険として保存する。
  auto travel_cost = [&](const string& route) {
    int r=solver.start_row, c=solver.start_column, cost=0;
    for (char ch : route) {
      if (ch == 'U') --r;
      if (ch == 'D') ++r;
      if (ch == 'L') --c;
      if (ch == 'R') ++c;
      cost += solver.grid[r][c]-'0';
    }
    return cost;
  };
  const int legacy_cost = travel_cost(answer);
  PatrolProblem problem(solver);
  auto initial = problem.initial_state();
  const int initial_score = -initial.cost;
  const double remaining = 2600.0-elapsed();
  vector<int> order;
  if (remaining > 0) {
    TimeBasedAnnealingRunner<PatrolProblem> runner(
        problem, std::move(initial), initial_score, remaining, 30.0, 1.0, 5, 256);
    runner.run();
    for (auto visit : runner.best_state().order) order.push_back(visit.point);
    cerr << "iterations=" << runner.iterations() << '\n';
  } else {
    for (auto visit : initial.order) order.push_back(visit.point);
  }
  // 通過途中の視界も利用し、期限内だけ不要な代表点を削る。
  solver.remove_unneeded_waypoints(order, started+chrono::milliseconds(2820));
  string candidate = solver.make_answer(order);
  if (solver.answer_is_valid(candidate) && travel_cost(candidate) < legacy_cost)
    answer = std::move(candidate);
  cerr << "travel=" << travel_cost(answer) << " legacy=" << legacy_cost << '\n';
  cout << answer << '\n';
}
#endif
