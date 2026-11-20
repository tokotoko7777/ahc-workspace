#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>

// A Gaussian latent scalar B, observed as round_to_step(B*U) where U is
// uniform on positive [low, high]. For the positive observed bin, only B>0
// contributes. Computes posterior moments by piecewise Gauss-Legendre
// integration of the exact rounded observation mass.
namespace rounded_multiplicative_normal {

struct Moments {
    bool valid=false;
    double mean_shift=0.,variance=0.,log_mass=0.;
};

inline Moments posterior(double mean,double variance,double observed,double step,
                         double low,double high) {
    if(!(variance>0 && step>0 && observed>step/2 && low>0 && high>low)) return {};
    const double sigma=std::sqrt(variance);
    const double y1=observed-step/2, y2=observed+step/2;
    std::array<double,4> boundaries{y1/high,y2/high,y1/low,y2/low};
    std::sort(boundaries.begin(),boundaries.end());
    for(double& z:boundaries) z=(z-mean)/sigma;
    if(!std::isfinite(boundaries.front()) || !std::isfinite(boundaries.back())) return {};
    const double ref=std::clamp(0.,boundaries.front(),boundaries.back());
    if(!std::isfinite(ref*ref)) return {};
    const double range=std::sqrt(ref*ref+72.);
    const double width=std::min(2.,8./(1.+std::abs(ref)));
    constexpr std::array<double,6> node{
        .1252334085114689,.3678314989981802,.5873179542866175,
        .7699026741943047,.9041172563704749,.9815606342467192};
    constexpr std::array<double,6> weight{
        .2491470458134028,.2334925365383548,.2031674267230659,
        .1600783285433462,.1069393259953184,.0471753363865118};
    double s0=0.,s1=0.,s2=0.;
    for(unsigned i=0;i+1<boundaries.size();++i) {
        const double a=std::max(boundaries[i],-range),b=std::min(boundaries[i+1],range);
        if(!(b>a)) continue;
        const int pieces=std::max(1,int(std::ceil((b-a)/width)));
        for(int p=0;p<pieces;++p) {
            const double half=(b-a)/(2*pieces),mid=a+(2*p+1)*half;
            for(unsigned j=0;j<node.size();++j) for(double sign:{-1.,1.}) {
                const double z=mid+sign*half*node[j];
                const double x=mean+sigma*z;
                if(!(x>0)) continue;
                const double mass=(std::min(high,y2/x)-std::max(low,y1/x))/(high-low);
                if(!(mass>0)) continue;
                const double delta=z-ref;
                const double w=half*weight[j]*mass*std::exp(-.5*delta*(z+ref));
                s0+=w;s1+=w*delta;s2+=w*delta*delta;
            }
        }
    }
    if(!(s0>0.) || !std::isfinite(s0)) return {};
    const double shifted=s1/s0;
    return {true,sigma*(ref+shifted),variance*std::max(0.,s2/s0-shifted*shifted),
            std::log(s0)-.5*ref*ref-.91893853320467274178};
}

}  // namespace rounded_multiplicative_normal
