#include "../library/rounded-multiplicative-normal.hpp"
#include <iostream>

int main() {
    // Prior B is N(10,1). Observe B*U rounded to a multiple of 0.001.
    // U is uniform between 0.9 and 1.1.
    auto p=rounded_multiplicative_normal::posterior(10.,1.,10.20,.001,.9,1.1);
    if(!p.valid) return 1;
    std::cout<<10.+p.mean_shift<<' '<<p.variance<<' '<<p.log_mass<<'\n';
}
