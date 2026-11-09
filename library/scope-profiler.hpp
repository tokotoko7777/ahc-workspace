#include <chrono>
#include <cstdint>
#include <ostream>
// Pre-contest public source (created with generative AI):
// https://github.com/tokotoko7777/ahc-precontest-kit/blob/main/library/scope-profiler.hpp
// 着想: https://github.com/asi1024/MarathonLibrary/blob/13fe8241ac04fcdcfb8da6e84b050bd4b23ad3a9/snippets/profiler.h

// 処理別の回数と合計時間を測る。-DAHC_ENABLE_PROFILING を付けた時だけ有効。
// TODO: ScopeProfiler evaluation("evaluate"); のように処理名を付ける。
// TODO: 測りたいブロックの先頭へ auto guard = evaluation.measure(); と書く。
// TODO: 終了時に evaluation.report(cerr); を呼ぶ。stdoutへは出さない。
// 無効ビルドでは時計を読まない・記録しない・出力しない。最適化時は空の処理になる。
// 機種別のCPU周波数は不要。ネストした区間は内側の時間も含む（exclusive時間ではない）。
// 同じProfilerを複数threadから使わない。ProfilerはGuardより長生きさせる。
struct ScopeProfiler {
#ifdef AHC_ENABLE_PROFILING
  const char* name; // 文字列リテラルなど、このProfilerより長生きする名前を渡す。
  std::uint64_t count = 0;
  double total_ms = 0.0;
  explicit ScopeProfiler(const char* label) : name(label) {}
#else
  explicit ScopeProfiler(const char*) {}
#endif

  struct Guard {
#ifdef AHC_ENABLE_PROFILING
    ScopeProfiler& owner;
    std::chrono::steady_clock::time_point started;
    explicit Guard(ScopeProfiler& profiler)
        : owner(profiler), started(std::chrono::steady_clock::now()) {}
#else
    explicit Guard(ScopeProfiler&) {}
#endif
    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
    ~Guard() {
#ifdef AHC_ENABLE_PROFILING
      owner.total_ms += std::chrono::duration<double, std::milli>(
          std::chrono::steady_clock::now() - started).count();
      ++owner.count;
#endif
    }
  };

  Guard measure() { return Guard(*this); }
  std::uint64_t calls() const {
#ifdef AHC_ENABLE_PROFILING
    return count;
#else
    return 0;
#endif
  }
  double elapsed_ms() const {
#ifdef AHC_ENABLE_PROFILING
    return total_ms;
#else
    return 0.0;
#endif
  }
  void report(std::ostream& output) const {
#ifdef AHC_ENABLE_PROFILING
    output << name << ": calls=" << count << " total_ms=" << total_ms << '\n';
#else
    (void)output;
#endif
  }
};
