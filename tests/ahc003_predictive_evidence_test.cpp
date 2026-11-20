#define AHC_TEST
#define AHC_REVERSIBLE_ENSEMBLE
#include "../problems/ahc003/main.cpp"
#undef AHC_TEST
#include <cassert>
#include <random>

int main() {
    GaussianModel model(4,.6,.3,0);
    std::mt19937 gen(391673621);
    for(int i=0;i<500;++i) {
        std::vector<int> path(1+gen()%85);
        for(int& e:path) e=gen()%1740;
        const int y=5000*path.size()+int(gen()%4001)-2000;
        const auto eval=model.predictive_evidence(path,y);
        const auto updated=model.update(path,y);
        assert(std::abs(eval-updated)<1e-10);
    }
}
