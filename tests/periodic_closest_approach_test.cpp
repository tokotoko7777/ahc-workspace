#include "../library/periodic-closest-approach.hpp"
#include <cassert>
#include <random>

int main() {
    auto edge=periodic_closest_approach(97,20,2,0,100,80,0,30);
    assert(edge.time==1 && edge.squared_distance==401);
    auto fixed=periodic_closest_approach(50,0,0,0,100,100,-5,100);
    assert(fixed.time==-5 && fixed.squared_distance==2500);
    std::mt19937 gen(5737247);
    std::uniform_int_distribution<int> all(-360,360);
    for(int test=0;test<12000;++test) {
        double x=all(gen)/4.,y=all(gen)/4.,u=all(gen)/8.,v=all(gen)/8.;
        double px=1+gen()%150,py=1+gen()%160;
        int lo=int(gen()%25)-12,hi=lo+gen()%75;
        const auto fast=periodic_closest_approach(x,y,u,v,px,py,lo,hi);
        double best=1e200;
        int arg=lo;
        for(int t=lo;t<=hi;++t) {
            double dx=std::remainder(x+t*u,px),dy=std::remainder(y+t*v,py);
            double value=dx*dx+dy*dy;
            if(value<best) best=value,arg=t;
        }
        assert(fast.time==arg && fast.squared_distance==best);
    }
}
