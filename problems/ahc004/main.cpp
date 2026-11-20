// AHC_SOURCE: examples/search/ahc004_genome_sa.cpp
// Source repository: https://github.com/tokotoko7777/ahc-workspace
// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

// Record the selected initial-board strategy; the prior example remains unchanged.
#if !defined(AHC_BASELINE_SEED) && !defined(AHC_CONTIG_SEED)
#define AHC_CONTIG_SEED
#endif

#include <bits/stdc++.h>
#include "../../library/time-based-simulated-annealing.hpp"
#include "../../library/aho-corasick.hpp"
#include "../../library/coverage-counter.hpp"
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/examples/search/ahc004_genome_sa.cpp
// Problem / scoring: https://atcoder.jp/contests/ahc004/tasks/ahc004_a
using namespace std;

// library/timer.hpp
struct Timer {
  chrono::steady_clock::time_point start;

  Timer() : start(chrono::steady_clock::now()) {}

  double elapsed_ms() const {
    const auto now = chrono::steady_clock::now();
    return chrono::duration<double, milli>(now - start).count();
  }
};

// library/random.hpp
struct Random {
  mt19937_64 engine;

  explicit Random(uint64_t seed = 0) : engine(seed) {}

  uint64_t next_u64() { return engine(); }

  template <class Int>
  Int next_int(Int left, Int right) {
    assert(left < right);
    uniform_int_distribution<Int> distribution(left, right - 1);
    return distribution(engine);
  }

  double next_double() {
    return uniform_real_distribution<double>(0.0, 1.0)(engine);
  }
};

struct Target {
  string text;
  int frequency = 0;
  int covered_weight = 0;
  int overlap_strength = 0;
  bool maximal = true;
};

// library/sequence-overlap.hpp
template <class Sequence>
int suffix_prefix_overlap(const Sequence& first, const Sequence& second) {
  const int limit = static_cast<int>(min(first.size(), second.size()));
  for (int length = limit; length >= 1; --length) {
    bool same = true;
    for (int index = 0; index < length; ++index) {
      if (first[first.size() - length + index] != second[index]) {
        same = false;
        break;
      }
    }
    if (same) return length;
  }
  return 0;
}


// ===== 問題ごとに書く部分（local-search/basic.cppと同じインターフェース） =====
struct GenomeProblem {
  using Score = double;
  using Board = vector<string>;
  // TODO: 現在解と、差分評価で参照するcacheを書く。
  // line_hitsには同じ語の複数出現も残す。countが0になる時だけ失点する。
  struct State {
    Board board;
    vector<vector<int>> line_hits;
    CoverageCounter<int>::State coverage;
    vector<uint64_t> line_code, line_empty;
#ifdef AHC004_FAST_PLACEMENT
    vector<array<uint64_t,20>> rotated_code, rotated_empty;
#endif
    int empty = 0;
  };
  // TODO: 1近傍で変更する場所と変更後の値を書く（評価時にはStateを触らない）。
  struct Move {
    vector<pair<int, char>> changes;
    int invariant_line = -1, swap_line = -1;
  };
  int n, m;
  vector<Target> targets;
  CoverageCounter<int> coverage;
  AhoCorasick<8> matcher;
  int max_length = 0;
  vector<vector<int>> line_cells;
  // 採用判定前の一時領域。最良Stateにコピーする必要はないのでProblemに置く。
  Board scratch;
  vector<int> changed_lines;
  vector<vector<int>> next_hits;
  int next_covered = 0, next_empty = 0;

  static vector<int> coverage_weights(const vector<Target>& targets) {
    vector<int> weights;
    for (const auto& target : targets) weights.push_back(target.frequency);
    return weights;
  }

  GenomeProblem(int n_, int m_, const vector<Target>& targets_)
      : n(n_), m(m_), targets(targets_), coverage(coverage_weights(targets_)),
        line_cells(2*n), next_hits(2*n) {
    for (int id = 0; id < (int)targets.size(); ++id) {
      vector<int> symbols;
      for (char c : targets[id].text) symbols.push_back(c - 'A');
      matcher.add(symbols, id);
      max_length = max(max_length, (int)symbols.size());
    }
    matcher.build();
    for (int line = 0; line < 2*n; ++line) {
      for (int p = 0; p < n + max_length - 1; ++p) {
        line_cells[line].push_back(line < n ? line*n+p%n : (p%n)*n+line-n);
      }
      next_hits[line].reserve(n * max_length);
    }
  }

  // 20文字+最大長-1文字だけ読む。開始位置0..19にある一致だけを数える。
  void scan_line(const Board& board, int line, vector<int>& hits) const {
    hits.clear();
    int state = 0;
    for (int p = 0; p < (int)line_cells[line].size(); ++p) {
      const int cell = line_cells[line][p];
      const char c = board[cell/n][cell%n];
      if (c == '.') { state = 0; continue; }
      state = matcher.advance(state, c - 'A');
      for (int id : matcher.matches(state)) {
        if (p + 1 - (int)targets[id].text.size() < n) hits.push_back(id);
      }
    }
  }

  void encode_line(State& state, int line) const {
    uint64_t code = 0, empty = 0;
    for (int p = 0; p < n; ++p) {
      int cell = line_cells[line][p];
      char c = state.board[cell/n][cell%n];
      if (c == '.') empty |= 1ULL << (3*p);
      else code |= uint64_t(c-'A') << (3*p);
    }
    state.line_code[line] = code;
    state.line_empty[line] = empty;
#ifdef AHC004_FAST_PLACEMENT
    for (int start = 0; start < n; ++start) {
      const int sh = 3*start, inv = 3*(n-start);
      state.rotated_code[line][start] = (code >> sh) | (code << inv);
      state.rotated_empty[line][start] = (empty >> sh) | (empty << inv);
    }
#endif
  }

  // 3bit/文字。XOR後の各3bitを1bitへ畳み、popcountで一致数を数える。
  // 空白は別bit列に持つので、'A'と取り違えない。
  int placement_quality(const State& state, int line, int start,
                        uint64_t word, int length) const {
    auto rotate = [&](uint64_t bits) {
      return (bits >> (3*start)) | (bits << (3*(n-start)));
    };
    const uint64_t low_bits = ((1ULL << (3*length))-1)/7;
    const uint64_t empty = rotate(state.line_empty[line]) & low_bits;
    const uint64_t diff = rotate(state.line_code[line]) ^ word;
    const uint64_t mismatch = (diff | (diff>>1) | (diff>>2) | empty) & low_bits;
    const int same = length - __builtin_popcountll(mismatch);
    return 1250*same + 330*__builtin_popcountll(empty) - 250*length;
  }

  State make_state(Board board) const {
    State state;
    state.board = std::move(board);
    state.line_hits.resize(2*n);
    state.line_code.resize(2*n);
    state.line_empty.resize(2*n);
#ifdef AHC004_FAST_PLACEMENT
    assert(n<=20);
    state.rotated_code.resize(2*n);
    state.rotated_empty.resize(2*n);
#endif
    state.coverage = coverage.make_state();
    for (int line = 0; line < 2*n; ++line) {
      encode_line(state, line);
      scan_line(state.board, line, state.line_hits[line]);
      for (int id : state.line_hits[line]) coverage.add(state.coverage, id);
    }
    for (const string& row : state.board) state.empty += count(row.begin(), row.end(), '.');
    return state;
  }

  Score score(int covered, int empty) const {
    // 全語を含む時だけ空白bonus。公式点の丸め前をm/1e8倍した尺度。
    return covered < m ? covered : double(m) * (2*n*n) / (2*n*n-empty);
  }
  Score score(const State& state) const { return score(state.coverage.covered_weight, state.empty); }

  // TODO: 問題に合う近傍を1つ生成する。nulloptなら今回の試行はスキップ。
  // ここでは未収録語を重なりのよい場所へ上書きする。
#if defined(AHC004_FAST_PLACEMENT) && defined(__GNUC__) && !defined(__clang__) && \
    (defined(__x86_64__) || defined(__i386__))
  // GCC dispatches after checking the host feature; generic builds stay safe.
  __attribute__((target_clones("popcnt","default")))
#endif
  optional<Move> propose_move(const State& state, mt19937_64& rng, double) {
    auto random_int = [&](int upper) {
      return uniform_int_distribution<int>(0, upper-1)(rng);
    };
    if (state.coverage.covered_weight == m) {
      const int cell = random_int(n*n);
      if (state.board[cell/n][cell%n] == '.') return nullopt;
      return Move{{{cell, '.'}}};
    }
    int chosen = -1;
    for (int trial = 0; trial < 32; ++trial) {
      int id = random_int((int)targets.size());
      if (!state.coverage.counts[id]) { chosen = id; break; }
    }
    if (chosen < 0) {
      const int first = random_int((int)targets.size());
      for (int k = 0; k < (int)targets.size(); ++k) {
        int id = (first+k) % targets.size();
        if (!state.coverage.counts[id]) { chosen = id; break; }
      }
    }
    if (chosen < 0) return nullopt;
    const string& text = targets[chosen].text;
    uint64_t word = 0;
    for (int k = 0; k < (int)text.size(); ++k) word |= uint64_t(text[k]-'A') << (3*k);
    int best_quality = INT_MIN, best_line = 0, best_start = 0, ties = 0;
#ifdef AHC004_EXACT_PLACEMENT
    const bool rerank = (rng() & 3) == 0;
    array<int,40> line_quality, line_start, line_ties{};
    if (rerank) line_quality.fill(INT_MIN);
#endif
#ifdef AHC004_FAST_PLACEMENT
    const int length = int(text.size());
    const uint64_t low_bits = ((1ULL << (3*length))-1)/7;
    for (int line = 0; line < 2*n; ++line) for (int start = 0; start < n; ++start) {
      const uint64_t empty = state.rotated_empty[line][start] & low_bits;
      const uint64_t diff = state.rotated_code[line][start] ^ word;
      const uint64_t mismatch = (diff | (diff>>1) | (diff>>2) | empty) & low_bits;
      const int quality = 1250*(length - __builtin_popcountll(mismatch))
                         + 330*__builtin_popcountll(empty) - 250*length;
#else
    for (int trial = 0; trial < 2*n*n; ++trial) {
      const int line = trial/n, start = trial%n;
      const int quality = placement_quality(state, line, start, word, (int)text.size());
#endif
#ifdef AHC004_EXACT_PLACEMENT
      if (rerank) {
        if (quality > line_quality[line]) {
          line_quality[line] = quality; line_start[line] = start; line_ties[line] = 1;
        } else if (quality == line_quality[line] && random_int(++line_ties[line]) == 0) {
          line_start[line] = start;
        }
      }
#endif
      // 同点を1/2で置換すると走査末尾に偏る。k個目は1/kで置換する。
      if (quality > best_quality) {
        best_quality = quality; best_line = line; best_start = start; ties = 1;
      } else if (quality == best_quality && random_int(++ties) == 0) {
        best_line = line; best_start = start;
      }
    }
    auto build = [&](int line, int start) {
      Move move;
      move.changes.reserve(text.size());
      for (int k = 0; k < (int)text.size(); ++k) {
        int cell = line_cells[line][start+k];
        if (state.board[cell/n][cell%n] != text[k]) move.changes.emplace_back(cell, text[k]);
      }
      return move;
    };
    Move move = build(best_line, best_start);
#ifdef AHC004_EXACT_PLACEMENT
    if (rerank && !move.changes.empty()) {
      Score best_exact = *evaluate_move(state, move, -INFINITY);
      line_quality[best_line] = INT_MIN;
      // The geometric pass is only a filter. Compare alternatives on other
      // lines using exactly the same coverage delta that SA will accept.
      for (int alternative = 0; alternative < 3; ++alternative) {
        const int line = max_element(line_quality.begin(), line_quality.begin()+2*n)
                         - line_quality.begin();
        if (line_quality[line] < best_quality-2500) break;
        line_quality[line] = INT_MIN;
        Move next = build(line, line_start[line]);
        if (next.changes.empty()) continue;
        const Score exact = *evaluate_move(state, next, -INFINITY);
        if (exact > best_exact ||
            (exact == best_exact && next.changes.size() < move.changes.size())) {
          best_exact = exact;
          move = std::move(next);
        }
      }
    }
#endif
    if (move.changes.empty()) return nullopt;
    return move;
  }

  // A full cyclic line preserves that line's hit multiset. Swapping two
  // parallel lines exchanges their hit multisets without changing coverage.
  optional<Move> line_cycle(const State& state,int line,int amount) const {
    Move move;
    move.invariant_line=line;
    for(int k=0;k<n;++k) {
      int from=line_cells[line][(k+amount)%n],to=line_cells[line][k];
      char c=state.board[from/n][from%n];
      if(state.board[to/n][to%n]!=c)move.changes.emplace_back(to,c);
    }
    if(move.changes.empty())return nullopt;
    return move;
  }
  optional<Move> line_swap(const State& state,int line,int other) const {
    Move move;
    move.invariant_line=line;move.swap_line=other;
    assert(line/n==other/n);
    for(int k=0;k<n;++k) {
      int a=line_cells[line][k],b=line_cells[other][k];
      char x=state.board[a/n][a%n],y=state.board[b/n][b%n];
      if(x!=y) {
        move.changes.emplace_back(a,y);move.changes.emplace_back(b,x);
      }
    }
    if(move.changes.empty())return nullopt;
    return move;
  }

  // TODO: 「変更後score - 変更前score」を返す。負の値もそのまま返してよい。
  // thresholdを超えないと証明できる場合だけnulloptで打ち切れる。
  // この版では上限判定を入れず、正確な差分を最後まで求める。
  optional<Score> evaluate_move(const State& state, const Move& move, double threshold) {
    (void)threshold;
    coverage.clear_delta();
    scratch = state.board;
    next_empty = state.empty;
    uint64_t line_mask = 0;
    for (auto [cell, c] : move.changes) {
      next_empty += (c == '.') - (scratch[cell/n][cell%n] == '.');
      scratch[cell/n][cell%n] = c;
      line_mask |= (1ULL << (cell/n)) | (1ULL << (n+cell%n));
    }
    if(move.invariant_line>=0)line_mask &= ~(1ULL << move.invariant_line);
    if(move.swap_line>=0)line_mask &= ~(1ULL << move.swap_line);
    changed_lines.clear();
    while (line_mask) {
      int line = __builtin_ctzll(line_mask);
      line_mask &= line_mask - 1;
      changed_lines.push_back(line);
      for (int id : state.line_hits[line]) coverage.add_delta(id, -1);
      scan_line(scratch, line, next_hits[line]);
      for (int id : next_hits[line]) coverage.add_delta(id, 1);
    }
    next_covered = state.coverage.covered_weight + coverage.score_delta(state.coverage);
    return score(next_covered, next_empty) - score(state);
  }

  // TODO: 採用した手だけ反映する。直前のevaluate_moveのcacheを使う。
  // 不採用なら呼ばれないので、rollbackは不要。
  void apply_move(State& state, Move& move) {
    coverage.apply_delta(state.coverage);
    state.board.swap(scratch);
    for (int line : changed_lines) {
      state.line_hits[line].swap(next_hits[line]);
      encode_line(state, line);
    }
    if(move.swap_line>=0)std::swap(state.line_hits[move.invariant_line],state.line_hits[move.swap_line]);
    if(move.invariant_line>=0)encode_line(state,move.invariant_line);
    if(move.swap_line>=0)encode_line(state,move.swap_line);
    state.empty = next_empty;
  }
};
// ===== ここまでが問題依存。時計・温度・採否・最良解保存はRunnerへ任せる =====

#ifndef AHC004_TEST
#ifdef AHC_CONTIG_SEED
#include "../../library/circular-pattern-components.hpp"

// Only consistent high-evidence suffix/prefix overlaps form initial contigs.
// Contigs may cross the arbitrary root of a length-n circular line.
static vector<string> seed_from_contigs(int n,const vector<Target>& targets,
        const vector<int>& maximal_indices,Random& random,int length_cut) {
  vector<string> p;
  for(int i:maximal_indices) p.push_back(targets[i].text);
  CircularPatternComponents groups(n,p);
  struct Edge {int a,b,i,k; uint64_t tie;};
  vector<Edge> edges;
  for(int i=0;i<int(p.size());++i)for(int j=0;j<int(p.size());++j)if(i!=j) {
    int ov=suffix_prefix_overlap(p[i],p[j]);
    if(ov<length_cut || ov>=int(min(p[i].size(),p[j].size())))continue;
    edges.push_back({i,j,int(p[i].size())-ov,ov,random.next_u64()});
  }
  sort(edges.begin(),edges.end(),[](const Edge& a,const Edge& b){
    return tie(a.k,a.tie)>tie(b.k,b.tie);
  });
  for(const auto& e:edges) groups.join(e.a,e.b,e.i);
  vector<int> count(p.size());
  for(int i=0;i<int(p.size());++i)count[groups.leader(i)]+=targets[maximal_indices[i]].covered_weight;
  vector<pair<pair<int,int>,int>> order;
  for(int r=0;r<int(p.size());++r)if(count[r]) {
    auto& word=groups.pattern(r);
    int length=n-std::count(word.begin(),word.end(),'.');
    order.push_back({{length,count[r]},r});
  }
  sort(order.rbegin(),order.rend());
  vector<string> board(n,string(n,'.'));
  for(auto [t,r]:order) {
    const string& pattern=groups.pattern(r);
    int best=-1000000,line_best=-1,pos_best=-1,axis_best=-1,ties=0;
    vector<pair<int,char>> known;
    for(int k=0;k<n;++k)if(pattern[k]!='.')known.push_back({k,pattern[k]});
    for(int axis=0;axis<2;++axis)for(int line=0;line<n;++line) {
      int fullness=0;
      for(int pos=0;pos<n;++pos)fullness+=(axis?board[pos][line]:board[line][pos])!='.';
      for(int start=0;start<n;++start) {
        int hits=0;
        bool fit=true;
        for(auto [k,c]:known) {
          int z=(start+k)%n;
          char prev=axis?board[z][line]:board[line][z];
          if(prev!='.'&&prev!=c){fit=false;break;}
          hits+=prev==c;
        }
        if(!fit)continue;
        int objective=100*hits-(hits==0?fullness:0);
        if(objective>best)best=objective,ties=0;
        else if(objective<best)continue;
        if(random.next_int(0,++ties)==0)line_best=line,pos_best=start,axis_best=axis;
      }
    }
    if(line_best<0)continue;
    for(auto [k,c]:known) {
      int z=(pos_best+k)%n;
      (axis_best ? board[z][line_best] : board[line_best][z])=c;
    }
  }
  return board;
}
#endif

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, m;
  cin >> n >> m;
  vector<string> input_strings(m);
  map<string, int> frequency;
  uint64_t input_hash = 1469598103934665603ULL;
  for (string& text : input_strings) {
    cin >> text;
    ++frequency[text];
    for (char letter : text) {
      input_hash ^= static_cast<unsigned char>(letter);
      input_hash *= 1099511628211ULL;
    }
  }
  Timer timer;
  constexpr double GREEDY_END_MS = 150.0;
  constexpr double SEARCH_END_MS = 2780.0;

  vector<Target> targets;
  targets.reserve(frequency.size());
  for (const auto& [text, count] : frequency) {
    targets.push_back({text, count});
  }

  const int target_count = static_cast<int>(targets.size());
  for (int i = 0; i < target_count; ++i) {
    for (int j = 0; j < target_count; ++j) {
      if (i == j) continue;
      if (targets[i].text.size() < targets[j].text.size() &&
          targets[j].text.find(targets[i].text) != string::npos) {
        targets[i].maximal = false;
      }
    }
  }

  vector<int> maximal_indices;
  for (int i = 0; i < target_count; ++i) {
    if (!targets[i].maximal) continue;
    maximal_indices.push_back(i);
    for (int j = 0; j < target_count; ++j) {
      if (targets[i].text.find(targets[j].text) != string::npos) {
        targets[i].covered_weight += targets[j].frequency;
      }
    }
  }

  for (int i : maximal_indices) {
    array<int, 3> largest{};
    for (int j : maximal_indices) {
      if (i == j) continue;
      const int overlap = max(
          suffix_prefix_overlap(targets[i].text, targets[j].text),
          suffix_prefix_overlap(targets[j].text, targets[i].text));
      if (overlap > largest[0]) {
        largest[0] = overlap;
        sort(largest.begin(), largest.end());
      }
    }
    targets[i].overlap_strength = largest[0] + largest[1] + largest[2];
  }

  using Board = vector<string>;

  const auto is_present = [&](const Board& board, const string& text) {
    const int length = static_cast<int>(text.size());
    for (int direction = 0; direction < 2; ++direction) {
      for (int line = 0; line < n; ++line) {
        for (int start = 0; start < n; ++start) {
          bool same = true;
          for (int offset = 0; offset < length; ++offset) {
            const int row = direction == 0 ? line : (start + offset) % n;
            const int column = direction == 0 ? (start + offset) % n : line;
            if (board[row][column] != text[offset]) {
              same = false;
              break;
            }
          }
          if (same) return true;
        }
      }
    }
    return false;
  };

  const auto count_score = [&](const Board& board) {
    int covered = 0;
    for (const Target& target : targets) {
      if (is_present(board, target.text)) covered += target.frequency;
    }
    int empty = 0;
    for (const string& row : board) {
      empty += count(row.begin(), row.end(), '.');
    }
    return pair<int, int>{covered, empty};
  };

  Random random(input_hash);
  Board best_board(n, string(n, '.'));
  pair<int, int> best_score{-1, -1};
  int attempt = 0;

  do {
    Board board(n, string(n, '.'));
#ifdef AHC_CONTIG_SEED
    if(attempt%3==1)board=seed_from_contigs(n,targets,maximal_indices,random,4+(attempt/3)%3);
#endif
    vector<int> order = maximal_indices;
    vector<uint64_t> priority(target_count);
    for (int index : order) {
      const uint64_t base =
          1000000ULL * targets[index].text.size() +
          5000ULL * targets[index].covered_weight +
          3000ULL * targets[index].overlap_strength;
      const uint64_t noise =
          attempt == 0 ? 0 : random.next_u64() % 2500000ULL;
      priority[index] = base + noise;
    }
    sort(order.begin(), order.end(), [&](int first, int second) {
      return priority[first] > priority[second];
    });

    vector<int> remaining;
    remaining.reserve(target_count);
    for (int i = 0; i < target_count; ++i) {
      if (!targets[i].maximal) remaining.push_back(i);
    }
    sort(remaining.begin(), remaining.end(), [&](int first, int second) {
      if (targets[first].frequency != targets[second].frequency) {
        return targets[first].frequency > targets[second].frequency;
      }
      return targets[first].text.size() > targets[second].text.size();
    });
    order.insert(order.end(), remaining.begin(), remaining.end());

    for (int target_index : order) {
      const string& text = targets[target_index].text;
      if (is_present(board, text)) continue;

      long long best_quality = numeric_limits<long long>::min();
      int best_direction = -1;
      int best_line = -1;
      int best_start = -1;
      int equal_candidates = 0;

      for (int direction = 0; direction < 2; ++direction) {
        for (int line = 0; line < n; ++line) {
          int line_filled = 0;
          for (int position = 0; position < n; ++position) {
            const int row = direction == 0 ? line : position;
            const int column = direction == 0 ? position : line;
            line_filled += board[row][column] != '.';
          }

          for (int start = 0; start < n; ++start) {
            bool compatible = true;
            int matching = 0;
            int new_cells = 0;
            for (int offset = 0; offset < static_cast<int>(text.size());
                 ++offset) {
              const int row = direction == 0 ? line : (start + offset) % n;
              const int column =
                  direction == 0 ? (start + offset) % n : line;
              const char current = board[row][column];
              if (current == '.') {
                ++new_cells;
              } else if (current == text[offset]) {
                ++matching;
              } else {
                compatible = false;
                break;
              }
            }
            if (!compatible) continue;

            long long quality = 1000000LL * matching - 10000LL * new_cells;
            if (matching == 0) {
              quality -= 100LL * line_filled;
            } else {
              quality += 10LL * line_filled;
            }

            if (quality > best_quality) {
              best_quality = quality;
              best_direction = direction;
              best_line = line;
              best_start = start;
              equal_candidates = 1;
            } else if (quality == best_quality) {
              ++equal_candidates;
              if (random.next_int(0, equal_candidates) == 0) {
                best_direction = direction;
                best_line = line;
                best_start = start;
              }
            }
          }
        }
      }

      if (best_direction == -1) continue;
      for (int offset = 0; offset < static_cast<int>(text.size()); ++offset) {
        const int row = best_direction == 0
                            ? best_line
                            : (best_start + offset) % n;
        const int column = best_direction == 0
                               ? (best_start + offset) % n
                               : best_line;
        board[row][column] = text[offset];
      }
    }

    const pair<int, int> score = count_score(board);
    if (score > best_score) {
      best_score = score;
      best_board = move(board);
    }
    ++attempt;
  } while (timer.elapsed_ms() < GREEDY_END_MS);

  GenomeProblem problem(n, m, targets);
  auto initial = problem.make_state(best_board);
  const double initial_score = problem.score(initial);
  const double remaining_ms = SEARCH_END_MS - timer.elapsed_ms();
  if (remaining_ms > 0) {
    TimeBasedAnnealingRunner<GenomeProblem> runner(
        problem, std::move(initial), initial_score, remaining_ms, 6.0, 0.03,
        input_hash, 32);
    runner.run();
    best_board = runner.best_state().board;
    cerr << "iterations=" << runner.iterations()
         << " covered=" << runner.best_state().coverage.covered_weight << "/" << m << '\n';
  }
  for (const string& row : best_board) cout << row << '\n';
}
#endif
