#include "../library/calendar-interval-packing.hpp"
#include <algorithm>
#include <random>

int main() {
    std::mt19937 rng(6123317);
    for(int trial=0;trial<1000;++trial) {
        const int horizon=1+rng()%11,n=rng()%11;
        CalendarIntervalPacking pack(horizon);
        std::vector<unsigned char> blackout(horizon+1),available(n);
        for(int j=1;j<=horizon;++j) blackout[j]=rng()%4==0;
        struct Job {int s,e,v;};
        std::vector<Job> jobs(n);
        for(int i=0;i<n;++i) {
            int a=1+rng()%horizon,b=1+rng()%horizon;
            auto& z=jobs[i];
            z={std::min(a,b),std::max(a,b),int(rng()%40)-5};
            pack.add(i,z.s,z.e,z.v);
            available[i]=rng()%4!=0;
        }
        auto eval=[&](int mask) {
            auto closed=blackout;
            int sum=0;
            for(int i=0;i<n;++i) if(mask>>i&1) {
                if(!available[i]) return -100000;
                for(int day=jobs[i].s;day<=jobs[i].e;++day) {
                    if(closed[day]) return -100000;
                    closed[day]=1;
                }
                sum+=jobs[i].v;
            }
            return sum;
        };
        int best=0;
        for(int mask=0;mask<1<<n;++mask) best=std::max(best,eval(mask));
        auto plan=pack.best(blackout,[&](int id){return available[id]!=0;});
        int mask=0;
        for(int id:plan.jobs) {
            assert(!(mask>>id&1));
            mask|=1<<id;
        }
        assert(eval(mask)==best && plan.value==best);
    }
}
