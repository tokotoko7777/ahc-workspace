#define AHC_TEST
#define AHC_TARGET_PROBE
#define AHC_FUTURE_EXPERIMENT
#include "../problems/ahc003/main.cpp"
#include <cassert>
#include <random>

int main() {
    std::mt19937 rng(3461775);
    GaussianModel model(4,.6,.3,0);
    for(int step=0;step<600;++step) {
        std::vector<int> observed(8+rng()%60),a(10+rng()%25),b(10+rng()%25);
        for(auto* v:{&observed,&a,&b}) for(int& e:*v) e=rng()%1740;
        if(step%17==0) model.update(observed,5000*observed.size()+int(rng()%7000));
        double gap=(rng()%1000)*.001;
        auto p=model.projected_observation(observed);
        auto d=model.route_gap(a,b);
        double direct=model.decision_value(observed,a,b,gap),projected=p.benefit(d,gap);
        assert(std::abs(direct-projected)<1e-10);
    }
}
