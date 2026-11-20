#pragma once
#include <span>
#include <utility>

namespace gaussian_linear_information {
struct Reduction {
    double signal_variance;
    double trace_drop;
};
// Covariance is a symmetric PSD, row-major n-by-n matrix;
// feature holds unique valid indices with their nonzero coefficients.
// A hypothetical observation has independent additive noise variance r>0.
// Does not change the covariance. Cost O(n * feature.size()).
inline Reduction expected_reduction(int n, std::span<const double> covariance,
                                    std::span<const std::pair<int,double>> feature,
                                    double noise_variance) {
    double signal = 0.0, squared = 0.0;
    for (auto [j, x] : feature) {
        double ax = 0.0;
        for (auto [k, y] : feature) ax += covariance[j * n + k] * y;
        signal += x * ax;
    }
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (auto [j, x] : feature) ax += covariance[i * n + j] * x;
        squared += ax * ax;
    }
    return {signal, squared / (noise_variance + signal)};
}
}
