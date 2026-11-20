// AHC_SOURCE: examples/search/ahc005_patrol_sa.cpp
#define AHC005_TEST
#include "../../examples/search/ahc005_patrol_sa.cpp"
#include "route-splice.hpp"
#ifdef AHC005_PATH_AWARE_DIAGNOSTIC
#include "path-aware.hpp"
#endif

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
#ifdef AHC005_PATH_AWARE_DIAGNOSTIC
  if(solver.answer_is_valid(candidate) && !candidate.empty()) {
    const int common=travel_cost(candidate);
    int entered_control=0,entered_aware=0;
    auto now=chrono::steady_clock::now();
    auto a=path_aware_polish(solver,order,min(now+chrono::milliseconds(66),started+chrono::milliseconds(2825)),false,entered_control);
    now=chrono::steady_clock::now();
    auto b=path_aware_polish(solver,order,min(now+chrono::milliseconds(66),started+chrono::milliseconds(2895)),true,entered_aware);
    string ca=solver.make_answer(a),cb=solver.make_answer(b);
    bool valid_a=solver.answer_is_valid(ca)&&!ca.empty(),valid_b=solver.answer_is_valid(cb)&&!cb.empty();
    if(!valid_a||!valid_b)throw logic_error("path-aware coverage invariant");
    cerr<<"path_common="<<common<<" path_control="<<travel_cost(ca)<<" path_extended="<<travel_cost(cb)
        <<" path_control_moves="<<entered_control<<" path_extended_moves="<<entered_aware<<' ';
    if(travel_cost(cb)<common)candidate=std::move(cb);
  }
#endif
  if (solver.answer_is_valid(candidate) && travel_cost(candidate) < legacy_cost)
    answer = std::move(candidate);
#ifdef AHC005_SPLICE
  const int before=travel_cost(answer);
  const auto cutoff=started+chrono::milliseconds(2910);
  if(chrono::steady_clock::now()<cutoff) {
    string changed=splice_realized_walk(solver,answer,cutoff);
    if(travel_cost(changed)<before)answer=std::move(changed);
  }
  cerr<<"before_splice="<<before<<' ';
#endif
  cerr << "travel=" << travel_cost(answer) << " legacy=" << legacy_cost << '\n';
  cout << answer << '\n';
}
