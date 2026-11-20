#pragma once
#include <cassert>
#include <cstdint>
#include <span>
#include <vector>

// Weighted interval scheduling on the integer days 1..horizon. Intervals
// are inclusive, so [1,3] cannot share a resource with [3,5].
class CalendarIntervalPacking {
    struct Job {int id,first; std::int64_t value;};
    std::vector<std::vector<Job>> by_end_;
public:
    explicit CalendarIntervalPacking(int horizon):by_end_(horizon+1) {assert(horizon>=0);}
    void add(int id,int first,int last,std::int64_t value) {
        assert(1<=first && first<=last && last<int(by_end_.size()));
        by_end_[last].push_back({id,first,value});
    }
    struct Plan {std::int64_t value=0;std::vector<int> jobs;};
    // blackout[day]!=0 disallows any selected interval covering that day.
    // allowed(id) can exclude a job used by a different resource.
    template<class Allowed>
    Plan best(std::span<const unsigned char> blackout, Allowed allowed) const {
        const int n=by_end_.size()-1;
        assert(int(blackout.size())==n+1);
        std::vector<int> prefix(n+1), prev(n+1);
        std::vector<const Job*> chosen(n+1);
        std::vector<std::int64_t> score(n+1);
        for(int d=1;d<=n;++d) {
            prefix[d]=prefix[d-1]+(blackout[d]!=0);
            score[d]=score[d-1];
            prev[d]=d-1;
            for(const auto& job:by_end_[d]) if(prefix[d]==prefix[job.first-1] && allowed(job.id)) {
                auto v=score[job.first-1]+job.value;
                if(v>score[d]) {score[d]=v;chosen[d]=&job;prev[d]=job.first-1;}
            }
        }
        Plan result{score[n],{}};
        for(int d=n;d>0;d=prev[d]) if(chosen[d]) result.jobs.push_back(chosen[d]->id);
        return result;
    }
};
