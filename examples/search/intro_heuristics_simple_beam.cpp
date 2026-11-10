// GCC提出用の最適化。診断時は -DAHC_DISABLE_GCC_OPTIMIZE で無効化。
#if defined(__GNUC__) && !defined(__clang__) && !defined(AHC_DISABLE_GCC_OPTIMIZE)
#pragma GCC optimize("O3")
#endif

// Introduction to Heuristics Contest A: AtCoder Contest Scheduling
// https://atcoder.jp/contests/intro-heuristics/tasks/intro_heuristics_a
//
// D日の日程を先頭から作る、SimpleBeamSearch単体の完全な解答例。
// リポジトリ内ではheaderをincludeする。提出時はheader全文をこの位置へ貼る。

#include <array>
#include <iostream>
#include <vector>

#include "../../library/simple-beam-search.hpp"

struct ScheduleState {
  int day = 0;
  std::array<int, 26> last_day{};
  long long score = 0;
  std::array<unsigned char, 365> answer{};
};

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int days;
  if (!(std::cin >> days)) return 0;

  std::array<int, 26> decay{};
  for (int& value : decay) std::cin >> value;

  std::vector<std::array<int, 26>> satisfaction(days);
  for (auto& row : satisfaction) {
    for (int& value : row) std::cin >> value;
  }

  constexpr int BEAM_WIDTH = 500;
  SimpleBeamSearch<ScheduleState, long long> beam(
      ScheduleState{}, BEAM_WIDTH);
  beam.reserve_candidates(BEAM_WIDTH * 26);
  std::vector<ScheduleState> child_buffer;
  child_buffer.reserve(26);

  for (int day = 0; day < days; ++day) {
    const bool advanced = beam.step(
        [&](const ScheduleState& state) -> std::vector<ScheduleState>& {
          child_buffer.clear();

          for (int contest = 0; contest < 26; ++contest) {
            ScheduleState next = state;
            next.answer[day] = static_cast<unsigned char>(contest);
            next.last_day[contest] = day + 1;
            next.score += satisfaction[day][contest];
            for (int type = 0; type < 26; ++type) {
              next.score -=
                  static_cast<long long>(decay[type]) *
                  (day + 1 - next.last_day[type]);
            }
            next.day = day + 1;
            child_buffer.push_back(std::move(next));
          }
          return child_buffer;
        },
        [&](const ScheduleState& state) {
          // 最近開催した種類ほど、今後の満足度低下も小さくなる。
          // 最終日ではremaining=0なので、本来のscoreそのものになる。
          const int remaining = days - state.day;
          long long rank_score = state.score;
          for (int type = 0; type < 26; ++type) {
            rank_score +=
                static_cast<long long>(decay[type]) *
                state.last_day[type] * remaining;
          }
          return rank_score;
        });
    if (!advanced) break;
  }

  const ScheduleState& answer = beam.best();
  for (int day = 0; day < days; ++day) {
    std::cout << static_cast<int>(answer.answer[day]) + 1 << '\n';
  }
}
