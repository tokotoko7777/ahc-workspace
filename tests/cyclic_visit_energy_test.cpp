#include "../library/cyclic-visit-energy.hpp"
#include <cassert>
#include <numeric>
#include <random>

int main() {
    std::mt19937 rng(210193);
    for(int k=0;k<1200;++k) {
        const int n=2+rng()%18, span=n+rng()%40;
        CyclicVisitEnergy metric(n);
        std::vector<int> route(span), weights(n);
        std::iota(route.begin(),route.begin()+n,0);
        for(int i=n;i<span;++i) route[i]=rng()%n;
        std::shuffle(route.begin(),route.end(),rng);
        for(int& w:weights) w=rng()%1000+1;
        std::vector<std::int64_t> age(n);
        std::int64_t total=0;
        for(int tick=0;tick<2*span;++tick) {
            const int v=route[tick%span];
            for(auto& a:age) ++a;
            age[v]=0;
            if(tick>=span)
                for(int i=0;i<n;++i) total+=age[i]*weights[i];
        }
        assert(metric.numerator(route,weights)==total);
        std::rotate(route.begin(),route.begin()+rng()%span,route.end());
        assert(metric.numerator(route,weights)==total);
        for(int& x:route) if(x==0) x=1;
        assert(!metric.numerator(route,weights).has_value());
    }
}
