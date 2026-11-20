#include "../library/rounded-multiplicative-normal.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>

int main() {
    std::mt19937 rng(39051579);
    constexpr int bins=32768;
    // Independent reference: integrate over the multiplier U after analytically
    // integrating the normal on the bin [low/U, high/U].
    const auto phi=[](long double x) {return std::exp(-x*x/2)*.39894228040143267794L;};
    const auto Q=[](long double x) {return std::erfc(x*.7071067811865475244L)*.5L;};
    const auto prob=[&](long double a,long double b) {
        return b<0 ? Q(-b)-Q(-a) : Q(a)-Q(b);
    };
    for(int rep=0;rep<180;++rep) {
        const long double mean=12.+(rng()%45000)*.001;
        const long double sd=mean*(.025L+.15L*(rng()%1001)/1000);
        const long double y=std::round(1000*mean*(.78L+.44L*(rng()%1001)/1000))/1000;
        const long double eps=.0005L;
        long double a0=0,a1=0,a2=0;
        for(int q=0;q<bins;++q) {
            const long double u=.9L+.2L*(q+.5L)/bins;
            const long double a=((y-eps)/u-mean)/sd,b=((y+eps)/u-mean)/sd;
            const long double z=prob(a,b),p1=phi(a),p2=phi(b);
            a0+=z;a1+=p1-p2;a2+=z+a*p1-b*p2;
        }
        const long double shift=a1/a0;
        const long double var=a2/a0-shift*shift;
        const auto value=rounded_multiplicative_normal::posterior(
            mean,sd*sd,y,2*eps,.9,1.1);
        assert(value.valid);
        assert(std::abs(value.mean_shift/sd-shift)<1e-7L);
        assert(std::abs(value.variance/(sd*sd)-var)<1e-7L);
        assert(std::abs(value.log_mass-std::log(a0/bins))<3e-7L);
    }
    assert(!rounded_multiplicative_normal::posterior(1,0,1,.1,.9,1.1).valid);
}
