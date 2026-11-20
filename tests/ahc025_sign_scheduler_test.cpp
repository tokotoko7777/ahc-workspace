#define AHC025_GROUP_INFORMATION
#define main ahc025_solution_main
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#include "../problems/ahc025/main.cpp"
#pragma GCC diagnostic pop
#undef main
#include <cassert>

int main() {
    SignScheduler a{{2.,2.},{1.,1.}};
    const std::vector<int> x{0},y{1};
    // N(0,2) gap: conditional gap means are +/-2/sqrt(pi).
    const double pi=std::acos(-1.);
    assert(std::abs(a.information(x,y)-4/pi)<1e-12);
    a.observe(x,y,1);
    assert(std::abs(a.mean[0]-2.-1./std::sqrt(pi))<1e-12);
    assert(std::abs(a.mean[1]-2.+1./std::sqrt(pi))<1e-12);
    for(double v:a.variance) assert(std::abs(v-1.+1./pi)<1e-12);
    const auto before=a.gap(x,y);
    a.observe(x,y,0);
    const auto after=a.gap(x,y);
    assert(before.first>0. && std::abs(after.first)<1e-12 && after.second<before.second);
}
