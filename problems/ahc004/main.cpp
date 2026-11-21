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

#ifdef AHC_ANCHOR_GEOMETRY
#include "anchor-congruence.hpp"
#ifdef AHC004_WILD_GEOMETRY
#include "wild-geometry.hpp"
#ifdef AHC004_MEDIUM_DIVERSE
#include "medium-diverse.hpp"
#endif
#endif
#ifdef AHC004_MERGED_CYCLES
#include "partial-lines.hpp"
#endif
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
#ifdef AHC004_SOURCE_HOTPATH
    char packed_board[400]{};
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
#ifdef AHC004_SOURCE_HOTPATH
  struct FlatNode { int next[8],first,last; };
  struct FlatOutput { int id,length; };
  vector<FlatNode> flat_nodes;
  vector<FlatOutput> flat_output;
  char trial_board[400]{};
  void scan_flat(const char* board,int line,vector<int>& hits)const{
    hits.clear();
    const auto* nodes=flat_nodes.data();
    const auto* output=flat_output.data();
    const int step=line<n?1:n;
    const char* first=board+(line<n?line*n:line-n);
    const int stop=n+max_length-1;
    int state=0,k=0;
    for(int p=0;p<stop;++p){
      const char c=first[k*step];
      if(++k==n)k=0;
      if(c=='.'){state=0;continue;}
      state=nodes[state].next[c-'A'];
      for(int t=nodes[state].first;t<nodes[state].last;++t)
        if(p+1-output[t].length<n)hits.push_back(output[t].id);
    }
  }
#endif

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
#ifdef AHC004_SOURCE_HOTPATH
    assert(n<=20);
    flat_nodes.reserve(matcher.nodes.size());
    for(const auto& node:matcher.nodes){
      FlatNode flat{};
      copy(node.next.begin(),node.next.end(),flat.next);
      flat.first=int(flat_output.size());
      for(int id:node.output)flat_output.push_back({id,int(targets[id].text.size())});
      flat.last=int(flat_output.size());
      flat_nodes.push_back(flat);
    }
#endif
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
#ifdef AHC004_SOURCE_HOTPATH
    for(int r=0;r<n;++r)memcpy(state.packed_board+r*n,state.board[r].data(),n);
#endif
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
#if defined(AHC004_SOURCE_HOTPATH) && defined(__SIZEOF_INT128__) && defined(__GLIBCXX__)
      // The same full-width Lemire reduction used by current libstdc++ for
      // mt19937_64. Keep the standard distribution on other implementations.
      const uint64_t bound=upper;
      for(;;){
        const __uint128_t product=__uint128_t(rng())*bound;
        const uint64_t low=uint64_t(product);
        if(low>=bound || low>=uint64_t(-bound)%bound)return int(product>>64);
      }
#else
      return uniform_int_distribution<int>(0, upper-1)(rng);
#endif
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
#ifdef AHC004_SOURCE_HOTPATH
    int line_quality[40],line_start[40],line_ties[40]{};
    if(rerank)for(int i=0;i<2*n;++i)line_quality[i]=INT_MIN;
#else
    array<int,40> line_quality, line_start, line_ties{};
    if (rerank) line_quality.fill(INT_MIN);
#endif
#endif
#ifdef AHC004_FAST_PLACEMENT
    const int length = int(text.size());
    const uint64_t low_bits = ((1ULL << (3*length))-1)/7;
#ifdef AHC004_SOURCE_HOTPATH
    const auto* code_lanes=state.rotated_code.data();
    const auto* empty_lanes=state.rotated_empty.data();
    for(int line=0;line<2*n;++line){
     const uint64_t* codes=code_lanes[line].data(),*empties=empty_lanes[line].data();
     for(int start=0;start<n;++start){
      const uint64_t empty=empties[start]&low_bits;
      const uint64_t diff=codes[start]^word;
#else
    for (int line = 0; line < 2*n; ++line) for (int start = 0; start < n; ++start) {
      const uint64_t empty = state.rotated_empty[line][start] & low_bits;
      const uint64_t diff = state.rotated_code[line][start] ^ word;
#endif
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
#if defined(AHC004_FAST_PLACEMENT) && defined(AHC004_SOURCE_HOTPATH)
    }
#endif
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
#ifdef AHC004_SOURCE_HOTPATH
        int line=0;
        for(int i=1;i<2*n;++i)if(line_quality[i]>line_quality[line])line=i;
#else
        const int line = max_element(line_quality.begin(), line_quality.begin()+2*n)
                         - line_quality.begin();
#endif
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
#ifdef AHC004_SOURCE_HOTPATH
    memcpy(trial_board,state.packed_board,n*n);
#else
    scratch = state.board;
#endif
    next_empty = state.empty;
    uint64_t line_mask = 0;
    for (auto [cell, c] : move.changes) {
#ifdef AHC004_SOURCE_HOTPATH
      next_empty+=(c=='.')-(trial_board[cell]=='.');
      trial_board[cell]=c;
#else
      next_empty += (c == '.') - (scratch[cell/n][cell%n] == '.');
      scratch[cell/n][cell%n] = c;
#endif
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
#ifdef AHC004_SOURCE_HOTPATH
      scan_flat(trial_board,line,next_hits[line]);
#else
      scan_line(scratch, line, next_hits[line]);
#endif
      for (int id : next_hits[line]) coverage.add_delta(id, 1);
    }
    next_covered = state.coverage.covered_weight + coverage.score_delta(state.coverage);
    return score(next_covered, next_empty) - score(state);
  }

#if (defined(AHC004_LINE_BROAD) || defined(AHC004_LINE_BROAD_SA_CONTROL)) && !defined(AHC004_LINE_REBUILD)
#error "A post-rebuild tail requires AHC004_LINE_REBUILD"
#endif
#ifdef AHC004_LINE_REBUILD
  struct RebuildStats { int lines=0, checked=0, accepted=0; };
  // The orthogonal estimate may count the same newly available target
  // in two perpendicular lines. It only ranks candidates, never accepts.
  vector<array<int,9>> orthogonal_rewards(const State& state,int line,State& blank) const {
    Board board=state.board;
    for(int k=0;k<n;++k){const int p=line_cells[line][k];board[p/n][p%n]='.';}
    blank=make_state(board);
    vector<array<int,9>> rewards(n);
    vector<int> seen(targets.size(),-1),hits;
    int visit=0;
    for(int k=0;k<n;++k){
      const int p=line_cells[line][k],cross=line<n?n+p%n:p/n;
      for(int ch=0;ch<8;++ch){
        ++visit;board[p/n][p%n]=char('A'+ch);
        scan_line(board,cross,hits);
        for(int id:hits)if(!blank.coverage.counts[id] && seen[id]!=visit){
          rewards[k][ch]+=targets[id].frequency;seen[id]=visit;
        }
      }
      board[p/n][p%n]='.';
    }
    return rewards;
  }
  bool rebuild_line(State& current,int line,const Timer& timer,double deadline,RebuildStats& stats,
                    int broad=0){
    if(timer.elapsed_ms()>=deadline)return false;
    ++stats.lines;
    assert(n<=20);
    State blank;
    const auto rewards=orthogonal_rewards(current,line,blank);
    string fill(n,'.');
    array<int,20> upper{};
    for(int k=0;k<n;++k){
      const int p=line_cells[line][k];
      const char old=current.board[p/n][p%n];
      int best=old=='.'?8:old-'A';
      for(int c=0;c<9;++c)if(rewards[k][c]>rewards[k][best])best=c;
      upper[k]=rewards[k][best];fill[k]=best==8?'.':char('A'+best);
    }
    struct Frag{int id,value,loss,agreement;uint64_t bits,mask;};
    vector<Frag> pool;
    // Diversified passes emphasize packet weight, then orthogonal support.
    const int word_price=broad==1?48:broad==2?24:32;
    const int orth_price=32;
    const int agree_price=broad==1?-1:1;
    auto merit=[&](const Frag& a){return word_price*a.value-orth_price*a.loss+agree_price*a.agreement;};
    for(int id=0;id<int(targets.size());++id)if(!blank.coverage.counts[id]){
      if((id&63)==0 && timer.elapsed_ms()>=deadline)break;
      vector<int> contained;
      int node=0,weight=0;
      for(char c:targets[id].text){
        node=matcher.advance(node,c-'A');
        for(int match:matcher.matches(node))if(!blank.coverage.counts[match])contained.push_back(match);
      }
      ranges::sort(contained);contained.erase(unique(contained.begin(),contained.end()),contained.end());
      for(int x:contained)weight+=targets[x].frequency;
      vector<Frag> placements;
      for(int start=0;start<n;++start){
        uint64_t bits=0,mask=0;
        int loss=0,agreement=0;
        for(int j=0;j<int(targets[id].text.size());++j){
          int k=(start+j)%n,c=targets[id].text[j]-'A';
          bits|=uint64_t(c)<<(3*k);mask|=7ULL<<(3*k);
          loss+=upper[k]-rewards[k][c];
          agreement+=fill[k]==targets[id].text[j];
        }
        placements.push_back({id,weight,loss,agreement,bits,mask});
      }
      stable_sort(placements.begin(),placements.end(),[&](const Frag& a,const Frag& b){return merit(a)>merit(b);});
      pool.insert(pool.end(),placements.begin(),placements.begin()+min(broad?8:4,n));
    }
    stable_sort(pool.begin(),pool.end(),[&](const Frag& a,const Frag& b){return merit(a)>merit(b);});
    const size_t pool_limit=broad?224:96,beam_limit=broad?48:24;
    if(pool.size()>pool_limit)pool.resize(pool_limit);
    struct Beam{uint64_t bits=0,mask=0;array<int,3> ids{-1,-1,-1};int reward=0,loss=0,agreement=0;};
    vector<Beam> beam(1);
    Move best_move;
    double best=1e-10;
    auto evaluate=[&](const Beam& s){
      if(timer.elapsed_ms()>=deadline)return;
      Move move;
      for(int k=0;k<n;++k){
        const int p=line_cells[line][k];
        const char c=(s.mask>>(3*k)&7)?char('A'+(s.bits>>(3*k)&7)):fill[k];
        if(c!=current.board[p/n][p%n])move.changes.emplace_back(p,c);
      }
      if(move.changes.empty())return;
      ++stats.checked;
      const auto change=evaluate_move(current,move,-INFINITY);
      if(change && *change>best){best=*change;best_move=std::move(move);}
    };
    evaluate(beam[0]);
    for(int depth=0;depth<3 && timer.elapsed_ms()<deadline;++depth){
      vector<Beam> next;
      for(const auto& b:beam)for(const auto& f:pool){
        if(find(b.ids.begin(),b.ids.end(),f.id)!=b.ids.end() ||
           ((b.bits^f.bits)&b.mask&f.mask) || (f.mask&~b.mask)==0)continue;
        Beam v=b;
        v.bits|=f.bits;v.mask|=f.mask;v.ids[depth]=f.id;v.reward+=f.value;
        // Charge each cell once even when two compatible fragments overlap.
        uint64_t new_letters=f.mask&~b.mask;
        while(new_letters){
          const int k=countr_zero(new_letters)/3,c=f.bits>>(3*k)&7;
          new_letters&=~(7ULL<<(3*k));
          v.loss+=upper[k]-rewards[k][c];
          v.agreement+=fill[k]==char('A'+c);
        }
        next.push_back(v);
      }
      auto beam_merit=[&](const Beam& v){return word_price*v.reward-orth_price*v.loss+agree_price*v.agreement;};
      stable_sort(next.begin(),next.end(),[&](const Beam& a,const Beam& b){
        return beam_merit(a)>beam_merit(b);
      });
      beam.clear();
      for(const auto& v:next){
        if(any_of(beam.begin(),beam.end(),[&](const Beam& b){return b.mask==v.mask && b.bits==v.bits;}))continue;
        beam.push_back(v);evaluate(v);
        if(beam.size()==beam_limit || timer.elapsed_ms()>=deadline)break;
      }
      if(beam.empty())break;
    }
    if(best_move.changes.empty())return false;
    auto exact=evaluate_move(current,best_move,-INFINITY);
    assert(exact && *exact>0);
    apply_move(current,best_move);++stats.accepted;
    return true;
  }
#endif
  // TODO: 採用した手だけ反映する。直前のevaluate_moveのcacheを使う。
  // 不採用なら呼ばれないので、rollbackは不要。
  void apply_move(State& state, Move& move) {
    coverage.apply_delta(state.coverage);
#ifdef AHC004_SOURCE_HOTPATH
    memcpy(state.packed_board,trial_board,n*n);
    for(auto [cell,c]:move.changes)state.board[cell/n][cell%n]=c;
#else
    state.board.swap(scratch);
#endif
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

  const auto fill_words = [&](Board& board, int attempt) {
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

  };

#ifdef AHC004_LINE_REBUILD
  const char* checkpoint_in=getenv("AHC004_LINE_CHECKPOINT_IN");
  const char* checkpoint_out=getenv("AHC004_LINE_CHECKPOINT_OUT");
  if(checkpoint_in && checkpoint_out)return 2;
  double checkpoint_spare=0;
  vector<int> checkpoint_lines;
  if(checkpoint_in){
    ifstream file(checkpoint_in);
    string magic;
    uint64_t hash=0;
    int dimension=0;
    if(!(file>>magic>>hash>>dimension>>checkpoint_spare) || magic!="AHC004_LINE_V1" ||
       hash!=input_hash || dimension!=n || !isfinite(checkpoint_spare) ||
       checkpoint_spare<0 || checkpoint_spare>300.0)return 2;
    for(string& row:best_board){
      if(!(file>>row) || int(row.size())!=n ||
         row.find_first_not_of(".ABCDEFGH")!=string::npos)return 2;
    }
    checkpoint_lines.resize(2*n);
    for(int& v:checkpoint_lines)if(!(file>>v))return 2;
    auto sorted=checkpoint_lines;ranges::sort(sorted);
    for(int i=0;i<2*n;++i)if(sorted[i]!=i)return 2;
    if(!(file>>random.engine))return 2;
  }
  if(!checkpoint_in)
#endif
  do {
    Board board(n, string(n, '.'));
#ifdef AHC_CONTIG_SEED
    if(attempt%3==1)board=seed_from_contigs(n,targets,maximal_indices,random,4+(attempt/3)%3);
#endif
    fill_words(board, attempt);
    const pair<int, int> score = count_score(board);
    if (score > best_score) {
      best_score = score;
      best_board = move(board);
    }
    ++attempt;
  } while (timer.elapsed_ms() < GREEDY_END_MS);

#ifdef AHC_ANCHOR_GEOMETRY
#ifdef AHC004_LINE_REBUILD
  if (!checkpoint_in) {
#endif
  // First finish and score the original starts above. Geometry then spends
  // part of the existing wall-clock allowance, never replacing a better seed.
  const auto ordinary_key = best_score;
  constexpr double GEOMETRY_END_MS = 1000.0;
  const auto past_geometry = [&] { return timer.elapsed_ms() >= GEOMETRY_END_MS; };
  vector<string> words;
  vector<int> weights;
  for (const auto& target : targets) {
    words.push_back(target.text);
    weights.push_back(target.frequency);
  }
  const double extraction_end = timer.elapsed_ms() + 12.0;
  auto closed = ahc004::observed_cycles(n, words, weights, 28000,
      [&] { return past_geometry() || timer.elapsed_ms() >= extraction_end; });
  // Beyond 42 cycles, do not let false positives make the distinct matching
  // lower bound impossible. The evidence ranking already prefers read support.
  if (closed.cycles.size() > 40) closed.cycles.resize(40);
  int sweep_visits=0, fourth_visits=0, realized=0, kept_rows=0, kept_cols=0;
  bool sweep_cap=false, fourth_cap=false;
  pair<int,int> raw{-1,-1}, filled_key{-1,-1};
  int supported_reads=0;
  if (closed.cycles.size() >= 28) {
    vector<bool> seen(target_count);
    for (const auto& c : closed.cycles) for (int id : c.words) seen[id]=true;
    for (int id=0; id<target_count; ++id) if (seen[id]) supported_reads+=weights[id];
  }
#ifdef AHC004_MERGED_CYCLES
  int merged_new_lines=0;
  const int orig_cycle_count=closed.cycles.size(), orig_supported=supported_reads;
  int longest_word=0;
  for(const auto& w:words)longest_word=max(longest_word,int(w.size()));
  if ((orig_cycle_count < 28 || 4*orig_supported < 3*m) &&
      longest_word >= 9 && !past_geometry()) {
    const double merge_end=min(GEOMETRY_END_MS-60.0,timer.elapsed_ms()+18.0);
    const auto past_merge=[&] {return timer.elapsed_ms()>=merge_end;};
    const auto partial=ahc004::partial_assembled_lines(n,words,5,past_merge);
    std::set<std::string> unique_lines;
    for(const auto& c:closed.cycles) unique_lines.insert(c.letters);
    auto candidates=closed.cycles;
    for(const auto& line:partial) {
      if(past_merge())break;
      if(line.known!=n)break;
      if(!unique_lines.insert(line.letters).second)continue;
      ahc004::Cycle cycle{line.letters,{},0};
      const std::string wrapped=cycle.letters+cycle.letters.substr(0,n-1);
      for(int i=0;i<int(words.size());++i) {
        if(wrapped.find(words[i])>=size_t(n))continue;
        cycle.words.push_back(i);
        const int len=words[i].size();
        cycle.evidence+=(len-1)*(len-1)*min(3,weights[i]);
      }
      candidates.push_back(std::move(cycle));
      ++merged_new_lines;
    }
    if(!past_merge() && candidates.size()>=28) {
      std::sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){
        return a.evidence!=b.evidence?a.evidence>b.evidence:a.letters<b.letters;
      });
      if(candidates.size()>40)candidates.resize(40);
      std::vector<bool> seen(target_count);
      for(const auto& c:candidates)for(int id:c.words)seen[id]=true;
      int after=0;
      for(int i=0;i<target_count;++i)if(seen[i])after+=weights[i];
      if(4*after>=3*m) {
        closed.cycles=std::move(candidates);
        supported_reads=after;
      }
    }
  }
#endif
#ifdef AHC004_WILD_GEOMETRY
  bool full_geometry_attempted=false;
#endif
  if (closed.cycles.size() >= 28 && 4*supported_reads >= 3*m && !past_geometry()) {
#ifdef AHC004_WILD_GEOMETRY
    full_geometry_attempted=true;
#endif
    const double triple_end = min(GEOMETRY_END_MS-50.0, timer.elapsed_ms()+175.0);
    auto anchors = ahc004::three_parallel_anchors(closed.cycles, 1368000,
         [&] {return timer.elapsed_ms() >= triple_end;}, 768);
    const double four_end = GEOMETRY_END_MS-60.0;
    auto fourth = ahc004::fourth_row_extensions(closed.cycles, anchors.layouts,
         15000000, [&] {return timer.elapsed_ms() >= four_end;});
    sweep_visits=anchors.examined;
    fourth_visits=fourth.examined;
    sweep_cap=anchors.capped;
    fourth_cap=fourth.capped;
    for (const auto& f : fourth.ranked) {
      if (realized>=4 || past_geometry()) break;
      auto geometry=ahc004::realize_anchor(closed.cycles,anchors.layouts[f.parent],f,past_geometry);
      if (geometry.board.empty()) continue;
      ++realized;
      raw=max(raw,count_score(geometry.board));
      if(past_geometry()) break;
      fill_words(geometry.board, attempt+realized);
      auto key=count_score(geometry.board);
      if(key>filled_key) {
        filled_key=key;
        kept_rows=geometry.rows;
        kept_cols=geometry.columns;
      }
      if(key>best_score) {
        best_score=key;
        best_board=move(geometry.board);
      }
    }
  }
#ifdef AHC004_ANCHOR_RESCUE
  // Existing complete-cycle initialization always goes first. A failed pass
  // can be caused by an orientation split among false-positive top-ranked
  // anchors; retry with a disjoint high-evidence quintet rather than widen
  // compatibility or allow a worse seed.
  int rescue_sweep=0, rescue_fourth=0, rescue_realized=0, rescue_unique=0;
  // The additive arm leaves the merged-bank solve unchanged and never
  // applies alternate-anchor rescue when the original bank failed its gate.
  if (sweep_visits && best_score==ordinary_key
#ifdef AHC004_MERGED_CYCLES
      && orig_cycle_count>=28 && 4*orig_supported>=3*m
#endif
  ) {
    constexpr double rescue_end=1550.0;
    const auto deadline=[&] { return timer.elapsed_ms()>=rescue_end; };
    const double three_stop=min(rescue_end-260.0,timer.elapsed_ms()+175.0);
    auto retry=ahc004::three_parallel_anchors(closed.cycles,1368000,
        [&] {return timer.elapsed_ms()>=three_stop;},640,{5,6,7,8,9});
    auto refine=ahc004::fourth_row_extensions(closed.cycles,retry.layouts,8500000,
        [&] { return timer.elapsed_ms()>=rescue_end-150.0; },true);
    rescue_sweep=retry.examined;
    rescue_fourth=refine.examined;
    vector<pair<pair<int,int>,Board>> ranked_boards;
    vector<string> seen_keys;
    for(const auto& f:refine.ranked) {
      if(rescue_realized>=32 || timer.elapsed_ms()>=rescue_end-90.0)break;
      auto geo=ahc004::realize_anchor(closed.cycles,retry.layouts[f.parent],f,deadline);
      if(geo.board.empty())continue;
      ++rescue_realized;
      auto key=ahc004::geometry_shape_key(geo.board);
      if(find(seen_keys.begin(),seen_keys.end(),key)!=seen_keys.end())continue;
      seen_keys.push_back(move(key));
      ranked_boards.emplace_back(count_score(geo.board),move(geo.board));
    }
    rescue_unique=ranked_boards.size();
    stable_sort(ranked_boards.begin(),ranked_boards.end(),[](const auto& a,const auto& b) {
      return a.first>b.first;
    });
    const auto rng_before_rescue=random.engine;
    for(int i=0;i<min<int>(4,ranked_boards.size()) && !deadline();++i){
      auto& b=ranked_boards[i].second;
      fill_words(b,attempt+realized+i+1);
      auto key=count_score(b);
      if(key>best_score) {
        best_score=key;
        best_board=move(b);
      }
      if(best_score.first==m)break;
    }
    if(best_score==ordinary_key) random.engine=rng_before_rescue;
  }
#endif
  cerr << "geometry_cycles=" << closed.cycles.size()
       << " geometry_extraction_capped=" << closed.capped
       << " geometry_supported=" << supported_reads << " geometry_words=" << m
       << " geometry_sweep_visits=" << sweep_visits
       << " geometry_sweep_cap=" << sweep_cap
       << " geometry_fourth_visits=" << fourth_visits
       << " geometry_fourth_cap=" << fourth_cap
       << " geometry_realized=" << realized
       << " geometry_rows=" << kept_rows << " geometry_cols=" << kept_cols
       << " geometry_regular=" << ordinary_key.first << " geometry_regular_empty=" << ordinary_key.second
       << " geometry_raw=" << raw.first << " geometry_raw_empty=" << raw.second
       << " geometry_filled=" << filled_key.first << " geometry_filled_empty=" << filled_key.second
       << " geometry_selected=" << (best_score>ordinary_key)
       << " geometry_ms=" << timer.elapsed_ms() << '\n';
#ifdef AHC004_ANCHOR_RESCUE
  if(rescue_sweep) {
    cerr<<"geometry_rescue_sweep="<<rescue_sweep<<" geometry_rescue_fourth="<<rescue_fourth
        <<" geometry_rescue_realized="<<rescue_realized<<" geometry_rescue_unique="<<rescue_unique
        <<" geometry_rescue_selected="<<(best_score>ordinary_key)<<'\n';
  }
#endif
#ifdef AHC004_MERGED_CYCLES
  cerr << "partial_original=" << orig_cycle_count
       << " partial_original_support=" << orig_supported
       << " partial_novel=" << merged_new_lines << '\n';
#endif

#ifdef AHC004_WILD_GEOMETRY
  int wild_longest=0;
  for(const auto& w:words)wild_longest=max(wild_longest,int(w.size()));
  int wild_lines=0,wild_tri=0,wild_parents=0,wild_4=0,wild_real=0,wild_reject=0;
  int wild_extensions=0,wild_raw=0,wild_filled=0;
  bool wild_selected=false;
  const double wild_begin=timer.elapsed_ms();
  if (!full_geometry_attempted && n==20 && (wild_longest==9 || (wild_longest==8 && m>=680))) {
    const double assembly_end=wild_begin+18.0;
    auto bank=ahc004::make_wild_bank(ahc004::partial_assembled_lines(n,words,4,
                      [&]{return timer.elapsed_ms()>=assembly_end;}),12);
    wild_lines=bank.n;
    if (bank.n>=24) {
      const double triple_end=min(690.0,wild_begin+440.0);
      auto triples=ahc004::wild_triples(bank,192,1368000,
                  [&]{return timer.elapsed_ms()>=triple_end;});
      const double fourth_end=min(1510.0,wild_begin+1300.0);
      auto fourth=ahc004::wild_fourth(bank,triples,15000000,
                  [&]{return timer.elapsed_ms()>=fourth_end;});
      wild_tri=triples.examined;wild_parents=fourth.parents;wild_4=fourth.tested;
      const double seed_end=min(1760.0,wild_begin+1575.0);
      std::vector<std::pair<std::pair<int,int>,Board>> verified;
      std::set<std::array<int,4>> seen;
      for(const auto& anchor:fourth.ranked) {
        if(wild_real>=32 || timer.elapsed_ms()>=seed_end)break;
        if(!seen.insert(ahc004::wild_anchor_key(anchor)).second)continue;
        auto x=ahc004::wild_realize(bank,anchor,
                      [&]{return timer.elapsed_ms()>=seed_end;});
        if(x.board.empty())continue;
        ++wild_real;
        wild_extensions=max(wild_extensions,x.extensions);
        if(x.extensions<2 && anchor.strong<17){++wild_reject;continue;}
        auto raw=count_score(x.board);
        wild_raw=max(wild_raw,raw.first);
        if(raw>best_score){
          best_score=raw;best_board=x.board;wild_selected=true;
        }
        verified.emplace_back(raw,std::move(x.board));
      }
      std::stable_sort(verified.begin(),verified.end(),[](const auto& a,const auto& b){return a.first>b.first;});
      for(int i=0;i<min<int>(5,verified.size()) && timer.elapsed_ms()<1850.0;++i) {
        auto& candidate=verified[i].second;
        fill_words(candidate,attempt+i+1);
        const auto score=count_score(candidate);
        wild_filled=max(wild_filled,score.first);
        if (score>best_score) {
          best_score=score;best_board=std::move(candidate);wild_selected=true;
        }
      }
    }
  }
  cerr<<"wild_lines="<<wild_lines<<" wild_triples="<<wild_tri
      <<" wild_parents="<<wild_parents<<" wild_fourth="<<wild_4
      <<" wild_realized="<<wild_real<<" wild_rejected="<<wild_reject
      <<" wild_extensions="<<wild_extensions<<" wild_raw="<<wild_raw
      <<" wild_filled="<<wild_filled<<" wild_selected="<<wild_selected
      <<" wild_ms="<<timer.elapsed_ms()-wild_begin<<'\n';
#endif
#if defined(AHC004_WILD_GEOMETRY) && defined(AHC004_MEDIUM_DIVERSE)
  int medium_tri=0,medium_parents=0,medium_realized=0,medium_rejected=0;
  int medium_raw=0,medium_filled=0;
  bool medium_selected=false;
  const double medium_begin=timer.elapsed_ms();
  // This experimental path targets the gap below the old L6 M>=680 gate.
  // Do not alter any path whose complete or primary wildcard geometry ran.
  if (!full_geometry_attempted && !wild_tri && !wild_selected
      && n==20 && wild_longest==8 && 600<=m && m<680 && medium_begin<500.0) {
    const auto old_rng=random.engine;
    const auto old_score=best_score;
    auto bank=ahc004::make_wild_bank(ahc004::partial_assembled_lines(n,words,4,
                             [&]{return timer.elapsed_ms()-medium_begin>=18.0;}),12);
    if(bank.n>=24) {
      auto trio=ahc004::wild_offset_diverse(bank,384,1368000,
                      [&]{return timer.elapsed_ms()>=620.0;});
      auto fourth=ahc004::medium_fourth(bank,trio,49000000,
                      [&]{return timer.elapsed_ms()>=1390.0;});
      medium_tri=trio.examined;medium_parents=fourth.parents;
      std::vector<std::pair<std::pair<int,int>,Board>> verified;
      std::set<std::array<int,4>> seen;
      for(const auto& a:fourth.ranked) {
        if(medium_realized>=40 || timer.elapsed_ms()>=1695.0)break;
        if(!seen.insert(ahc004::wild_anchor_key(a)).second)continue;
        auto x=ahc004::wild_realize(bank,a,[&]{return timer.elapsed_ms()>=1695.0;});
        if(x.board.empty())continue;
        ++medium_realized;
        // Unlike crossing count alone, independent extra lines provide
        // corroborating intersections not involved in choosing the anchors.
        if(x.extensions<2){++medium_rejected;continue;}
        auto raw=count_score(x.board);
        medium_raw=max(medium_raw,raw.first);
        if(raw>best_score){best_score=raw;best_board=x.board;}
        verified.emplace_back(raw,std::move(x.board));
      }
      std::stable_sort(verified.begin(),verified.end(),
                        [](const auto& a,const auto& b){return a.first>b.first;});
      for(int i=0;i<min<int>(5,verified.size())&&timer.elapsed_ms()<1815.0;++i){
        auto& board=verified[i].second;
        fill_words(board,attempt+i+1);
        auto score=count_score(board);
        medium_filled=max(medium_filled,score.first);
        if(score>best_score){best_score=score;best_board=std::move(board);}
        if(best_score.first==m)break;
      }
    }
    medium_selected=best_score>old_score;
    if(!medium_selected)random.engine=old_rng;
  }
  cerr<<"medium_triples="<<medium_tri<<" medium_parents="<<medium_parents
      <<" medium_realized="<<medium_realized<<" medium_rejected="<<medium_rejected
      <<" medium_raw="<<medium_raw<<" medium_filled="<<medium_filled
      <<" medium_selected="<<medium_selected<<" medium_ms="<<timer.elapsed_ms()-medium_begin<<'\n';
#endif
#ifdef AHC004_LINE_REBUILD
  }
#endif
#endif
  GenomeProblem problem(n, m, targets);
  auto initial = problem.make_state(best_board);
  const double initial_score = problem.score(initial);
#if defined(AHC004_LINE_REBUILD) || defined(AHC004_LINE_REBUILD_SA_CONTROL)
  constexpr double ANNEAL_END_MS = 2500.0;
#else
  constexpr double ANNEAL_END_MS = SEARCH_END_MS;
#endif
  const double remaining_ms = ANNEAL_END_MS - timer.elapsed_ms();
  if (remaining_ms > 0
#ifdef AHC004_LINE_REBUILD
      && !checkpoint_in
#endif
      ) {
    TimeBasedAnnealingRunner<GenomeProblem> runner(
        problem, std::move(initial), initial_score, remaining_ms, 6.0, 0.03,
        input_hash, 32);
    runner.run();
    best_board = runner.best_state().board;
    cerr << "iterations=" << runner.iterations()
         << " covered=" << runner.best_state().coverage.covered_weight << "/" << m << '\n';
  }
#ifdef AHC004_LINE_REBUILD
  auto pol=problem.make_state(best_board);
  const auto pre=problem.score(pol);
  GenomeProblem::RebuildStats stats;
  vector<int> lines(2*n);iota(lines.begin(),lines.end(),0);
  if(checkpoint_in)lines=checkpoint_lines;
  for(int round=0;!checkpoint_in && round<4 && timer.elapsed_ms()<SEARCH_END_MS;++round){
    shuffle(lines.begin(),lines.end(),random.engine);
    int old=stats.accepted;
    for(int line:lines){
      if(timer.elapsed_ms()>=SEARCH_END_MS)break;
      problem.rebuild_line(pol,line,timer,SEARCH_END_MS,stats);
    }
    if(stats.accepted==old)break;
  }
  cerr<<"line_rebuild lines="<<stats.lines<<" exact="<<stats.checked<<" accepted="<<stats.accepted
      <<" initial="<<pre<<" final="<<problem.score(pol)<<'\n';
  const double saved_spare=max(0.,SEARCH_END_MS-timer.elapsed_ms());
  if(checkpoint_out){
    ofstream file(checkpoint_out);
    file<<"AHC004_LINE_V1 "<<input_hash<<' '<<n<<' '<<setprecision(17)<<saved_spare<<'\n';
    for(const string& row:pol.board)file<<row<<'\n';
    for(int v:lines)file<<v<<' ';
    file<<'\n'<<random.engine<<'\n';
    if(!file)return 2;
  }
#if defined(AHC004_LINE_BROAD) && defined(AHC004_LINE_BROAD_SA_CONTROL)
#error "Select a single post-rebuild tail"
#endif
#if defined(AHC004_LINE_BROAD) || defined(AHC004_LINE_BROAD_SA_CONTROL)
  const double tail_pre=problem.score(pol);
  const double tail_start=timer.elapsed_ms();
  const double tail_end=checkpoint_in?tail_start+checkpoint_spare:SEARCH_END_MS;
#ifdef AHC004_LINE_BROAD
  GenomeProblem::RebuildStats wide;
  for(int sweep=0;sweep<8 && timer.elapsed_ms()<tail_end;++sweep){
    shuffle(lines.begin(),lines.end(),random.engine);
    for(int line:lines){
      if(timer.elapsed_ms()>=tail_end)break;
      problem.rebuild_line(pol,line,timer,tail_end,wide,1+sweep%2);
    }
  }
  cerr<<"line_tail broad lines="<<wide.lines<<" exact="<<wide.checked<<" accepted="<<wide.accepted
      <<" start_ms="<<tail_start<<" end_ms="<<timer.elapsed_ms()<<" initial="<<tail_pre
      <<" final="<<problem.score(pol)<<'\n';
  best_board=std::move(pol.board);
#else
  const double spare=tail_end-tail_start;
  if(spare>0){
    TimeBasedAnnealingRunner<GenomeProblem> tail(problem,std::move(pol),tail_pre,spare,0.10,0.012,
                                                 input_hash^0xc288746dULL,32);
    tail.run();
    cerr<<"line_tail sa iterations="<<tail.iterations()<<" start_ms="<<tail_start
        <<" end_ms="<<timer.elapsed_ms()<<" initial="<<tail_pre
        <<" final="<<problem.score(tail.best_state())<<'\n';
    best_board=tail.best_state().board;
  }else{
    cerr<<"line_tail sa iterations=0 start_ms="<<tail_start<<" end_ms="<<timer.elapsed_ms()
        <<" initial="<<tail_pre<<" final="<<tail_pre<<'\n';
    best_board=std::move(pol.board);
  }
#endif
#else
  best_board=std::move(pol.board);
#endif
#elif defined(AHC004_LINE_REBUILD_SA_CONTROL)
  auto pol=problem.make_state(best_board);
  const auto pre=problem.score(pol);
  const double spare=SEARCH_END_MS-timer.elapsed_ms();
  if(spare>0){
    TimeBasedAnnealingRunner<GenomeProblem> runner(problem,std::move(pol),pre,spare,0.12,0.015,
                                                   input_hash^0x400445ddULL,32);
    runner.run();
    cerr<<"line_rebuild_control iterations="<<runner.iterations()<<" initial="<<pre
        <<" final="<<problem.score(runner.best_state())<<'\n';
    best_board=runner.best_state().board;
  }
#endif
  for (const string& row : best_board) cout << row << '\n';
}
#endif
