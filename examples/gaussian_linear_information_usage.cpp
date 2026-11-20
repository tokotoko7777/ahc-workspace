#include "../library/gaussian-linear-information.hpp"
#include <array>
#include <cmath>
#include <iostream>
int main() {
    // An observation of the first coordinate, whose noise variance is 1,
    // helps the correlated second coordinate too.
    const std::array<double,4> covariance{4,1,1,1};
    const std::array<std::pair<int,double>,1> feature{{{0,1}}};
    const auto gain=gaussian_linear_information::expected_reduction(2,covariance,feature,1);
    if (std::abs(gain.trace_drop-3.4)>1e-12) return 1;
    const std::array<std::pair<int,double>,2> difference{{{0,1},{1,-1}}};
    const double difference_gain = gaussian_linear_information::target_variance_drop(
        2, covariance, feature, difference, 1);
    if (std::abs(difference_gain-1.8)>1e-12) return 1;
    std::cout << gain.trace_drop << ' ' << difference_gain << '\n';
}
