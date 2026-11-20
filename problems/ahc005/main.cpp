// AHC_SOURCE: examples/search/ahc005_patrol_sa.cpp
#define AHC005_TEST
#include "../../examples/search/ahc005_patrol_sa.cpp"
#include "route-splice.hpp"

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
