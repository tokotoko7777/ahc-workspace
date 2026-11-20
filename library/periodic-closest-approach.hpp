#pragma once
#include <algorithm>
#include <cmath>
#include <limits>

struct PeriodicApproach {
    int time;
    double squared_distance;
};

// Exact nearest integer sample on a rectangular torus, up to floating-point precision.
// Positive periods, lo <= hi, finite inputs and image indices fitting long long.
// Image sweep work is proportional to the product of the wrap counts in the two axes.
inline PeriodicApproach periodic_closest_approach(double dx, double dy, double vx, double vy,
                                                    double px, double py, int lo, int hi) {
    const double a=dx+lo*vx,b=dx+hi*vx,c=dy+lo*vy,d=dy+hi*vy;
    const auto image=[](double x,double period){return static_cast<long long>(std::floor(x/period+.5));};
    const double speed=vx*vx+vy*vy;
    PeriodicApproach best{lo,std::numeric_limits<double>::infinity()};
    for(long long ix=image(std::min(a,b),px);ix<=image(std::max(a,b),px);++ix) {
        for(long long iy=image(std::min(c,d),py);iy<=image(std::max(c,d),py);++iy) {
            const double x=dx-ix*px,y=dy-iy*py;
            const double t=speed==0 ? lo : std::clamp(-(x*vx+y*vy)/speed,double(lo),double(hi));
            const int p=static_cast<int>(std::floor(t)),q=static_cast<int>(std::ceil(t));
            for(int u : {p,q}) {
                const double xx=x+u*vx,yy=y+u*vy;
                const double f=xx*xx+yy*yy;
                if(f<best.squared_distance || (f==best.squared_distance && u<best.time))
                    best={u,f};
            }
        }
    }
    return best;
}
