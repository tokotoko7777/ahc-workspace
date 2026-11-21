#pragma once
// Problem-specific open-mouth edge enclosures. Include with AHC008 constants.
class ParallelPens {
  struct Pen {
    std::vector<std::pair<int,int>> work;
    std::bitset<CELLS> inside;
    int gate = -1, gate_stance = -1;
  };

 public:
  ParallelPens() {
    auto add = [&](int axis, bool high, int start, int length, int depth) {
      Pen pen;
      const auto cell = [&](int depth0, int cross) {
        const int y = high ? S - 1 - depth0 : depth0;
        return axis == 0 ? y*S+cross : cross*S+y;
      };
      for (int t = 0; t < depth; ++t) {
        pen.work.emplace_back(cell(t,start), cell(t,start-1));
        pen.work.emplace_back(cell(t,start+length-1), cell(t,start+length));
        for (int j = 1; j < length-1; ++j) pen.inside[cell(t,start+j)] = true;
      }
      for (int j = 0; j < length; ++j) {
        const int target = cell(depth,start+j), stance = cell(depth+1,start+j);
        if (j == length/2) pen.gate = target, pen.gate_stance = stance;
        else pen.work.emplace_back(target, stance);
      }
      interior_ |= pen.inside;
      for (const auto& [wall, stance] : pen.work) future_[wall] = true;
      future_[pen.gate] = true;
      pens_.push_back(std::move(pen));
    };
    for (bool high : {false,true}) {
      for (int t : {1,8,15,22}) add(0, high, t, 6, 5);
      for (int t : {8,17}) add(1, high, t, 6, 5);
    }
  }

  std::string plan(const std::vector<int>& humans, const std::vector<int>& pets,
                   const std::bitset<CELLS>& walls) {
    std::bitset<CELLS> no_build, set, moved, claimed, occupied;
    for (int p : pets) {
      no_build[p] = true;
      for (int d = 0; d < 4; ++d) {
        int q = adjacent(p, d);
        if (q >= 0) no_build[q] = true;
      }
    }
    for (int h : humans) occupied[h] = no_build[h] = true;
    struct Job { int wall, stance, bonus; bool can_build; };
    std::vector<Job> jobs;
    for (const auto& pen : pens_) {
      if (walls[pen.gate]) continue;
      int inside = std::ranges::count_if(pets, [&](int p) { return pen.inside[p]; });
      int remaining = 0;
      for (const auto& [wall, stance] : pen.work) remaining += !walls[wall];
      const bool clear_humans = (pen.inside & occupied).none();
      if (remaining == 0 && inside > 0 && clear_humans)
        jobs.push_back({pen.gate, pen.gate_stance, 50 + 5*inside, !no_build[pen.gate]});
      for (const auto& [wall, stance] : pen.work) if (!walls[wall])
        jobs.push_back({wall, stance, 4*std::min(inside,2) + (15-remaining)/3, !no_build[wall]});
    }
    std::string ans(humans.size(), '.');
    for (int i = 0; i < int(humans.size()); ++i) {
      const int h = humans[i];
      const Job* immediate = nullptr;
      for (const auto& job : jobs) {
        if (job.stance == h && job.can_build && !set[job.wall] && !moved[job.wall] &&
            (!immediate || job.bonus > immediate->bonus)) immediate = &job;
      }
      if (immediate) {
        ans[i] = builds[direction(h,immediate->wall)];
        set[immediate->wall] = claimed[immediate->wall] = true;
        continue;
      }
      bfs_.run(h, [&](int p) { return !walls[p] && !set[p]; });
      const Job* best = nullptr;
      int best_key = INT_MAX;
      for (const auto& job : jobs) {
        if (claimed[job.wall]) continue;
        int d = bfs_.distance(job.stance);
        if (d < 0) continue;
        int key = 3*d - job.bonus + (!job.can_build ? 4 : 0);
        if (key < best_key) best_key = key, best = &job;
      }
      int goal = -1;
      if (best) {
        claimed[best->wall] = true;
        goal = best->stance;
      } else if (interior_[h] || future_[h]) {
        int best_dist = INT_MAX;
        for (int p = 0; p < CELLS; ++p) {
          int d = bfs_.distance(p);
          if (!interior_[p] && !future_[p] && d >= 0 && d < best_dist)
            best_dist = d, goal = p;
        }
      }
      if (goal >= 0) {
        const int next = bfs_.first_step(goal);
        if (next >= 0 && next != h) {
          ans[i] = moves[direction(h,next)];
          moved[next] = true;
        }
      }
    }
    return ans;
  }

 private:
  std::vector<Pen> pens_;
  std::bitset<CELLS> interior_, future_;
  FlatGridFirstStepBfs bfs_{S,S};
};
