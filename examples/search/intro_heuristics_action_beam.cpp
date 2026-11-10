// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

// Introduction to Heuristics Contest A: AtCoder Contest Scheduling
// https://atcoder.jp/contests/intro-heuristics/tasks/intro_heuristics_a
//
// 「問題ごとに書く部分」と「汎用ビームサーチ」を分けた完全な解答例。
// リポジトリ内ではheaderをincludeする。提出時はheader全文をこの位置へ貼る。

#include <array>
#include <iostream>
#include <numeric>
#include <vector>

#include "../../library/action-beam-search.hpp"

// ========= ここだけ問題に合わせて書く =========

struct ContestSchedulingProblem {
  // Stateは「ある日まで決めた予定」を1個表す。
  // Actionを適用するために変化する情報だけを入れる。
  // decayやsatisfactionのような全候補で共通の入力はProblem本体へ置く。
  struct State {
    // 次に決める日の0-index。day日ぶんの予定は決定済み。
    int day = 0;
    // コンテスト種類ごとに、最後に開催した日を1-indexで保存する。
    std::array<int, 26> last_day{};
    // day日目までの問題文どおりの得点。出力解の得点計算にも使える。
    long long score = 0;
    // 復元用の操作履歴。answer[d]はd日目に選んだコンテストの0-index。
    std::array<unsigned char, 365> answer{};
  };

  // Actionは「次の日にどのコンテストを開くか」だけなのでintで十分。
  // 全候補ぶん一時保存されるため、盤面やState全体を入れない。
  using Action = int;

  // Scoreは候補を残す順番の比較に使う型。既定では大きいほど良い。
  // この例では本来の得点に将来の見込みを足した順位値をlong longで持つ。
  using Score = long long;

  // ここからは全Stateで共通の入力。Stateへ入れないので、幅500でもコピーされない。
  int days = 0;
  std::array<int, 26> decay{};
  std::vector<std::array<int, 26>> satisfaction;
  std::array<int, 26> contests{};

  bool read_input() {
    if (!(std::cin >> days)) return false;
    for (int& value : decay) std::cin >> value;
    satisfaction.resize(days);
    for (auto& row : satisfaction) {
      for (int& value : row) std::cin >> value;
    }
    std::iota(contests.begin(), contests.end(), 0);
    return true;
  }

  // まだ1日も決めていないStateを返す。
  State initial_state() const { return State{}; }

  // initial_state()の順位値を返す。Runnerの第3引数に渡す。
  Score initial_score() const { return 0; }

  // Actionを何回適用すれば完成か。この問題では1日につき1回。
  int max_turns() const { return days; }

  // 現在のStateから次に試すActionを全て返す。
  // この問題では毎日26種類を全て試す。合法手がStateごとに違う問題なら、
  // stateを見てvector<Action>を作る。空のコンテナを返すとその枝は行き止まりになる。
  // contestsはProblemが探索中ずっと保持するため、const参照で返しても安全。
  const std::array<int, 26>& generate_actions(const State&) const {
    return contests;
  }

  // contestを選んだ「後」の子Stateの順位値を返す。
  // 返すのは得点差分ではなく順位値そのもの。大きい候補から残される。
  // この関数は全候補に呼ばれるのでstateを変更せず、Stateをコピーせずに計算する。
  // 順位値は候補選抜専用なので、問題文の最終得点と完全に同じでなくてもよい。
  Score evaluate_action(const State& state, const Action& contest) const {
    const int next_day = state.day + 1;
    Score next_score = state.score + satisfaction[state.day][contest];
    for (int type = 0; type < 26; ++type) {
      const int next_last = type == contest ? next_day
                                             : state.last_day[type];
      next_score -= static_cast<long long>(decay[type]) *
                    (next_day - next_last);
    }

    const int remaining = days - next_day;
    Score rank_score = next_score;
    for (int type = 0; type < 26; ++type) {
      const int next_last = type == contest ? next_day
                                             : state.last_day[type];
      rank_score += static_cast<long long>(decay[type]) *
                    next_last * remaining;
    }
    return rank_score;
  }

  // 選抜を通ったcontestをStateへ本当に適用する。
  // stateはライブラリが親からコピー済みなので、直接変更してよい。
  // evaluate_actionと同じ1手後になるよう、盤面相当の情報、得点、ターン、
  // hashや使用回数がある問題ならそれら全て、そして答えの履歴を更新する。
  // Actionも候補ごとのコピーなので変更可能だが、この例では変更しない。
  void apply_action(State& state, Action& contest) const {
    state.answer[state.day] = static_cast<unsigned char>(contest);
    state.last_day[contest] = state.day + 1;
    state.score += satisfaction[state.day][contest];
    for (int type = 0; type < 26; ++type) {
      state.score -= static_cast<long long>(decay[type]) *
                     (state.day + 1 - state.last_day[type]);
    }
    ++state.day;
  }

  // 探索完了後のStateから、問題文が要求する形式だけを出力する。
  void print_answer(const State& answer) const {
    for (int day = 0; day < days; ++day) {
      std::cout << static_cast<int>(answer.answer[day]) + 1 << '\n';
    }
  }
};

// ========= 問題ごとに書く部分はここまで =========

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  ContestSchedulingProblem problem;
  if (!problem.read_input()) return 0;

  constexpr int BEAM_WIDTH = 500;
  // 引数は順に Problem、初期State、初期順位値、幅。
  // 小さいScoreを良いものとして残す問題では、最後にfalseを追加する。
  ActionBeamRunner<ContestSchedulingProblem> beam(
      problem,
      problem.initial_state(),
      problem.initial_score(),
      BEAM_WIDTH);
  // 最大max_turns世代進める。全枝が行き止まりなら、その時点で停止する。
  beam.run(problem.max_turns());
  // best()は最終ビームの中で順位値が最大のState。
  problem.print_answer(beam.best());
}
